#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->toolBar->hide();

    m_redirectCerr = std::make_unique<cerrRedirect>(m_capturedCerr.rdbuf());
    m_redirectCout = std::make_unique<coutRedirect>(m_capturedCout.rdbuf());


    // this allows me to use a hidden button to maintain the appearance of the grid layout
    // QSizePolicy sizePolicy = ui->pushButtonHidden->sizePolicy();
    // sizePolicy.setRetainSizeWhenHidden(true);
    // ui->pushButtonHidden->setSizePolicy(sizePolicy);
    // ui->pushButtonHidden->hide();

    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionRun, &QAction::triggered, this, &MainWindow::actionRun);
    connect(ui->actionChooseOSIMFile, &QAction::triggered, this, &MainWindow::actionChooseOSIMFile);
    connect(ui->actionChooseTRCFile, &QAction::triggered, this, &MainWindow::actionChooseTRCFile);
    connect(ui->actionOutputFolder, &QAction::triggered, this, &MainWindow::actionOutputFolder);
    connect(ui->pushButtonOSIMFile, &QPushButton::clicked, this, &MainWindow::actionChooseOSIMFile);
    connect(ui->pushButtonTRCFile, &QPushButton::clicked, this, &MainWindow::actionChooseTRCFile);
    connect(ui->pushButtonOutputFolder, &QPushButton::clicked, this, &MainWindow::actionOutputFolder);
    connect(ui->pushButtonRun, &QPushButton::clicked, this, &MainWindow::actionRun);
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
}

void MainWindow::actionRun()
{
    setStatusString("Running MocoTrack");
    try
    {
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
    m_tracker.setStartTime(ui->doubleSpinBoxStartTime->value());
    m_tracker.setEndTime(ui->doubleSpinBoxEndTime->value());
    m_tracker.setMeshInterval(ui->doubleSpinBoxMeshSize->value());
    m_tracker.run();
    setEnabled();
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
    ui->actionRun->setEnabled(checkReadFile(m_tracker.osimFile()) && checkReadFile(m_tracker.trcFile()) && checkWriteFolder(m_tracker.outputFolder()) && m_tracker.experimentName().size());
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

