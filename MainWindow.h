#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBasicTimer>
#include <QProcess>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void actionRun();
    void actionStop();
    void actionChooseTRCFile();
    void actionChooseOSIMFile();
    void actionOutputFolder();
    void actionBatch();
    void actionChooseMocoTrackExe();
    void pushButtonAutofill();
    void textChangedTRCFile(const QString &text);
    void textChangedOSIMFile(const QString &text);
    void textChangedOutputFolder(const QString &text);
    void textChangedExperimentName(const QString &text);

private slots:
    void readStandardError();
    void readStandardOutput();
    void handleFinished();

private:
    void closeEvent(QCloseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    void setEnabled();
    void setStatusString(const QString &s);
    void log(const QString &s);
    void basicTimer();
    void enumerateMenu(QMenu *menu, QList<QAction *> *actionList, bool addSubmenus = false, bool addSeparators = false);
    bool isStopable();
    void lookForMocoTrack();

    static void FindFiles(const QString &filename, const QString &path, QStringList *matchingFiles);

    static bool checkReadFile(const std::string &filename, bool checkExecutable);
    static bool checkReadFolder(const std::string &foldername);
    static bool checkWriteFile(const std::string &filename);
    static bool checkWriteFolder(const std::string &foldername);
    static bool checkReadFile(const QString &filename, bool checkExecutable = false);
    static bool checkReadFolder(const QString &foldername);
    static bool checkWriteFile(const QString &filename);
    static bool checkWriteFolder(const QString &foldername);

    Ui::MainWindow *ui;

    QBasicTimer m_basicTimer;
    uint64_t m_timerCounter = 0;

    std::string m_currentLogFile;
    size_t m_currentLogFilePosition = 0;

    QProcess *m_tracker = nullptr;
    QString m_trackerExecutable;

    std::chrono::time_point<std::chrono::system_clock> m_startTime = std::chrono::system_clock::from_time_t(0);
    bool m_batchProcessing = false;
    QVector<QIcon> m_iconList;
    int m_iconListIndex = 0;
};
#endif // MAINWINDOW_H
