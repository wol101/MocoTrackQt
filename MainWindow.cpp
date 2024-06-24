#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>

using namespace std::chrono_literals;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->toolBar->hide();

    QFont editorFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ui->plainTextEditOutput->setFont(editorFont);

    m_redirectCerr = std::make_unique<cerrRedirect>(m_capturedCerr.rdbuf());
    m_redirectCout = std::make_unique<coutRedirect>(m_capturedCout.rdbuf());

    // this allows me to use a hidden button to maintain the appearance of the grid layout
    // QSizePolicy sizePolicy = ui->pushButtonHidden->sizePolicy();
    // sizePolicy.setRetainSizeWhenHidden(true);
    // ui->pushButtonHidden->setSizePolicy(sizePolicy);
    // ui->pushButtonHidden->hide();

    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionRun, &QAction::triggered, this, &MainWindow::actionRun);
    connect(ui->actionStop, &QAction::triggered, this, &MainWindow::actionStop);
    connect(ui->actionChooseOSIMFile, &QAction::triggered, this, &MainWindow::actionChooseOSIMFile);
    connect(ui->actionChooseTRCFile, &QAction::triggered, this, &MainWindow::actionChooseTRCFile);
    connect(ui->actionOutputFolder, &QAction::triggered, this, &MainWindow::actionOutputFolder);
    connect(ui->pushButtonOSIMFile, &QPushButton::clicked, this, &MainWindow::actionChooseOSIMFile);
    connect(ui->pushButtonTRCFile, &QPushButton::clicked, this, &MainWindow::actionChooseTRCFile);
    connect(ui->pushButtonOutputFolder, &QPushButton::clicked, this, &MainWindow::actionOutputFolder);
    connect(ui->pushButtonRun, &QPushButton::clicked, this, &MainWindow::actionRun);
    connect(ui->pushButtonStop, &QPushButton::clicked, this, &MainWindow::actionStop);
    connect(ui->lineEditOSIMFile, &QLineEdit::textChanged, this, &MainWindow::textChangedOSIMFile);
    connect(ui->lineEditTRCFile, &QLineEdit::textChanged, this, &MainWindow::textChangedTRCFile);
    connect(ui->lineEditOutputFolder, &QLineEdit::textChanged, this, &MainWindow::textChangedOutputFolder);
    connect(ui->lineEditExperimentName, &QLineEdit::textChanged, this, &MainWindow::textChangedExperimentName);

    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    ui->lineEditExperimentName->setText(settings.value("LastExperimentName", "").toString());
    ui->lineEditOSIMFile->setText(settings.value("LastOSIMFile", "").toString());
    ui->lineEditTRCFile->setText(settings.value("LastTRCFile", "").toString());
    ui->lineEditOutputFolder->setText(settings.value("LastOutputFolder", "").toString());
    ui->doubleSpinBoxStartTime->setValue(settings.value("LastStartTime", "").toDouble());
    ui->doubleSpinBoxEndTime->setValue(settings.value("LastEndTime", "").toDouble());
    ui->doubleSpinBoxMeshSize->setValue(settings.value("LastMeshSize", "").toDouble());

    // and this timer just makes sure that buttons are regularly updated
    m_basicTimer.start(1000, Qt::CoarseTimer, this);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    settings.setValue("LastStartTime", ui->doubleSpinBoxStartTime->value());
    settings.setValue("LastEndTime", ui->doubleSpinBoxEndTime->value());
    settings.setValue("LastMeshSize", ui->doubleSpinBoxMeshSize->value());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.sync();
    event->accept(); // pass the event to the default handle
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_basicTimer.timerId()) { basicTimer(); }
    else { QMainWindow::timerEvent(event); }
}

void MainWindow::basicTimer()
{
    // handle logging
    std::string cerrStr = m_capturedCerr.str();
    if (cerrStr.size())
    {
        log(QString::fromStdString(cerrStr));
        m_capturedCerr.clear();
    }
    std::string coutStr = m_capturedCout.str();
    if (coutStr.size())
    {
        log(QString::fromStdString(coutStr));
        m_capturedCout.clear();
    }
    if (m_trackerFuture.valid())
    {
        std::string stdoutPath = m_tracker.stdoutPath();
        if (stdoutPath != m_currentLogFile)
        {
            m_currentLogFile = stdoutPath;
            m_currentLogFilePosition = 0;
        }
        FILE *f = fopen(m_currentLogFile.c_str(), "r");
        if (f)
        {
#ifdef WIN32
            _fseeki64(f, int64_t(m_currentLogFilePosition), SEEK_SET);
#else
            fseek(f, m_currentLogFilePosition, SEEK_SET);
#endif
            std::vector<char> buffer(1024 * 1024);
            size_t bytesRead = fread(buffer.data(), 1, buffer.size(), f);
            fclose(f);
            log(QString::fromLatin1(buffer.data(), bytesRead));
            m_currentLogFilePosition += bytesRead;
        }
    }
    // check whether tracking has finished
    if (m_trackerFuture.valid())
    {
        if (m_trackerFuture.wait_for(0ms) == std::future_status::ready)
        {
            std::string *ptr = m_trackerFuture.get();
            if (ptr) log(QString::fromStdString(*ptr));
            m_trackerThread.join(); // and it is now safe to join the thread
            setStatusString("Tracking finished");
        }
    }

    // check the controls
    setEnabled();
}

