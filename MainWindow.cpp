#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include "LineEditDouble.h"

#include "pystring/pystring.h"

#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QTimer>

#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <regex>

using namespace std::chrono_literals;
using namespace std::string_literals;

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
    connect(ui->actionChooseOutputFolder, &QAction::triggered, this, &MainWindow::actionChooseOutputFolder);
    connect(ui->actionChooseBatchFile, &QAction::triggered, this, &MainWindow::actionChooseBatchFile);
    connect(ui->actionChooseMocoTrackExe, &QAction::triggered, this, &MainWindow::actionChooseMocoTrackExe);
    connect(ui->pushButtonOSIMFile, &QPushButton::clicked, this, &MainWindow::actionChooseOSIMFile);
    connect(ui->pushButtonTRCFile, &QPushButton::clicked, this, &MainWindow::actionChooseTRCFile);
    connect(ui->pushButtonOutputFolder, &QPushButton::clicked, this, &::MainWindow::actionChooseOutputFolder);
    connect(ui->pushButtonBatchFile, &QPushButton::clicked, this, &::MainWindow::actionChooseBatchFile);
    connect(ui->pushButtonWeightsFile, &QPushButton::clicked, this, &::MainWindow::actionChooseWeightsFile);
    connect(ui->pushButtonRun, &QPushButton::clicked, this, &MainWindow::actionRun);
    connect(ui->pushButtonStop, &QPushButton::clicked, this, &MainWindow::actionStop);
    connect(ui->pushButtonAutofill, &QPushButton::clicked, this, &MainWindow::pushButtonAutofill);
    connect(ui->lineEditOSIMFile, &QLineEdit::textChanged, this, &MainWindow::textChangedOSIMFile);
    connect(ui->lineEditTRCFile, &QLineEdit::textChanged, this, &MainWindow::textChangedTRCFile);
    connect(ui->lineEditOutputFolder, &QLineEdit::textChanged, this, &MainWindow::textChangedOutputFolder);
    connect(ui->lineEditExperimentName, &QLineEdit::textChanged, this, &MainWindow::textChangedExperimentName);
    connect(ui->lineEditBatchFile, &QLineEdit::textChanged, this, &MainWindow::textChangedBatchFile);
    connect(ui->lineEditWeightsFile, &QLineEdit::textChanged, this, &MainWindow::textChangedWeightsFile);
    connect(ui->toolButtonRunBatch, &QPushButton::clicked, this, &MainWindow::toolButtonRunBatch);

    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    ui->lineEditExperimentName->setText(settings.value("ExperimentName", "").toString());
    ui->lineEditOSIMFile->setText(settings.value("OSIMFile", "").toString());
    ui->lineEditTRCFile->setText(settings.value("TRCFile", "").toString());
    ui->lineEditOutputFolder->setText(settings.value("OutputFolder", "").toString());
    ui->lineEditBatchFile->setText(settings.value("BatchFile", "").toString());
    ui->lineEditWeightsFile->setText(settings.value("WeightsFile", "").toString());

    auto children = findChildren<LineEditDouble *>();
    for (auto &&it : children)
    {
        it->setBottom(std::numeric_limits<float>::min());
        it->setDecimals(6);
    }
    ui->lineEditStartTime->setBottom(0);
    ui->lineEditStartTime->setValue(settings.value("StartTime", "0").toDouble());
    ui->lineEditEndTime->setValue(settings.value("EndTime", "1").toDouble());
    ui->lineEditReserveForce->setValue(settings.value("ReservesForce", "100").toDouble());
    ui->lineEditGlobalWeight->setValue(settings.value("GlobalWeight", "10").toDouble());
    ui->lineEditConstraintTolerance->setValue(settings.value("ConstraintTolerance", "1e-4").toDouble());
    ui->lineEditConvergenceTolerance->setValue(settings.value("ConvergenceTolerance", "1e-3").toDouble());

    ui->spinBoxMeshIntervals->setValue(settings.value("MeshIntervals", "50").toInt());

    ui->checkBoxAddReserves->setChecked(settings.value("AddReserves", "").toBool());
    ui->checkBoxRemoveMuscles->setChecked(settings.value("RemoveMuscles", "").toBool());

    setWindowIcon(QIcon(QString(":/images/running_icon_cutout.svg")));
    for (int i = 0; i < 12; i++)
    {
        m_iconList.push_back(QIcon(QString(":/images/running_icon%1.svg").arg(i)));
    }
    ui->toolButtonRunBatch->setIcon(m_iconList[0]);

    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    lookForMocoTrack();

    // initialise the batch data
    for (size_t i = 0; i < m_batchColumnHeadings.size(); i++) { m_batchData.push_back(std::vector<std::string>()); }

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::basicTimer);
    m_timer->start(100); // 100ms is fast enough for this application

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
    settings.setValue("ReservesForce", ui->lineEditReserveForce->value());
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

