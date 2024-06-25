#ifndef TRACKER_H
#define TRACKER_H

#include <string>
#include <filesystem>

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

    bool addReserves() const;
    void setAddReserves(bool newAddReserves);

    bool removeMuscles() const;
    void setRemoveMuscles(bool newRemoveMuscles);

    double reservesOptimalForce() const;
    void setReservesOptimalForce(double newReservesOptimalForce);

    double globalTrackingWeight() const;
    void setGlobalTrackingWeight(double newGlobalTrackingWeight);

    double convergenceTolerance() const;
    void setConvergenceTolerance(double newConvergenceTolerance);

    double constraintTolerance() const;
    void setConstraintTolerance(double newConstraintTolerance);

private:

    std::string m_trcFile;
    std::string m_osimFile;
    std::string m_outputFolder;
    std::string m_experimentName;

    double m_startTime = 0;
    double m_endTime = 1.0;
    double m_meshInterval = 0.02;
    double m_reservesOptimalForce = 100.0;
    double m_globalTrackingWeight = 10.0;
    double m_convergenceTolerance = 1e-3;
    double m_constraintTolerance = 1e-4;

    bool m_addReserves = false;
    bool m_removeMuscles = false;

    std::string m_lastError;
};

class WorkingDirectoryGuard
{
public:
    WorkingDirectoryGuard(std::string newWorkingDirectory)
    {
        m_storedPath = std::filesystem::current_path();
        std::filesystem::current_path(newWorkingDirectory);
    }
    ~WorkingDirectoryGuard()
    {
        std::filesystem::current_path(m_storedPath);
    }
private:
    std::filesystem::path m_storedPath;
};

#endif // TRACKER_H