void MainWindow::actionRun()
{
    try
    {
        if (m_trackerFuture.valid()) throw std::runtime_error("MocoTrack already running");
        if (!checkReadFile(m_tracker.osimFile())) throw std::runtime_error("OSIM file cannot be read");
        if (!checkReadFile(m_tracker.trcFile())) throw std::runtime_error("TRC file cannot be read");
        if (!checkWriteFolder(m_tracker.outputFolder())) throw std::runtime_error("Output folder cannot be used");
        if (m_tracker.experimentName().size() == 0) throw std::runtime_error("Experiment name not valid");
        if (ui->doubleSpinBoxStartTime->value() >= ui->doubleSpinBoxEndTime->value()) throw std::runtime_error("Invalid start and end times");
        if (ui->doubleSpinBoxMeshSize->value() <= 0) throw std::runtime_error("Invalid mesh size");
    }
    catch (const std::runtime_error& ex)
    {
        setStatusString(QString("Run Error: ") + QString::fromStdString(ex.what()));
        QMessageBox::critical(this, "Run Error", QString::fromStdString(ex.what()));
        return;
    }
    setStatusString("Running MocoTrack");
    m_tracker.setStartTime(ui->doubleSpinBoxStartTime->value());
    m_tracker.setEndTime(ui->doubleSpinBoxEndTime->value());
    m_tracker.setMeshInterval(ui->doubleSpinBoxMeshSize->value());

    std::packaged_task<std::string *()> task( [this] { return m_tracker.run(); } ); // lambda function is needed because the class function has an implicit pointer
    m_trackerFuture = task.get_future();
    std::thread thread(std::move(task));
    m_trackerThread.swap(thread); // swap is the only option here for storing the new thread

    setEnabled();
}

void MainWindow::actionStop()
{
    setStatusString("Trying to stop simulation");
    if (!m_trackerFuture.valid()) return;
    std::string workingPath = m_tracker.workingPath();
    QDir dir(QString::fromStdString(workingPath));
    QStringList itemList = dir.entryList(QDir::Files, QDir::SortFlag::Name);
    for (auto &&item : itemList)
    {
        if (item.startsWith("delete_this_to_stop_optimization"))
        {
            QString filename = dir.absoluteFilePath(item);
            QFile file(filename);
            file.remove();
            log(QString("Removing %1").arg(filename));
        }
    }
}

bool MainWindow::isStopable()
{
    if (!m_trackerFuture.valid()) return false;
    std::string workingPath = m_tracker.workingPath();
    QDir dir(QString::fromStdString(workingPath));
    QStringList itemList = dir.entryList(QDir::Files, QDir::SortFlag::Name);
    for (auto &&item : itemList)
    {
        if (item.startsWith("delete_this_to_stop_optimization"))
        {
            return true;
        }
    }
    return false;
}



void MainWindow::actionChooseOSIMFile()
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    QString lastFile = settings.value("LastOSIMFile", "").toString();
    QString fileName = QFileDialog::getOpenFileName(this, "Open OpenSim model file", lastFile, "OpenSim Files (*.osim);;All Files (*.*)");
    if (fileName.size())
    {
        ui->lineEditOSIMFile->setText(fileName);
    }
}

void MainWindow::actionChooseTRCFile()
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    QString lastFile = settings.value("LastTRCFile", "").toString();
    QString fileName = QFileDialog::getOpenFileName(this, "Open TRC marker file", lastFile, "TRC Files (*.trc);;All Files (*.*)");
    if (fileName.size())
    {
        ui->lineEditTRCFile->setText(fileName);
    }
}

void MainWindow::actionOutputFolder()
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    QString lastFolder = settings.value("LastOutputFolder", "").toString();
    QString folderName = QFileDialog::getExistingDirectory(this, "Save STO output file", lastFolder);
    if (folderName.size())
    {
        ui->lineEditOutputFolder->setText(folderName);
    }
}

void MainWindow::textChangedTRCFile(const QString &text)
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    m_tracker.setTrcFile(text.toStdString());
    settings.setValue("LastTRCFile", text);
    setEnabled();
}

void MainWindow::textChangedOSIMFile(const QString &text)
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    m_tracker.setOsimFile(text.toStdString());
    settings.setValue("LastOSIMFile", text);
    setEnabled();
}