void MainWindow::basicTimer()
{
    // this is triggered every 100ms (approximately)
    ++m_timerCounter;

    if (m_batchProcessingRunning)
    {
        // animate the icon
        ui->toolButtonRunBatch->setIcon(m_iconList[m_iconListIndex]);
        ui->toolButtonRunBatch->repaint();
        ++m_iconListIndex;
        if (m_iconListIndex >= m_iconList.size()) m_iconListIndex = 0;
    }
    else
    {
        m_iconListIndex = 0;
        ui->toolButtonRunBatch->setIcon(m_iconList[m_iconListIndex]);
    }

    if (m_timerCounter % 10) // so this will happen about once per second
    {
        // check whether we should be running something
        if (!m_tracker && m_batchProcessingRunning && m_batchProcessingIndex < m_batchData[0].size())
        {
            std::string p = pystring::os::path::dirname(ui->lineEditBatchFile->text().toStdString()); // want the paths to be relative to the batch file
            Q_ASSERT(m_batchProcessingIndex < m_batchData[0].size());
            ui->lineEditExperimentName->setText(QString::fromStdString(m_batchData[0][m_batchProcessingIndex]));
            ui->lineEditOSIMFile->setText(QString::fromStdString(pystring::os::path::join(p, m_batchData[1][m_batchProcessingIndex])));
            ui->lineEditTRCFile->setText(QString::fromStdString(pystring::os::path::join(p, m_batchData[2][m_batchProcessingIndex])));
            ui->lineEditOutputFolder->setText(QString::fromStdString(pystring::os::path::join(p, m_batchData[3][m_batchProcessingIndex])));
            ui->lineEditWeightsFile->setText(QString::fromStdString(pystring::os::path::join(p, m_batchData[4][m_batchProcessingIndex])));
            ui->lineEditStartTime->setText(QString::fromStdString(m_batchData[5][m_batchProcessingIndex]));
            ui->lineEditEndTime->setText(QString::fromStdString(m_batchData[6][m_batchProcessingIndex]));
            ui->lineEditReserveForce->setText(QString::fromStdString(m_batchData[7][m_batchProcessingIndex]));
            ui->lineEditGlobalWeight->setText(QString::fromStdString(m_batchData[8][m_batchProcessingIndex]));
            ui->lineEditConvergenceTolerance->setText(QString::fromStdString(m_batchData[9][m_batchProcessingIndex]));
            ui->lineEditConstraintTolerance->setText(QString::fromStdString(m_batchData[10][m_batchProcessingIndex]));
            ui->spinBoxMeshIntervals->setValue(std::stoi(m_batchData[11][m_batchProcessingIndex]));
            ui->checkBoxAddReserves->setChecked(strToBool(m_batchData[12][m_batchProcessingIndex]));
            ui->checkBoxRemoveMuscles->setChecked(strToBool(m_batchData[13][m_batchProcessingIndex]));
            ++m_batchProcessingIndex;
            actionRun();
        }

        // check the controls
        setEnabled();
    }
}

