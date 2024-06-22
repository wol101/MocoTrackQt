#include "Tracker.h"
#include "Common/TRCFileAdapter.h"

#include <OpenSim/Actuators/CoordinateActuator.h>
#include <OpenSim/Actuators/ModelOperators.h>
#include <OpenSim/Moco/osimMoco.h>

#include <QFileInfo>

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

std::string Tracker::solutionFile() const
{
    return m_solutionFile;
}

void Tracker::setSolutionFile(const std::string &newSolutionFile)
{
    m_solutionFile = newSolutionFile;
}

void Tracker::run()
{
    // use the tracking tool
    OpenSim::MocoTrack mocoTrack;
    mocoTrack.setName(m_name);

    // load a model
    OpenSim::ModelProcessor modelProcessor(m_osimFile);

    // modify the model as required
    // modelProcessor.append(OpenSim::ModOpAddExternalLoads("C:/Users/wis/Documents/OpenSim/4.5/Code/CPP/Moco/example3DWalking/grf_walk.xml"));
    modelProcessor.append(OpenSim::ModOpRemoveMuscles());
    modelProcessor.append(OpenSim::ModOpAddReserves(100.0, 1.0));

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