void MainWindow::textChangedOutputFolder(const QString &text)
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    m_tracker.setOutputFolder(text.toStdString());
    settings.setValue("LastOutputFolder", text);
    setEnabled();
}

void MainWindow::textChangedExperimentName(const QString &text)
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    m_tracker.setExperimentName(text.toStdString());
    settings.setValue("LastExperimentName", text);
    setEnabled();
}

void MainWindow::setEnabled()
{
    if (m_trackerFuture.valid()) // it is running so most things need to be disabled
    {
        auto children = findChildren<QWidget*>();
        for (auto &&it : children)
        {
            if (QPushButton *button = dynamic_cast<QPushButton *>(it)) { button->setEnabled(false); }
            if (QLineEdit *lineEdit = dynamic_cast<QLineEdit *>(it)) { lineEdit->setEnabled(false); }
            if (QDoubleSpinBox *spinBox = dynamic_cast<QDoubleSpinBox *>(it)) { spinBox->setEnabled(false); }
        }
        QList<QAction *> actionList;
        QList<QMenu*> list = menuBar()->findChildren<QMenu*>();
        for (auto &&menu : list) { enumerateMenu(menu, &actionList); }
        for (auto &&action : actionList) action->setEnabled(false);
        ui->actionQuit->setEnabled(true);
        bool isStoppable = isStopable();
        ui->actionStop->setEnabled(isStoppable);
        ui->pushButtonStop->setEnabled(isStoppable);
    }
    else
    {
        auto children = findChildren<QWidget*>();
        for (auto &&it : children)
        {
            if (QPushButton *button = dynamic_cast<QPushButton *>(it)) { button->setEnabled(true); }
            if (QLineEdit *lineEdit = dynamic_cast<QLineEdit *>(it)) { lineEdit->setEnabled(true); }
            if (QDoubleSpinBox *spinBox = dynamic_cast<QDoubleSpinBox *>(it)) { spinBox->setEnabled(true); }
        }
        QList<QAction *> actionList;
        QList<QMenu*> list = menuBar()->findChildren<QMenu*>();
        for (auto &&menu : list) { enumerateMenu(menu, &actionList); }
        for (auto &&action : actionList) action->setEnabled(true);
        ui->actionStop->setEnabled(false);
        ui->pushButtonStop->setEnabled(false);
    }
}

void MainWindow::enumerateMenu(QMenu *menu, QList<QAction *> *actionList, bool addSubmenus, bool addSeparators)
{
    for (auto &&action: menu->actions())
    {
        if (action->isSeparator())
        {
            if (addSeparators) actionList->append(action);
        }
        else
        {
            if (action->menu())
            {
                if (addSubmenus) actionList->append(action);
                enumerateMenu(action->menu(), actionList, addSubmenus, addSeparators);
            }
            else
            {
                actionList->append(action);
            }
        }
    }
}

void MainWindow::setStatusString(const QString &s)
{
    QStringList lines = s.split("\n", Qt::SkipEmptyParts);
    if (lines.size() == 0) { statusBar()->showMessage(s); }
    else { statusBar()->showMessage(lines[0]); }
    statusBar()->repaint();
    log(s);
}

void MainWindow::log(const QString &text)
{
    if (text.trimmed().size()) // only log strings with content
    {
        ui->plainTextEditOutput->appendPlainText(text);
        ui->plainTextEditOutput->repaint();
    }
}

bool MainWindow::checkReadFile(const std::string &filename)
{
    return checkReadFile(QString::fromStdString(filename));
}

bool MainWindow::checkReadFolder(const std::string &foldername)
{
    return checkReadFolder(QString::fromStdString(foldername));
}

bool MainWindow::checkWriteFile(const std::string &filename)
{
    return checkWriteFile(QString::fromStdString(filename));
}

bool MainWindow::checkWriteFolder(const std::string &foldername)
{
    return checkWriteFolder(QString::fromStdString(foldername));
}

bool MainWindow::checkReadFile(const QString &filename)
{
    if (filename.size() == 0) return false;
    QFileInfo info(filename);
    if (!info.isFile()) return false;
    if (info.isReadable()) return true;
    return false;
}

bool MainWindow::checkReadFolder(const QString &foldername)
{
    if (foldername.size() == 0) return false;
    QFileInfo info(foldername);
    if (!info.isDir()) return false;
    if (info.isReadable()) return true;
    return false;
}

bool MainWindow::checkWriteFile(const QString &filename)
{
    if (filename.size() == 0) return false;
    QFileInfo info(filename);
    if (!info.exists()) return true;
    if (!info.isFile()) return false;
    if (info.isWritable()) return true;
    return false;
}

bool MainWindow::checkWriteFolder(const QString &foldername)
{
    if (foldername.size() == 0) return false;
    QFileInfo info(foldername);
    if (!info.exists()) return true;
    if (!info.isDir()) return false;
    if (info.isWritable()) return true;
    return false;
}

