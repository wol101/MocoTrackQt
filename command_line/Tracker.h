#ifndef TRACKER_H
#define TRACKER_H

#include <OpenSim/OpenSim.h>

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

    int meshIntervals() const;
    void setMeshIntervals(int newMeshIntervals);

    bool addReserves() const;
    void setAddReserves(bool newAddReserves);

    bool removeMuscles() const;
    void setRemoveMuscles(bool newRemoveMuscles);

    double reservesOptimalForce() const;
    void setReservesOptimalForce(double newReservesOptimalForce);

    double markerTrackingWeight() const;
    void setMarkerTrackingWeight(double newMarkerTrackingWeight);

    double convergenceTolerance() const;
    void setConvergenceTolerance(double newConvergenceTolerance);

    double constraintTolerance() const;
    void setConstraintTolerance(double newConstraintTolerance);

    std::string weightsFile() const;
    void setWeightsFile(const std::string &newWeightsFile);

    double actuatorActivationWeight() const;
    void setActuatorActivationWeight(double newActuatorActivationWeight);

private:
    void createAnalyzerXML(const std::string &filename);

    static void readTabDelimitedFile(const std::string &filename, std::vector<std::string> *columnHeadings, std::vector<std::vector<std::string>> *data);

    std::string m_trcFile;
    std::string m_osimFile;
    std::string m_outputFolder;
    std::string m_experimentName;
    std::string m_weightsFile;

    std::string m_processedOsimFile;
    std::string m_statesPath;
    std::string m_controlsPath;
    OpenSim::Model m_model;
    std::string m_outputSubFolder;

    double m_startTime = 0;
    double m_endTime = 1.0;
    int m_meshIntervals = 50;
    double m_reservesOptimalForce = 100.0;
    double m_markerTrackingWeight = 10.0;
    double m_actuatorActivationWeight = 0.001;
    double m_convergenceTolerance = 1e-3;
    double m_constraintTolerance = 1e-4;

    bool m_addReserves = false;
    bool m_removeMuscles = false;

    std::vector<std::string> m_weightsFileColumnHeadings = {"Name", "Weight"};
    std::vector<std::vector<std::string>> m_weightsFileData;
    std::map<std::string, double> m_markerTrackingWeights;
    std::map<std::string, double> m_actuatorActivationWeights;

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
