#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBasicTimer>
#include <QProcess>
#include <QTimer>

#include <fstream>

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
    void actionChooseOutputFolder();
    void actionChooseBatchFile();
    void actionChooseMocoTrackExe();
    void pushButtonAutofill();
    void textChangedTRCFile(const QString &text);
    void textChangedOSIMFile(const QString &text);
    void textChangedOutputFolder(const QString &text);
    void textChangedExperimentName(const QString &text);
    void toolButtonRunBatch();

private slots:
    void readStandardError();
    void readStandardOutput();
    void handleFinished();
    void basicTimer();

private:
    void closeEvent(QCloseEvent *event) override;

    void setEnabled();
    void setStatusString(const QString &s);
    void log(const QString &s);
    void enumerateMenu(QMenu *menu, QList<QAction *> *actionList, bool addSubmenus = false, bool addSeparators = false);
    void lookForMocoTrack();

    static void FindFiles(const QString &filename, const QString &path, QStringList *matchingFiles);

    static bool checkReadFile(const std::string &filename, bool checkExecutable = false);
    static bool checkReadFolder(const std::string &foldername);
    static bool checkWriteFile(const std::string &filename);
    static bool checkWriteFolder(const std::string &foldername);
    static bool checkReadFile(const QString &filename, bool checkExecutable = false);
    static bool checkReadFolder(const QString &foldername);
    static bool checkWriteFile(const QString &filename);
    static bool checkWriteFolder(const QString &foldername);
    static void readTabDelimitedFile(const std::string &filename, std::vector<std::string> *columnHeadings, std::vector<std::vector<std::string>> *data);
    static bool strToBool(const std::string &input);

    Ui::MainWindow *ui;

    QTimer *m_timer;
    uint64_t m_timerCounter = 0;

    QProcess *m_tracker = nullptr;
    QString m_trackerExecutable;

    std::chrono::time_point<std::chrono::system_clock> m_startTime = std::chrono::system_clock::from_time_t(0);
    QVector<QIcon> m_iconList;
    int m_iconListIndex = 0;
    std::string m_batchFile;
    std::vector<std::string> m_batchColumnHeadings = {"RunID","OSIMFile","TRCFile","OutputFolder","MarkerWeights","StartTime","EndTime","ReserveForce","GlobalWeight","ConvergeTol","ConstraintTol","MeshIntervals","AddReserves","RemoveMuscles"};;
    std::vector<std::vector<std::string>> m_batchData;
    size_t m_batchProcessingIndex = 0;
    bool m_batchProcessingRunning = false;

    std::unique_ptr<std::ofstream> m_logStream;

};
#endif // MAINWINDOW_H
