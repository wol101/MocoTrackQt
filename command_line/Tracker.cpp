#include "Tracker.h"

#include "pystring/pystring.h"

#include <OpenSim/Common/STOFileAdapter.h>
#include <OpenSim/Actuators/CoordinateActuator.h>
#include <OpenSim/Actuators/ModelOperators.h>
#include <OpenSim/Moco/osimMoco.h>
#include <OpenSim/Tools/AnalyzeTool.h>
#include <OpenSim/Analyses/MuscleAnalysis.h>
#include <OpenSim/Analyses/JointReaction.h>
#include <OpenSim/Analyses/ForceReporter.h>
#include <OpenSim/Analyses/PointKinematics.h>
#include <OpenSim/Analyses/Kinematics.h>
#include <OpenSim/Analyses/BodyKinematics.h>

#include <filesystem>
#include <chrono>

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

std::string Tracker::experimentName() const
{
    return m_experimentName;
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

double Tracker::meshInterval() const
{
    return m_meshInterval;
}

void Tracker::setMeshInterval(double newMeshInterval)
{
    m_meshInterval = newMeshInterval;
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
    m_lastError.clear();
    // everything lives in a datetime folder within the output folder
    auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
    std::string folderName = std::format("{:%Y-%m-%d_%H-%M-%S}", time);
    std::string outputFolder = pystring::os::path::join(m_outputFolder, folderName);
    try
    {
        std::filesystem::create_directories(outputFolder);
    }
    catch (...)
    {
        m_lastError = "Error: Tracker::run() unable to create \"" + outputFolder + "\"";
        return &m_lastError;
    }
    WorkingDirectoryGuard workingDirectoryGuard(outputFolder);

    // use the tracking tool
    OpenSim::MocoTrack mocoTrack;
    mocoTrack.setName(m_experimentName);

    // load a model
    OpenSim::ModelProcessor modelProcessor(m_osimFile);

    // modify the model as required
    // modelProcessor.append(OpenSim::ModOpAddExternalLoads("C:/Users/wis/Documents/OpenSim/4.5/Code/CPP/Moco/example3DWalking/grf_walk.xml"));
    // modelProcessor.append(OpenSim::ModOpRemoveMuscles());
    modelProcessor.append(OpenSim::ModOpAddReserves(100.0, 1.0));

    // save this model to the outputfolder
    OpenSim::Model model = modelProcessor.process();
    std::string modelPath = pystring::os::path::join(outputFolder, "01_"s + m_experimentName + "_model.osim"s);
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
    mocoTrack.set_initial_time(m_startTime);
    mocoTrack.set_final_time(m_endTime);
    mocoTrack.set_mesh_interval(0.02);

    // now run the solver
    OpenSim::MocoSolution mocoSolution = mocoTrack.solve();
    if (!mocoSolution.success())
    {
        m_lastError = "Error: Solution not found";
        return &m_lastError;
    }

    // output the required state and control files
    std::string statesPath = pystring::os::path::join(outputFolder, "02_"s + m_experimentName + "_states.sto"s);
    OpenSim::STOFileAdapter::write(mocoSolution.exportToStatesTable(), statesPath);
    std::string controlsPath = pystring::os::path::join(outputFolder, "03_"s + m_experimentName + "_controls.sto"s);
    OpenSim::STOFileAdapter::write(mocoSolution.exportToControlsTable(), controlsPath);

    // now run some analyses to get the data we actually want
    OpenSim::AnalyzeTool analyzeSetup;
    analyzeSetup.setName(m_experimentName);
    analyzeSetup.setModelFilename(modelPath);
    analyzeSetup.setStatesFileName(statesPath);
    analyzeSetup.setResultsDir(outputFolder);
    analyzeSetup.setInitialTime(m_startTime);
    analyzeSetup.setFinalTime(m_endTime);

    auto muscleAnalysis = new OpenSim::MuscleAnalysis();
    auto forceReporter = new OpenSim::ForceReporter();
    auto jointReaction = new OpenSim::JointReaction();
    auto kinematics = new OpenSim::Kinematics();
    auto bodyKinematics = new OpenSim::BodyKinematics();

    jointReaction->setName("JointReaction"); // doesn't seem to get set properly elsewhere

    analyzeSetup.updAnalysisSet().adoptAndAppend(muscleAnalysis);
    analyzeSetup.updAnalysisSet().adoptAndAppend(forceReporter);
    analyzeSetup.updAnalysisSet().adoptAndAppend(jointReaction);
    analyzeSetup.updAnalysisSet().adoptAndAppend(kinematics);
    analyzeSetup.updAnalysisSet().adoptAndAppend(bodyKinematics);

    analyzeSetup.updControllerSet().adoptAndAppend(new OpenSim::PrescribedController(controlsPath));

    std::string analyzePath = pystring::os::path::join(outputFolder, "04_"s + m_experimentName + "_AnalyzeTool_setup.xml"s);
    analyzeSetup.print(analyzePath);

    OpenSim::AnalyzeTool analyze(analyzePath); // not sure why this needs to be a separate instance but that is how it is done in the examples
    analyze.run();

    return nullptr;
}

