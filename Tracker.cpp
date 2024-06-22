#include "Tracker.h"

#include "pystring/pystring.h"

#include "Common/TRCFileAdapter.h"

#include <OpenSim/Actuators/CoordinateActuator.h>
#include <OpenSim/Actuators/ModelOperators.h>
#include <OpenSim/Moco/osimMoco.h>

#include <QFileInfo>

#include <sstream>
#include <iomanip>
#include <ctime>
#include <filesystem>

using namespace std::string_literals;

Tracker::Tracker() {}

std::string Tracker::trcFile() const
{
    return m_trcFile;
}

void Tracker::setTrcFile(const std::string &newTrcFile)
{
    m_trcFile = newTrcFile;
}

std::string Tracker::osimFile() const
{
    return m_osimFile;
}

void Tracker::setOsimFile(const std::string &newOsimFile)
{
    m_osimFile = newOsimFile;
}

void Tracker::setExperimentName(const std::string &newExperimentName)
{
    m_experimentName = newExperimentName;
}

double Tracker::startTime() const
{
    return m_startTime;
}

void Tracker::setStartTime(double newStartTime)
{
    m_startTime = newStartTime;
}

double Tracker::endTime() const
{
    return m_endTime;
}

void Tracker::setEndTime(double newEndTime)
{
    m_endTime = newEndTime;
}

std::string Tracker::outputFolder() const
{
    return m_outputFolder;
}

void Tracker::setOutputFolder(const std::string &newOutputFolder)
{
    m_outputFolder = newOutputFolder;
}

std::string *Tracker::run()
{
    // everything lives in a datetime folder within the output folder
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
    std::string outputFolder = pystring::os::path::join(m_outputFolder, ss.str());
    try
    {
        std::filesystem::create_directories(outputFolder);
    }
    catch (...)
    {
        m_lastError = "Error: Tracker::run() unable to create \"" + outputFolder + "\"";
        return &m_lastError;
    }

    // use the tracking tool
    OpenSim::MocoTrack mocoTrack;
    mocoTrack.setName(m_experimentName);

    // load a model
    OpenSim::ModelProcessor modelProcessor(m_osimFile);

    // modify the model as required
    // modelProcessor.append(OpenSim::ModOpAddExternalLoads("C:/Users/wis/Documents/OpenSim/4.5/Code/CPP/Moco/example3DWalking/grf_walk.xml"));
    modelProcessor.append(OpenSim::ModOpRemoveMuscles());
    modelProcessor.append(OpenSim::ModOpAddReserves(100.0, 1.0));

    // save this model to the outputfolder
    OpenSim::Model model = modelProcessor.process();
    std::string modelPath = pystring::os::path::join(outputFolder, m_experimentName + "_model.osim"s);
    model.print(modelPath);

    // add the model to the tracking tool
    mocoTrack.setModel(modelProcessor);

    // add the markers
    // double lowpassFilterFreq = 6.0;
    // OpenSim::TimeSeriesTableVec3 markers(m_trcFile);
    // OpenSim::TimeSeriesTable markersFlat = markers.flatten();
    // mocoTrack.set_markers_reference(OpenSim::TableProcessor(markersFlat) | OpenSim::TabOpLowPassFilter(lowpassFilterFreq)); // requires std:c++20 and include <ranges>
    // mocoTrack.set_markers_reference(OpenSim::TableProcessor(markersFlat));
    mocoTrack.setMarkersReferenceFromTRC(m_trcFile);

    // alter some of the marker properties
    mocoTrack.set_allow_unused_references(true); // allows markers in the trc file not to be used - warning this can easily be an error that needs trapping
    mocoTrack.set_markers_global_tracking_weight(10);

    // OpenSim::TableProcessor tp = mocoTrack.get_markers_reference();
    // auto a = tp.process();

    // individual weight setting is more complex
    // OpenSim::MocoWeightSet markerWeights;
    // markerWeights.cloneAndAppend({"R.ASIS", 20});
    // markerWeights.cloneAndAppend({"L.ASIS", 20});
    // markerWeights.cloneAndAppend({"R.PSIS", 20});
    // markerWeights.cloneAndAppend({"L.PSIS", 20});
    // markerWeights.cloneAndAppend({"R.Knee", 10});
    // markerWeights.cloneAndAppend({"R.Ankle", 10});
    // markerWeights.cloneAndAppend({"R.Heel", 10});
    // markerWeights.cloneAndAppend({"R.MT5", 5});
    // markerWeights.cloneAndAppend({"R.Toe", 2});
    // markerWeights.cloneAndAppend({"L.Knee", 10});
    // markerWeights.cloneAndAppend({"L.Ankle", 10});
    // markerWeights.cloneAndAppend({"L.Heel", 10});
    // markerWeights.cloneAndAppend({"L.MT5", 5});
    // markerWeights.cloneAndAppend({"L.Toe", 2});
    // mocoTrack.set_markers_weight_set(markerWeights);

    // set the time subsample
    mocoTrack.set_initial_time(0.866684);
    mocoTrack.set_final_time(0.966684);
    // mocoTrack.set_final_time(1.866704);
    mocoTrack.set_mesh_interval(0.02);

    // now run the solver
    OpenSim::MocoSolution mocoSolution = mocoTrack.solve();
    mocoSolution.write(m_solutionFile);
}