void MainWindow::actionRun()
{
    Q_ASSERT(m_tracker == nullptr);

    QString osimFile = QFileInfo(ui->lineEditOSIMFile->text()).absoluteFilePath();
    QString trcFile = QFileInfo(ui->lineEditTRCFile->text()).absoluteFilePath();
    QString outputFolder = QFileInfo(ui->lineEditOutputFolder->text()).absoluteFilePath();
    QString experimentName = ui->lineEditExperimentName->text();
    QString weightsFile = ui->lineEditWeightsFile->text();
    double startTime = ui->lineEditStartTime->value();
    double endTime = ui->lineEditEndTime->value();
    double reservesForce = ui->lineEditReserveForce->value();
    double globalWeight = ui->lineEditGlobalWeight->value();
    double constraintTolerance = ui->lineEditConstraintTolerance->value();
    double convergenceTolerance = ui->lineEditConvergenceTolerance->value();
    int meshIntervals = ui->spinBoxMeshIntervals->value();
    bool addReserves = ui->checkBoxAddReserves->isChecked();
    bool removeMuscles = ui->checkBoxRemoveMuscles->isChecked();
    static QRegularExpression validExperimentName("^[^<>:\"/\\|?*]+$"); // this just excludes characters that are not valid in windows paths
    try
    {
        if (!checkReadFile(m_trackerExecutable, true)) throw std::runtime_error("trackerExecutable cannot be run");
        if (!checkReadFile(osimFile)) throw std::runtime_error("OSIM file cannot be read");
        if (!checkReadFile(trcFile)) throw std::runtime_error("TRC file cannot be read");
        if (!checkWriteFolder(outputFolder)) throw std::runtime_error("Output folder cannot be used");
        if (!validExperimentName.match(experimentName).hasMatch()) throw std::runtime_error("Experiment name not valid");
        if (startTime >= endTime) throw std::runtime_error("Invalid start and end times");
        if (weightsFile.size() && !checkReadFile(weightsFile)) throw std::runtime_error("Weights file cannot be read");

    }
    catch (const std::runtime_error& ex)
    {
        m_batchProcessingRunning = false;
        setStatusString(QString("Run Error: ") + QString::fromStdString(ex.what()));
        QMessageBox::critical(this, "Run Error", QString::fromStdString(ex.what()));
        return;
    }
    setStatusString("Running MocoTrack");

    m_startTime = std::chrono::system_clock::now();
    auto const time = std::chrono::current_zone()->to_local(m_startTime);
    std::string timeString = std::format("{:%Y-%m-%d %H-%M-%S}", time);
    std::string logPath = pystring::os::path::join(outputFolder.toStdString(), timeString + "_"s + experimentName.toStdString() + ".log"s);
    m_logStream = std::make_unique<std::ofstream>(logPath, std::ios::binary);
    log(QString::fromStdString("Simulation started at "s + timeString));

    m_tracker = new QProcess(this);
    QString program = QFileInfo(m_trackerExecutable).absoluteFilePath();
    QStringList arguments;
    arguments << "--trcFile" << trcFile
              << "--osimFile" << osimFile
              << "--outputFolder" << outputFolder
              << "--experimentName" << experimentName
              << "--weightsFile" << weightsFile
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
        m_batchProcessingRunning = false;
        m_tracker->kill();
        delete m_tracker;
        m_tracker = nullptr;
        setStatusString("Error starting up the GA");
    }
}

void MainWindow::actionStop()
{
    if (!m_tracker) return;
    int ret = QMessageBox::warning(this, "MocoTrackQt", "mocotrack is running.\nAre you sure you want to stop it?.", QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No), QMessageBox::No);
    if (ret == QMessageBox::No) { return; }
    setStatusString("Trying to stop MocoTrack");
    ui->actionStop->setEnabled(false);
    ui->pushButtonStop->setEnabled(false);
    ui->pushButtonStop->repaint();
    m_tracker->kill();
    std::this_thread::sleep_for(5000ms);
}

