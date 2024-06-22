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


    std::string outputFolder() const;
    void setOutputFolder(const std::string &newOutputFolder);

    std::string experimentName() const;
    void setExperimentName(const std::string &newExperimentName);

private:

    std::string m_trcFile;
    std::string m_osimFile;
    std::string m_outputFolder;
    std::string m_experimentName;
};

#endif // TRACKER_H
