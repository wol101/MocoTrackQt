#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include "LineEditDouble.h"

#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>

#include <thread>

using namespace std::chrono_literals;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->toolBar->hide();

    QFont editorFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ui->plainTextEditOutput->setFont(editorFont);

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
    connect(ui->pushButtonAutofill, &QPushButton::clicked, this, &MainWindow::pushButtonAutofill);
    connect(ui->lineEditOSIMFile, &QLineEdit::textChanged, this, &MainWindow::textChangedOSIMFile);
    connect(ui->lineEditTRCFile, &QLineEdit::textChanged, this, &MainWindow::textChangedTRCFile);
    connect(ui->lineEditOutputFolder, &QLineEdit::textChanged, this, &MainWindow::textChangedOutputFolder);
    connect(ui->lineEditExperimentName, &QLineEdit::textChanged, this, &MainWindow::textChangedExperimentName);

    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    ui->lineEditExperimentName->setText(settings.value("ExperimentName", "").toString());
    ui->lineEditOSIMFile->setText(settings.value("OSIMFile", "").toString());
    ui->lineEditTRCFile->setText(settings.value("TRCFile", "").toString());
    ui->lineEditOutputFolder->setText(settings.value("OutputFolder", "").toString());

    auto children = findChildren<LineEditDouble *>();
    for (auto &&it : children)
    {
        it->setBottom(std::numeric_limits<float>::min());
        it->setDecimals(6);
    }
    ui->lineEditStartTime->setBottom(0);
    ui->lineEditStartTime->setValue(settings.value("StartTime", "0").toDouble());
    ui->lineEditEndTime->setValue(settings.value("EndTime", "1").toDouble());
    ui->lineEditReservesForce->setValue(settings.value("ReservesForce", "100").toDouble());
    ui->lineEditGlobalWeight->setValue(settings.value("GlobalWeight", "10").toDouble());
    ui->lineEditConstraintTolerance->setValue(settings.value("ConstraintTolerance", "1e-4").toDouble());
    ui->lineEditConvergenceTolerance->setValue(settings.value("ConvergenceTolerance", "1e-3").toDouble());

    ui->spinBoxMeshIntervals->setValue(settings.value("MeshIntervals", "50").toInt());

    ui->checkBoxAddReserves->setChecked(settings.value("AddReserves", "").toBool());
    ui->checkBoxRemoveMuscles->setChecked(settings.value("RemoveMuscles", "").toBool());

    // and this timer just makes sure that buttons are regularly updated
    m_basicTimer.start(1000, Qt::CoarseTimer, this);

    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    lookForMocoTrack();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    if (m_tracker)
    {
        int ret = QMessageBox::warning(this, "MocoTrackQt", "mocotrack is running.\nAre you sure you want to close?.", QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No), QMessageBox::No);
        if (ret == QMessageBox::No)
        {
            event->ignore();
            return;
        }
        m_tracker->kill();
    }

    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    settings.setValue("StartTime", ui->lineEditStartTime->value());
    settings.setValue("EndTime", ui->lineEditEndTime->value());
    settings.setValue("ReservesForce", ui->lineEditReservesForce->value());
    settings.setValue("GlobalWeight", ui->lineEditGlobalWeight->value());
    settings.setValue("ConstraintTolerance", ui->lineEditConstraintTolerance->value());
    settings.setValue("ConvergenceTolerance", ui->lineEditConvergenceTolerance->value());
    settings.setValue("MeshIntervals", ui->spinBoxMeshIntervals->value());
    settings.setValue("AddReserves", ui->checkBoxAddReserves->isChecked());
    settings.setValue("RemoveMuscles", ui->checkBoxRemoveMuscles->isChecked());
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
    // check the controls
    setEnabled();
}

