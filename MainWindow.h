#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Tracker.h"

#include <QMainWindow>
#include <QBasicTimer>

#include <future>
#include <sstream>
#include <iostream>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// simple guard classes for std::cerr and std::cout stream capture
class cerrRedirect
{
public:
    cerrRedirect(std::streambuf *newBuffer)
    {
        oldBuffer = std::cerr.rdbuf(newBuffer);
    }
    ~cerrRedirect()
    {
        std::cerr.rdbuf(oldBuffer);
    }
private:
    std::streambuf *oldBuffer;
};

class coutRedirect
{
public:
    coutRedirect(std::streambuf *newBuffer)
    {
        oldBuffer = std::cerr.rdbuf(newBuffer);
    }
    ~coutRedirect()
    {
        std::cerr.rdbuf(oldBuffer);
    }
private:
    std::streambuf *oldBuffer;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void actionRun();
    void actionChooseTRCFile();
    void actionChooseOSIMFile();
    void actionOutputFolder();
    void textChangedTRCFile(const QString &text);
    void textChangedOSIMFile(const QString &text);
    void textChangedOutputFolder(const QString &text);
    void textChangedExperimentName(const QString &text);

private:
    void closeEvent(QCloseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    void setEnabled();
    void setStatusString(const QString &s);
    void log(const QString &s);
    void basicTimer();


    static bool checkReadFile(const std::string &filename);
    static bool checkReadFolder(const std::string &foldername);
    static bool checkWriteFile(const std::string &filename);
    static bool checkWriteFolder(const std::string &foldername);
    static bool checkReadFile(const QString &filename);
    static bool checkReadFolder(const QString &foldername);
    static bool checkWriteFile(const QString &filename);
    static bool checkWriteFolder(const QString &foldername);

    Ui::MainWindow *ui;

    Tracker m_tracker;
    std::future<std::string *> m_trackerFuture;
    std::thread m_trackerThread;

    std::stringstream m_capturedCerr;
    std::unique_ptr<cerrRedirect> m_redirectCerr;
    std::stringstream m_capturedCout;
    std::unique_ptr<coutRedirect> m_redirectCout;
    QBasicTimer m_basicTimer;

    std::string m_currentLogFile;
    size_t m_currentLogFilePosition = 0;

};
#endif // MAINWINDOW_H