void MainWindow::actionChooseBatchFile()
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    QString lastFile = settings.value("BatchFile", "").toString();
    QString fileName = QFileDialog::getOpenFileName(this, "Choose batch file", lastFile, "Batch Files (*.tab *.txt);;All Files (*.*)");
    if (fileName.size())
    {
        ui->lineEditBatchFile->setText(fileName);
        settings.setValue("BatchFile", fileName);

        // because we almost certainly have a valid file, try and put the firt line in the rest of the UI
        try
        {
            std::string batchFile = ui->lineEditBatchFile->text().toStdString();
            if (!(checkReadFile(batchFile))) throw std::runtime_error(batchFile + " cannot be read");
            std::vector<std::string> columnHeadings;
            std::vector<std::vector<std::string>> data;
            readTabDelimitedFile(batchFile, &columnHeadings, &data);
            if (columnHeadings.size() == 0 || data.size() == 0 || data[0].size() == 0) throw std::runtime_error(batchFile + " contains no data");
            if (m_batchColumnHeadings != columnHeadings) throw std::runtime_error(batchFile + " column heading mismatch");
            m_batchData = data;
            m_batchProcessingIndex = 0;
            m_batchProcessingRunning = false;
        }
        catch (const std::runtime_error& ex)
        {
            setStatusString(QString("Run Batch Error: ") + QString::fromStdString(ex.what()));
            QMessageBox::critical(this, "Run Batch Error", QString::fromStdString(ex.what()));
            return;
        }
        std::string p = pystring::os::path::dirname(ui->lineEditBatchFile->text().toStdString()); // want the paths to be relative to the batch file
        Q_ASSERT(m_batchProcessingIndex < m_batchData[0].size());
        ui->lineEditExperimentName->setText(QString::fromStdString(m_batchData[0][m_batchProcessingIndex]));
        ui->lineEditOSIMFile->setText(QString::fromStdString(pystring::os::path::join(p, m_batchData[1][m_batchProcessingIndex])));
        ui->lineEditTRCFile->setText(QString::fromStdString(pystring::os::path::join(p, m_batchData[2][m_batchProcessingIndex])));
        ui->lineEditOutputFolder->setText(QString::fromStdString(pystring::os::path::join(p, m_batchData[3][m_batchProcessingIndex])));
        ui->lineEditWeightsFile->setText(QString::fromStdString(pystring::os::path::join(p, m_batchData[4][m_batchProcessingIndex])));
        ui->lineEditStartTime->setText(QString::fromStdString(m_batchData[5][m_batchProcessingIndex]));
        ui->lineEditEndTime->setText(QString::fromStdString(m_batchData[6][m_batchProcessingIndex]));
        ui->lineEditReserveForce->setText(QString::fromStdString(m_batchData[7][m_batchProcessingIndex]));
        ui->lineEditGlobalWeight->setText(QString::fromStdString(m_batchData[8][m_batchProcessingIndex]));
        ui->lineEditConvergenceTolerance->setText(QString::fromStdString(m_batchData[9][m_batchProcessingIndex]));
        ui->lineEditConstraintTolerance->setText(QString::fromStdString(m_batchData[10][m_batchProcessingIndex]));
        ui->spinBoxMeshIntervals->setValue(std::stoi(m_batchData[11][m_batchProcessingIndex]));
        ui->checkBoxAddReserves->setChecked(strToBool(m_batchData[12][m_batchProcessingIndex]));
        ui->checkBoxRemoveMuscles->setChecked(strToBool(m_batchData[13][m_batchProcessingIndex]));
    }
}

void MainWindow::actionChooseWeightsFile()
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    QString lastFile = settings.value("WeightsFile", "").toString();
    QString fileName = QFileDialog::getOpenFileName(this, "Choose weights file", lastFile, "Weights Files (*.tab *.txt);;All Files (*.*)");
    if (fileName.size())
    {
        ui->lineEditWeightsFile->setText(fileName);
        settings.setValue("WeightsFile", fileName);
    }
}

void MainWindow::pushButtonAutofill()
{
    auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
    std::string experimentName = std::format("Run_{:%Y-%m-%d_%H-%M-%S}", time);
    ui->lineEditExperimentName->setText(QString::fromStdString(experimentName));
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

void MainWindow::actionChooseOutputFolder()
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    QString lastFolder = settings.value("OutputFolder", "").toString();
    QString folderName = QFileDialog::getExistingDirectory(this, "Save STO output file", lastFolder);
    if (folderName.size())
    {
        ui->lineEditOutputFolder->setText(folderName);
    }
}

void MainWindow::actionChooseMocoTrackExe()
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    QString lastFile = settings.value("TrackerExecutable", "mocotrack.exe").toString();
    QString fileName = QFileDialog::getOpenFileName(this, "Please choose the MocoTrack executable", lastFile, "Executable Files (*.exe);;All Files (*.*)");
    if (fileName.size())
    {
        m_trackerExecutable = fileName;
        settings.setValue("TrackerExecutable", m_trackerExecutable);
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

void MainWindow::textChangedWeightsFile(const QString &text)
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    settings.setValue("WeightsFile", text);
    setEnabled();
}

void MainWindow::textChangedBatchFile(const QString &text)
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    settings.setValue("BatchFile", text);
    setWindowTitle(text);
    setEnabled();
}