void MainWindow::actionRun()
{
    Q_ASSERT(m_tracker == nullptr);

    QString osimFile = QFileInfo(ui->lineEditOSIMFile->text()).absoluteFilePath();
    QString trcFile = QFileInfo(ui->lineEditTRCFile->text()).absoluteFilePath();
    QString outputFolder = QFileInfo(ui->lineEditOutputFolder->text()).absoluteFilePath();
    QString experimentName = ui->lineEditExperimentName->text();
    double startTime = ui->lineEditStartTime->value();
    double endTime = ui->lineEditEndTime->value();
    double reservesForce = ui->lineEditReservesForce->value();
    double globalWeight = ui->lineEditGlobalWeight->value();
    double constraintTolerance = ui->lineEditConstraintTolerance->value();
    double convergenceTolerance = ui->lineEditConvergenceTolerance->value();
    int meshIntervals = ui->spinBoxMeshIntervals->value();
    bool addReserves = ui->checkBoxAddReserves->isChecked();
    bool removeMuscles = ui->checkBoxRemoveMuscles->isChecked();
    QRegularExpression validExperimentName("^[^<>:\"/\\|?*]+$"); // this just excludes characters that are not valid in windows paths
    try
    {
        if (!checkReadFile(m_trackerExecutable, true)) throw std::runtime_error("trackerExecutable cannot be run");
        if (!checkReadFile(osimFile)) throw std::runtime_error("OSIM file cannot be read");
        if (!checkReadFile(trcFile)) throw std::runtime_error("TRC file cannot be read");
        if (!checkWriteFolder(outputFolder)) throw std::runtime_error("Output folder cannot be used");
        if (!validExperimentName.match(experimentName).hasMatch()) throw std::runtime_error("Experiment name not valid");
        if (startTime >= endTime) throw std::runtime_error("Invalid start and end times");
    }
    catch (const std::runtime_error& ex)
    {
        setStatusString(QString("Run Error: ") + QString::fromStdString(ex.what()));
        QMessageBox::critical(this, "Run Error", QString::fromStdString(ex.what()));
        return;
    }
    setStatusString("Running MocoTrack");

    m_tracker = new QProcess(this);
    QString program = QFileInfo(m_trackerExecutable).absoluteFilePath();
    QStringList arguments;
    arguments << "--trcFile" << trcFile
              << "--osimFile" << osimFile
              << "--outputFolder" << outputFolder
              << "--experimentName" << experimentName
              << "--startTime" << QString("%1").arg(startTime, 0, 'g', 17)
              << "--endTime" << QString("%1").arg(endTime, 0, 'g', 17)
              << "--reservesOptimalForce" << QString("%1").arg(reservesForce, 0, 'g', 17)
              << "--globalTrackingWeight" << QString("%1").arg(globalWeight, 0, 'g', 17)
              << "--convergenceTolerance" << QString("%1").arg(convergenceTolerance, 0, 'g', 17)
              << "--constraintTolerance" << QString("%1").arg(constraintTolerance, 0, 'g', 17)
              << "--addReserves" << QString("%1").arg(addReserves ? "true" : "false")
              << "--removeMuscles" << QString("%1").arg(removeMuscles ? "true" : "false")
              << "--meshIntervals" << QString("%1").arg(meshIntervals);

    connect(m_tracker, &QProcess::readyReadStandardOutput, this, &MainWindow::readStandardOutput);
    connect(m_tracker, &QProcess::readyReadStandardError, this, &MainWindow::readStandardError);
    connect(m_tracker, &QProcess::finished, this, &MainWindow::handleFinished);

    QStringList commandLine;
    commandLine.append(program.count(" ") ? QString("\"") + program + QString("\"") : program );
    for (auto &&argument : arguments) { commandLine.append(argument.count(" ") ? QString("\"") + argument + QString("\"") : argument); }
    log(commandLine.join(" "));

    m_tracker->start(program, arguments, QIODeviceBase::ReadWrite | QIODeviceBase::Unbuffered);
    if (m_tracker->waitForStarted(10000) == false)
    {
        m_tracker->kill();
        delete m_tracker;
        m_tracker = nullptr;
        setStatusString("Error starting up the GA");
    }
}


void MainWindow::actionStop()
{
    if (!m_tracker) return;
    setStatusString("Trying to stop MocoTrack");
    ui->actionStop->setEnabled(false);
    ui->pushButtonStop->setEnabled(false);
    ui->pushButtonStop->repaint();
    m_tracker->kill();
    std::this_thread::sleep_for(5000ms);
}

void MainWindow::actionBatch()
{
}

void MainWindow::pushButtonAutofill()
{
    auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
    std::string experimentName = std::format("Run_{:%Y-%m-%d_%H-%M-%S}", time);
    ui->lineEditExperimentName->setText(QString::fromStdString(experimentName));
}

bool MainWindow::isStopable()
{
    if (m_tracker) return true;
    return false;
}

void MainWindow::actionChooseOSIMFile()
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    QString lastFile = settings.value("OSIMFile", "").toString();
    QString fileName = QFileDialog::getOpenFileName(this, "Open OpenSim model file", lastFile, "OpenSim Files (*.osim);;All Files (*.*)");
    if (fileName.size())
    {
        ui->lineEditOSIMFile->setText(fileName);
    }
}

void MainWindow::actionChooseTRCFile()
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    QString lastFile = settings.value("TRCFile", "").toString();
    QString fileName = QFileDialog::getOpenFileName(this, "Open TRC marker file", lastFile, "TRC Files (*.trc);;All Files (*.*)");
    if (fileName.size())
    {
        ui->lineEditTRCFile->setText(fileName);
    }
}

void MainWindow::actionOutputFolder()
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    QString lastFolder = settings.value("OutputFolder", "").toString();
    QString folderName = QFileDialog::getExistingDirectory(this, "Save STO output file", lastFolder);
    if (folderName.size())
    {
        ui->lineEditOutputFolder->setText(folderName);
    }
}

void MainWindow::textChangedTRCFile(const QString &text)
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    settings.setValue("TRCFile", text);
    setEnabled();
}

void MainWindow::textChangedOSIMFile(const QString &text)
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    settings.setValue("OSIMFile", text);
    setEnabled();
}

void MainWindow::textChangedOutputFolder(const QString &text)
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    settings.setValue("OutputFolder", text);
    setEnabled();
}

