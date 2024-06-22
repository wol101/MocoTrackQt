#ifndef TRACKER_H
#define TRACKER_H

#include <string>

class Tracker
{
public:
    Tracker();

    bool canRun();
    void run();

    std::string trcFile() const;
    void setTrcFile(const std::string &newTrcFile);

    std::string osimFile() const;
    void setOsimFile(const std::string &newOsimFile);

    std::string solutionFile() const;
    void setSolutionFile(const std::string &newSolutionFile);

private:
    std::string m_trcFile;
    std::string m_osimFile;
    std::string m_solutionFile;
    std::string m_name = "qt_track";
};

#endif // TRACKER_H
