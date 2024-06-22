#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include <QFileDialog>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionRun, &QAction::triggered, this, &MainWindow::actionRun);
    connect(ui->actionChooseOSIMFile, &QAction::triggered, this, &MainWindow::actionChooseOSIMFile);
    connect(ui->actionChooseTRCFile, &QAction::triggered, this, &MainWindow::actionChooseTRCFile);
    connect(ui->actionChooseSTOFile, &QAction::triggered, this, &MainWindow::actionChooseSTOFile);

    setEnabled();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "qt_track");
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
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "qt_track");
    QString lastFile = settings.value("LastOSIMFile", "").toString();
    QString fileName = QFileDialog::getOpenFileName(this, "Open OpenSim model file", lastFile, "OpenSim Files (*.osim);;All Files (*.*)");
    if (fileName.size())
    {
        m_tracker.setOsimFile(fileName.toStdString());
        settings.setValue("LastOSIMFile", fileName);
    }
    setEnabled();
}

void MainWindow::actionChooseTRCFile()
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "qt_track");
    QString lastFile = settings.value("LastTRCFile", "").toString();
    QString fileName = QFileDialog::getOpenFileName(this, "Open TRC marker file", lastFile, "TRC Files (*.trc);;All Files (*.*)");
    if (fileName.size())
    {
        m_tracker.setTrcFile(fileName.toStdString());
        settings.setValue("LastTRCFile", fileName);
    }
    setEnabled();
}

void MainWindow::actionChooseSTOFile()
{
    QSettings settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope, "AnimalSimulationLaboratory", "qt_track");
    QString lastFile = settings.value("LastSTOFile", "").toString();
    QString fileName = QFileDialog::getSaveFileName(this, "Save STO output file", lastFile, "STO Files (*.sto);;All Files (*.*)");

    if (fileName.size())
    {
        m_tracker.setSolutionFile(fileName.toStdString());
        settings.setValue("LastSTOFile", fileName);
    }
    setEnabled();
}

void MainWindow::setEnabled()
{
    ui->actionRun->setEnabled(checkReadFile(m_tracker.osimFile()) && checkReadFile(m_tracker.trcFile()) && checkWriteFile(m_tracker.solutionFile()));
}

bool MainWindow::checkReadFile(const std::string &filename)
{
    return checkReadFile(QString::fromStdString(filename));
}

bool MainWindow::checkWriteFile(const std::string &filename)
{
    return checkWriteFile(QString::fromStdString(filename));
}


bool MainWindow::checkReadFile(const QString &filename)
{
    if (filename.size() == 0) return false;
    QFileInfo info(filename);
    if (!info.isFile()) return false;
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