void MainWindow::textChangedExperimentName(const QString &text)
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    settings.setValue("ExperimentName", text);
    setEnabled();
}

void MainWindow::setEnabled()
{
    if (m_tracker) // it is running so most things need to be disabled
    {
        auto children = findChildren<QWidget*>();
        for (auto &&it : children)
        {
            if (QPushButton *button = dynamic_cast<QPushButton *>(it)) { button->setEnabled(false); }
            if (QLineEdit *lineEdit = dynamic_cast<QLineEdit *>(it)) { lineEdit->setEnabled(false); }
            if (QCheckBox *checkBox = dynamic_cast<QCheckBox *>(it)) { checkBox->setEnabled(false); }
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
            if (QCheckBox *checkBox = dynamic_cast<QCheckBox *>(it)) { checkBox->setEnabled(false); }
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

bool MainWindow::checkReadFile(const std::string &filename, bool checkExecutable)
{
    return checkReadFile(QString::fromStdString(filename), checkExecutable);
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

bool MainWindow::checkReadFile(const QString &filename, bool checkExecutable)
{
    if (filename.size() == 0) return false;
    QFileInfo info(filename);
    if (!info.isFile()) return false;
    if (!info.isReadable()) return false;
    if (checkExecutable) { if (!info.isExecutable()) return false; }
    return true;
}

bool MainWindow::checkReadFolder(const QString &foldername)
{
    if (foldername.size() == 0) return false;
    QFileInfo info(foldername);
    if (!info.isDir()) return false;
    if (!info.isReadable()) return false;
    return true;
}

bool MainWindow::checkWriteFile(const QString &filename)
{
    if (filename.size() == 0) return false;
    QFileInfo info(filename);
    if (!info.exists()) return true;
    if (!info.isFile()) return false;
    if (!info.isWritable()) return false;
    return true;
}

bool MainWindow::checkWriteFolder(const QString &foldername)
{
    if (foldername.size() == 0) return false;
    QFileInfo info(foldername);
    if (!info.exists()) { return checkWriteFolder(info.absolutePath()); }
    if (!info.isDir()) return false;
    if (!info.isWritable()) return false;
    return true;
}

void MainWindow::readStandardError()
{
    QString output = m_tracker->readAllStandardError();
    log(output);
}

void MainWindow::readStandardOutput()
{
    QString output = m_tracker->readAllStandardOutput();
    log(output);
}

void MainWindow::handleFinished()
{
    if (!m_tracker)
    {
        setStatusString("Error in handleFinished");
        return;
    }
    int result = m_tracker->exitCode();
    int exitStatus = m_tracker->exitStatus();
    setStatusString(QString("handleFinished exitCode = %1 exitStatus = %2").arg(result).arg(exitStatus));
    delete m_tracker;
    m_tracker = nullptr;
    if (result == 0 && exitStatus == 0)
    {
        setStatusString("MocoTrack finished");
    }
    else
    {
        setStatusString("Error after running MocoTrack");
    }
    setEnabled();
}

void MainWindow::lookForMocoTrack()
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
#ifdef WIN32
    QString defaultExecutable = "mocotrack.exe";
#else
    QString defaultExecutable = "mocotrack";
#endif
    m_trackerExecutable = settings.value("TrackerExecutable", defaultExecutable).toString();
    if (checkReadFile(m_trackerExecutable, true)) return;
    QString startSearch = QCoreApplication::applicationDirPath();
    QDir dir(startSearch);
    m_trackerExecutable = dir.absoluteFilePath(defaultExecutable);
    if (checkReadFile(m_trackerExecutable, true))
    {
        settings.setValue("TrackerExecutable", m_trackerExecutable);
        return;
    }
    QStringList matchingFiles;
    FindFiles(defaultExecutable, startSearch, &matchingFiles);
    for (auto &&matchingFile : matchingFiles)
    {
        m_trackerExecutable = matchingFile;
        if (checkReadFile(m_trackerExecutable, true))
        {
            settings.setValue("TrackerExecutable", m_trackerExecutable);
            return;
        }
    }
    QString fileName = QFileDialog::getOpenFileName(this, "Please choose the MocoTrack executable", startSearch, "Executable Files (*.exe);;All Files (*.*)");
    if (fileName.size())
    {
        m_trackerExecutable = fileName;
        settings.setValue("TrackerExecutable", m_trackerExecutable);
        return;
    }
    setStatusString(QString("Startup Error: No MocoTrack executable found"));
    QMessageBox::critical(this, "Startup Error", "No MocoTrack executable found");
    return;
}

void MainWindow::FindFiles(const QString &filename, const QString &path, QStringList *matchingFiles)
{
    QDir dir(path);
    QFileInfoList items = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot); // QDir::NoDotAndDotDot needs to be specified with what is wanted otherwise the list is empty
    for (auto &&item : items)
    {
        if (item.isDir()) { FindFiles(filename, item.fileName(), matchingFiles); }
        else { if (filename == item.fileName()) matchingFiles->append(dir.absoluteFilePath(item.fileName())); }
    }
}
