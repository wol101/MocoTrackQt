#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "Tracker.h"

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
    void actionChooseTRCFile();
    void actionChooseOSIMFile();
    void actionChooseSTOFile();

private:
    void closeEvent(QCloseEvent *event) override;

    void setEnabled();

    static bool checkReadFile(const std::string &filename);
    static bool checkWriteFile(const std::string &filename);
    static bool checkReadFile(const QString &filename);
    static bool checkWriteFile(const QString &filename);

    Ui::MainWindow *ui;

    Tracker m_tracker;
};
#endif // MAINWINDOW_H