void MainWindow::toolButtonRunBatch()
{
    if (m_batchProcessingRunning)
    {
        int ret = QMessageBox::warning(this, "MocoTrackQt", "mocotrack batch is running.\nAre you sure you want to stop it?.", QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No), QMessageBox::No);
        if (ret == QMessageBox::No) { return; }
        m_batchProcessingRunning = false;
        setStatusString("Trying to stop MocoTrack batch");
        ui->actionStop->setEnabled(false);
        ui->pushButtonStop->setEnabled(false);
        ui->pushButtonStop->repaint();
        if (m_tracker) m_tracker->kill();
        std::this_thread::sleep_for(5000ms);
        return;
    }
    try
    {
        std::string batchFile = ui->lineEditBatchFile->text().toStdString();
        if (!(checkReadFile(batchFile))) throw std::runtime_error(batchFile + " cannot be read");
        std::vector<std::string> columnHeadings;
        std::vector<std::vector<std::string>> data;
        readTabDelimitedFile(batchFile, &columnHeadings, &data);
        if (columnHeadings.size() == 0 || data.size() == 0 || data[0].size() == 0) throw std::runtime_error(batchFile + " contains no data");
        if (m_batchColumnHeadings != columnHeadings) throw std::runtime_error(batchFile + " column heading mismatch");
        m_batchData = data;
        m_batchProcessingIndex = 0;
        m_batchProcessingRunning = true;
        setEnabled();
    }
    catch (const std::runtime_error& ex)
    {
        setStatusString(QString("Run Batch Error: ") + QString::fromStdString(ex.what()));
        QMessageBox::critical(this, "Run Batch Error", QString::fromStdString(ex.what()));
        return;
    }
}

void MainWindow::setEnabled()
{
    ui->lineEditConstraintTolerance->setEnabled(!m_tracker);
    ui->lineEditConvergenceTolerance->setEnabled(!m_tracker);
    ui->lineEditEndTime->setEnabled(!m_tracker);
    ui->lineEditGlobalWeight->setEnabled(!m_tracker);
    ui->lineEditReserveForce->setEnabled(!m_tracker);
    ui->lineEditStartTime->setEnabled(!m_tracker);
    ui->actionChooseBatchFile->setEnabled(!m_tracker);
    ui->actionChooseMocoTrackExe->setEnabled(!m_tracker);
    ui->actionChooseOSIMFile->setEnabled(!m_tracker);
    ui->actionChooseOutputFolder->setEnabled(!m_tracker);
    ui->actionChooseTRCFile->setEnabled(!m_tracker);
    ui->actionChooseWeightsFile->setEnabled(!m_tracker);
    ui->actionQuit->setEnabled(true);
    ui->actionRun->setEnabled(!m_tracker);
    ui->actionStop->setEnabled(m_tracker);
    ui->checkBoxAddReserves->setEnabled(!m_tracker);
    ui->checkBoxRemoveMuscles->setEnabled(!m_tracker);
    ui->lineEditExperimentName->setEnabled(!m_tracker);
    ui->lineEditOSIMFile->setEnabled(!m_tracker);
    ui->lineEditOutputFolder->setEnabled(!m_tracker);
    ui->lineEditTRCFile->setEnabled(!m_tracker);
    ui->pushButtonAutofill->setEnabled(!m_tracker);
    ui->pushButtonOSIMFile->setEnabled(!m_tracker);
    ui->pushButtonOutputFolder->setEnabled(!m_tracker);
    ui->pushButtonRun->setEnabled(!m_tracker);
    ui->pushButtonStop->setEnabled(m_tracker);
    ui->pushButtonTRCFile->setEnabled(!m_tracker);
    ui->pushButtonWeightsFile->setEnabled(!m_tracker);
    ui->pushButtonBatchFile->setEnabled(!m_tracker);
    ui->spinBoxMeshIntervals->setEnabled(!m_tracker);
    ui->toolButtonRunBatch->setEnabled(true);
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
    static QRegularExpression re("[\r\n]");
    QStringList lines = s.split(re, Qt::SkipEmptyParts);
    if (lines.size() == 0) { statusBar()->showMessage(s); }
    else { statusBar()->showMessage(lines[0]); }
    statusBar()->repaint();
    log(s);
}

