#ifndef TRACKER_H
#define TRACKER_H

#include <mutex>
#include <string>

class Tracker
{
public:
    Tracker();

    bool canRun();
    std::string *run();

    std::string trcFile() const;
    void setTrcFile(const std::string &newTrcFile);

    std::string osimFile() const;
    void setOsimFile(const std::string &newOsimFile);

    std::string outputFolder() const;
    void setOutputFolder(const std::string &newOutputFolder);

    std::string experimentName() const;
    void setExperimentName(const std::string &newExperimentName);

    double startTime() const;
    void setStartTime(double newStartTime);

    double endTime() const;
    void setEndTime(double newEndTime);

    double meshInterval() const;
    void setMeshInterval(double newMeshInterval);

    std::string stdoutPath();
    void setStdoutPath(const std::string &newStdoutPath);

    std::string workingPath();
    void setWorkingPath(const std::string &newWorkingPath);

private:

    std::string m_trcFile;
    std::string m_osimFile;
    std::string m_outputFolder;
    std::string m_experimentName;

    double m_startTime = 0;
    double m_endTime = 1.0;
    double m_meshInterval = 0.02;

    std::string m_lastError;
    std::string m_stdoutPath;
    std::string m_workingPath;

    std::mutex m_mutex;
};

#endif // TRACKER_H
