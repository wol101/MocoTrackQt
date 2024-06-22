#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include <QFileDialog>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // this allows me to use a hidden button to maintain the appearance of the grid layout
    QSizePolicy sizePolicy = ui->pushButtonHidden->sizePolicy();
    sizePolicy.setRetainSizeWhenHidden(true);
    ui->pushButtonHidden->setSizePolicy(sizePolicy);
    ui->pushButtonHidden->hide();

    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionRun, &QAction::triggered, this, &MainWindow::actionRun);
    connect(ui->actionChooseOSIMFile, &QAction::triggered, this, &MainWindow::actionChooseOSIMFile);
    connect(ui->actionChooseTRCFile, &QAction::triggered, this, &MainWindow::actionChooseTRCFile);
    connect(ui->actionOutputFolder, &QAction::triggered, this, &MainWindow::actionOutputFolder);
    connect(ui->pushButtonOSIMFile, &QPushButton::clicked, this, &MainWindow::actionChooseOSIMFile);
    connect(ui->pushButtonTRCFile, &QPushButton::clicked, this, &MainWindow::actionChooseTRCFile);
    connect(ui->pushButtonOutputFolder, &QPushButton::clicked, this, &MainWindow::actionOutputFolder);
    connect(ui->lineEditOSIMFile, &QLineEdit::textChanged, this, &MainWindow::textChangedOSIMFile);
    connect(ui->lineEditTRCFile, &QLineEdit::textChanged, this, &MainWindow::textChangedTRCFile);
    connect(ui->lineEditOutputFolder, &QLineEdit::textChanged, this, &MainWindow::textChangedOutputFolder);
    connect(ui->lineEditExperimentName, &QLineEdit::textChanged, this, &MainWindow::textChangedExperimentName);

    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    ui->lineEditExperimentName->setText(settings.value("LastExperimentName", "").toString());
    ui->lineEditOSIMFile->setText(settings.value("LastOSIMFile", "").toString());
    ui->lineEditTRCFile->setText(settings.value("LastTRCFile", "").toString());
    ui->lineEditOutputFolder->setText(settings.value("LastOutputFolder", "").toString());

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "MocoTrackQt");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.sync();
    event->accept(); // pass the event to the default handler
}

void MainWindow::actionRun()
{
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