void MainWindow::log(const QString &text)
{
    if (m_logStream) (*m_logStream) << text.toStdString();
    // appendPlainText adds a paragraph so we need to remove any trailing \n
    if (text.endsWith('\n')) ui->plainTextEditOutput->appendPlainText(text.chopped(1));
    else ui->plainTextEditOutput->appendPlainText(text);
    ui->plainTextEditOutput->repaint();
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
    if (!info.exists())
    {
        QDir dir(foldername);
        return dir.mkpath(".");
    }
    if (!info.isDir()) return false;
    if (!info.isWritable()) return false;
    return true;
}

void MainWindow::readStandardError()
{
    QString output = m_tracker->readAllStandardError();
    output.remove('\r');
    log(output);
}

void MainWindow::readStandardOutput()
{
    QString output = m_tracker->readAllStandardOutput();
    output.remove('\r');
    log(output);
}

void MainWindow::handleFinished()
{
    if (!m_tracker)
    {
        setStatusString("Error in handleFinished");
        return;
    }

    auto currentTime = std::chrono::system_clock::now();
    auto const time = std::chrono::current_zone()->to_local(currentTime);
    std::string timeString = std::format("Simulation finished at {:%Y-%m-%d %H-%M-%S}\n", time);
    log(QString::fromStdString(timeString));
    auto seconds = duration_cast<std::chrono::seconds>(currentTime - m_startTime);
    timeString = std::format("Duration = {:.3f} hours\n", double(seconds.count()) / 3600);
    log(QString::fromStdString(timeString));
    m_startTime = std::chrono::system_clock::from_time_t(0);

    int result = m_tracker->exitCode();
    int exitStatus = m_tracker->exitStatus();
    delete m_tracker;
    m_tracker = nullptr;
    if (m_batchProcessingRunning && m_batchProcessingIndex >= m_batchData[0].size())
    {
        m_batchProcessingRunning = false;
        m_batchProcessingIndex = 0;
    }
    if (result == 0 && exitStatus == 0)
    {
        setStatusString("MocoTrack finished");
    }
    else
    {
        setStatusString(QString("MocoTrack finished with exitCode = %1 exitStatus = %2").arg(result).arg(exitStatus));
    }
    if (m_logStream) m_logStream.reset();
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
    // nothing found so prompt the user
    actionChooseMocoTrackExe();
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

void MainWindow::readTabDelimitedFile(const std::string &filename, std::vector<std::string> *columnHeadings, std::vector<std::vector<std::string>> *data)
{
    columnHeadings->clear();
    data->clear();
    std::ostringstream buffer;
    try {
        std::ifstream file(filename, std::ios::binary);
        if (!file) return;
        buffer << file.rdbuf();
        file.close();
    }
    catch (...)
    {
        return;
    }
    std::vector<std::string> lines = pystring::splitlines(buffer.str());
    if (lines.size() == 0) return;
    *columnHeadings = pystring::split(lines[0], "\t");
    if (columnHeadings->size() == 0) return;
    for (size_t i = 0; i < columnHeadings->size(); i++)
    {
        data->push_back(std::vector<std::string>());
    }
    std::vector<std::string> tokens;
    for (size_t i = 1; i < lines.size(); i++)
    {
        tokens = pystring::split(lines[i], "\t");
        if (tokens.size() == 0) continue; // skip blank lines
        for (size_t j = 0; j < columnHeadings->size(); j++)
        {
            if (j < tokens.size()) { (*data)[j].push_back(tokens[j]); }
            else { (*data)[j].push_back(""); } // pad any lines that are incomplete
        }
    }
}

bool MainWindow::strToBool(const std::string &input) // checks for a number (zero/non-zero) and then for common false words
{
    const static std::vector<std::string> falseWords = {"false", "off", "no"};
    static std::regex numberMatch("([+-]?)(?=\\d|\\.\\d)\\d*(\\.\\d*)?([Ee]([+-]?\\d+))?");
    std::string processedInput = pystring::lower(pystring::strip(input));
    if (std::regex_match(processedInput, numberMatch))
    {
        if (std::stod(processedInput) == 0) return false;
        return true;
    }
    if (std::find(falseWords.begin(), falseWords.end(), processedInput) != falseWords.end()) return false;
    return true;
}

