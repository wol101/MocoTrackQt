#include "Tracker.h"
#include "XMLWriter.h"

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

std::string *Tracker::run()
{
    m_lastError.clear();
    // everything lives in a datetime folder within the output folder
    auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
    std::string folderName = std::format("{:%Y-%m-%d_%H-%M-%S}", time);
    m_outputSubFolder = pystring::os::path::join(m_outputFolder, folderName);
    try
    {
        std::filesystem::create_directories(m_outputSubFolder);
    }
    catch (...)
    {
        m_lastError = "Error: Tracker::run() unable to create \"" + m_outputSubFolder + "\"";
        return &m_lastError;
    }
    WorkingDirectoryGuard workingDirectoryGuard(m_outputSubFolder);

    if (m_weightsFile.size())
    {
        std::vector<std::string> weightsFileColumnHeadings;
        std::vector<std::vector<std::string>> weightsFileData;
        readTabDelimitedFile(m_weightsFile, &weightsFileColumnHeadings, &weightsFileData);
        if (m_weightsFileColumnHeadings != weightsFileColumnHeadings)
        {
            m_lastError = "Error: Tracker::run() unable to read \"" + m_weightsFile + "\"";
            return &m_lastError;
        }
        m_weightsFileData = weightsFileData;
    }

    // use the tracking tool
    OpenSim::MocoTrack mocoTrack;
    mocoTrack.setName(m_experimentName);

    // load a model
    OpenSim::ModelProcessor modelProcessor(m_osimFile);

    // modify the model as required
    // modelProcessor.append(OpenSim::ModOpAddExternalLoads("C:/Users/wis/Documents/OpenSim/4.5/Code/CPP/Moco/example3DWalking/grf_walk.xml"));
    if (m_removeMuscles) modelProcessor.append(OpenSim::ModOpRemoveMuscles());
    if (m_addReserves) modelProcessor.append(OpenSim::ModOpAddReserves(m_reservesOptimalForce));

    // add the model to the tracking tool
    mocoTrack.setModel(modelProcessor);

    // save this model to the outputfolder
    m_model = modelProcessor.process();
    m_processedOsimFile = pystring::os::path::join(m_outputSubFolder, "01_"s + m_experimentName + "_model.osim"s);
    std::cout << "Writing \"" << m_processedOsimFile << "\"\n" << std::flush;
    try
    {
        m_model.print(m_processedOsimFile);
    }
    catch (...)
    {
        m_lastError = "Error: Tracker::run() unable to create \"" + m_outputSubFolder + "\"";
        return &m_lastError;
    }

    // add the markers
    double lowpassFilterFreq = 6.0;
    OpenSim::TimeSeriesTableVec3 markers(m_trcFile);
    OpenSim::TimeSeriesTable markersFlat = markers.flatten();
    OpenSim::TableProcessor tableProcessor(markersFlat);
    tableProcessor.append(OpenSim::TabOpLowPassFilter(lowpassFilterFreq));
    mocoTrack.set_markers_reference(tableProcessor);
    // mocoTrack.setMarkersReferenceFromTRC(m_trcFile);

    // alter some of the marker properties
    mocoTrack.set_allow_unused_references(true); // allows markers in the trc file not to be used - warning this can easily be an error that needs trapping

    // adjust the weights
    mocoTrack.set_markers_global_tracking_weight(m_markerTrackingWeight);

    if (m_weightsFileData.size() && m_weightsFileData[0].size())
    {
        for (int i = 0; i < m_model.getMarkerSet().getSize(); ++i)
        {
            for (size_t j = 0; j < m_weightsFileData[0].size(); ++j)
            {
                if (m_weightsFileData[0][j] ==  m_model.getMarkerSet().get(i).getName())
                {
                    m_markerTrackingWeights[m_weightsFileData[0][j]] = std::stod(m_weightsFileData[1][j]);
                    break;
                }
            }
        }
        for (int i = 0; i < m_model.getForceSet().getSize(); ++i)
        {
            for (size_t j = 0; j < m_weightsFileData[0].size(); ++j)
            {
                if (m_weightsFileData[0][j] ==  m_model.getForceSet().get(i).getName() && m_model.getForceSet().get(i).getConcreteClassName() == "CoordinateActuator"s)
                {
                    m_actuatorActivationWeights[m_weightsFileData[0][j]] = std::stod(m_weightsFileData[1][j]);
                    break;
                }
            }
        }
    }

    if (m_markerTrackingWeights.size())
    {
        OpenSim::MocoWeightSet markerWeights;
        for (auto &&markerWeight : m_markerTrackingWeights)
        {
            markerWeights.cloneAndAppend({markerWeight.first, markerWeight.second});
        }
        mocoTrack.set_markers_weight_set(markerWeights);
    }

    // set the time subsample
    mocoTrack.set_initial_time(m_startTime);
    mocoTrack.set_final_time(m_endTime);

    // initialise the solver so we can alter some of the parameters
    OpenSim::MocoStudy mocoStudy = mocoTrack.initialize();

    // if (m_actuatorActivationWeights.size())
    // {
        OpenSim::MocoProblem& problem = mocoStudy.updProblem();
        OpenSim::MocoControlGoal& effort = dynamic_cast<OpenSim::MocoControlGoal&>(problem.updGoal("control_effort"));
        effort.setWeight(m_actuatorActivationWeight);
        for (auto &&actuatorActivationWeight : m_actuatorActivationWeights)
        {
            effort.setWeightForControl("/forceset/"s + actuatorActivationWeight.first, actuatorActivationWeight.second);
        }
    // }

    // for (const auto& coordAct : m_model.getComponentList<OpenSim::CoordinateActuator>())
    // {
    //     auto coordPath = coordAct.getAbsolutePathString();
    //     std::cerr << coordPath << "\n";
    //     auto it = m_actuatorActivationWeights.find(coordPath);
    //     if (it != m_actuatorActivationWeights.end())
    //     {
    //         effort.setWeightForControl(coordPath, it->second);
    //     }
    // }

    // Update the solver tolerances.
    auto& solver = mocoStudy.updSolver<OpenSim::MocoCasADiSolver>();
    solver.set_optim_convergence_tolerance(m_convergenceTolerance);
    solver.set_optim_constraint_tolerance(m_constraintTolerance);
    solver.set_num_mesh_intervals(m_meshIntervals);

    // now run the solver
    OpenSim::MocoSolution mocoSolution = mocoStudy.solve();
    if (!mocoSolution.success())
    {
        m_lastError = "Error: Solution not found";
        return &m_lastError;
    }

    // output the required state and control files
    m_statesPath = pystring::os::path::join(m_outputSubFolder, "02_"s + m_experimentName + "_states.sto"s);
    std::cout << "Writing \"" << m_statesPath << "\"\n" << std::flush;
    OpenSim::STOFileAdapter::write(mocoSolution.exportToStatesTable(), m_statesPath);
    m_controlsPath = pystring::os::path::join(m_outputSubFolder, "03_"s + m_experimentName + "_controls.sto"s);
    std::cout << "Writing \"" << m_controlsPath << "\"\n" << std::flush;
    OpenSim::STOFileAdapter::write(mocoSolution.exportToControlsTable(), m_controlsPath);

    // now run some analyses to get the data we actually want
    std::string analyzePath = pystring::os::path::join(m_outputSubFolder, "04_"s + m_experimentName + "_AnalyzeTool_setup.xml"s);
    std::cout << "Writing \"" << analyzePath << "\"\n" << std::flush;
    createAnalyzerXML(analyzePath);
    OpenSim::AnalyzeTool analyze(analyzePath);
    analyze.run();

    return nullptr;
}

void Tracker::createAnalyzerXML(const std::string &filename)
{
    XMLWriter xml(true);
    xml.initiateTag("OpenSimDocument", {{"Version", "40500"}});
    xml.initiateTag("AnalyzeTool", {{"name", m_experimentName}});

    // preamble specifying the general analyses parameters
    xml.tagAndContent("model_file", m_processedOsimFile);
    xml.tagAndContent("replace_force_set", "false");
    xml.tagAndContent("force_set_files");
    xml.tagAndContent("results_directory", m_outputSubFolder);
    xml.tagAndContent("output_precision", "8");
    xml.tagAndContent("initial_time", std::format("{:.17g}", m_startTime));
    xml.tagAndContent("final_time", std::format("{:.17g}", m_endTime));
    xml.tagAndContent("solve_for_equilibrium_for_auxiliary_states", "false");
    xml.tagAndContent("maximum_number_of_integrator_steps", "2000");
    xml.tagAndContent("maximum_integrator_step_size", "1");
    xml.tagAndContent("minimum_integrator_step_size", "1e-08");
    xml.tagAndContent("integrator_error_tolerance", "1e-05");

    // the analyses wanted
    xml.initiateTag("AnalysisSet", {{"name", "Analyses"}});
    xml.initiateTag("objects");

    // MuscleAnalysis
    xml.initiateTag("MuscleAnalysis", {{"name", "MuscleAnalysis"}});
    xml.tagAndContent("on", "true");
    xml.tagAndContent("start_time", "-Inf");
    xml.tagAndContent("end_time", "Inf");
    xml.tagAndContent("step_interval", "1");
    xml.tagAndContent("in_degrees", "true");
    xml.tagAndContent("muscle_list", "all");
    xml.tagAndContent("moment_arm_coordinate_list", "all");
    xml.tagAndContent("compute_moments", "true");
    xml.terminateTag("MuscleAnalysis");

    // ForceReporter
    xml.initiateTag("ForceReporter", {{"name", "ForceReporter"}});
    xml.tagAndContent("on", "true");
    xml.tagAndContent("start_time", "-Inf");
    xml.tagAndContent("end_time", "Inf");
    xml.tagAndContent("step_interval", "1");
    xml.tagAndContent("in_degrees", "true");
    xml.tagAndContent("include_constraint_forces", "true");
    xml.terminateTag("ForceReporter");

    // JointReaction
    xml.initiateTag("JointReaction", {{"name", "JointReaction"}});
    xml.tagAndContent("on", "true");
    xml.tagAndContent("start_time", "-Inf");
    xml.tagAndContent("end_time", "Inf");
    xml.tagAndContent("step_interval", "1");
    xml.tagAndContent("in_degrees", "true");
    xml.tagAndContent("forces_file");
    xml.tagAndContent("joint_names", "all");
    xml.tagAndContent("apply_on_bodies", "child");
    xml.tagAndContent("express_in_frame", "ground");
    xml.terminateTag("JointReaction");

    // Kinematics
    xml.initiateTag("Kinematics", {{"name", "Kinematics"}});
    xml.tagAndContent("on", "true");
    xml.tagAndContent("start_time", "-Inf");
    xml.tagAndContent("end_time", "Inf");
    xml.tagAndContent("step_interval", "1");
    xml.tagAndContent("in_degrees", "true");
    xml.terminateTag("Kinematics");

    // BodyKinematics
    xml.initiateTag("BodyKinematics", {{"name", "BodyKinematics"}});
    xml.tagAndContent("on", "true");
    xml.tagAndContent("start_time", "-Inf");
    xml.tagAndContent("end_time", "Inf");
    xml.tagAndContent("step_interval", "1");
    xml.tagAndContent("in_degrees", "true");
    xml.tagAndContent("bodies", "all");
    xml.tagAndContent("express_results_in_body_local_frame", "false");
    xml.terminateTag("BodyKinematics");

    // PointKinematics - needs to be specified per point
    for (int i = 0; i < m_model.getMarkerSet().getSize(); i++)
    {
        OpenSim::Marker marker = m_model.getMarkerSet().get(i);
        std::string markerName = marker.getName();
        std::string parentName = marker.getParentFrameName();
        std::vector<std::string> parts = pystring::split(parentName, "/"s);
        const SimTK::Vec3 location = marker.get_location();
        xml.initiateTag("PointKinematics", {{"name", "PointKinematics"}});
        xml.tagAndContent("on", "true");
        xml.tagAndContent("start_time", "-Inf");
        xml.tagAndContent("end_time", "Inf");
        xml.tagAndContent("step_interval", "1");
        xml.tagAndContent("in_degrees", "true");
        xml.tagAndContent("body_name", parts.back());
        xml.tagAndContent("relative_to_body_name", "ground");
        xml.tagAndContent("point_name", markerName);
        xml.tagAndContent("point", std::format("{:.17g} {:.17g} {:.17g}", location[0], location[1], location[2]));
        xml.terminateTag("PointKinematics");
    }

    xml.terminateTag("objects");
    xml.tagAndContent("groups");
    xml.terminateTag("AnalysisSet");

    // the controls
    xml.initiateTag("ControllerSet", {{"name", "Controllers"}});
    xml.initiateTag("objects");
    xml.initiateTag("PrescribedController");
    xml.tagAndContent("controls_file", m_controlsPath);
    xml.tagAndContent("interpolation_method", "1");
    xml.terminateTag("PrescribedController");
    xml.terminateTag("objects");
    xml.tagAndContent("groups");
    xml.terminateTag("ControllerSet");

    // postscript extras
    xml.tagAndContent("external_loads_file");
    xml.tagAndContent("states_file", m_statesPath);
    xml.tagAndContent("coordinates_file");
    xml.tagAndContent("speeds_file");
    xml.tagAndContent("lowpass_cutoff_frequency_for_coordinates", "-1");

    xml.terminateTag("AnalyzeTool");
    xml.terminateTag("OpenSimDocument");

    std::ofstream outputFile(filename, std::ios::binary);
    outputFile.write(xml.xmlString().data(), xml.xmlString().size());
    outputFile.close();
}

void Tracker::readTabDelimitedFile(const std::string &filename, std::vector<std::string> *columnHeadings, std::vector<std::vector<std::string>> *data)
{
    columnHeadings->clear();
    data->clear();
    std::ostringstream buffer;
    try {
        std::ifstream file(filename, std::ios::binary);
        if (!file) return;
        buffer << file.rdbuf();
        file.close();
    }
    catch (...)
    {
        return;
    }
    std::vector<std::string> lines = pystring::splitlines(buffer.str());
    if (lines.size() == 0) return;
    *columnHeadings = pystring::split(lines[0], "\t");
    if (columnHeadings->size() == 0) return;
    for (size_t i = 0; i < columnHeadings->size(); i++)
    {
        data->push_back(std::vector<std::string>());
    }
    std::vector<std::string> tokens;
    for (size_t i = 1; i < lines.size(); i++)
    {
        tokens = pystring::split(lines[i], "\t");
        if (tokens.size() == 0) continue; // skip blank lines
        for (size_t j = 0; j < columnHeadings->size(); j++)
        {
            if (j < tokens.size()) { (*data)[j].push_back(tokens[j]); }
            else { (*data)[j].push_back(""); } // pad any lines that are incomplete
        }
    }
}

double Tracker::actuatorActivationWeight() const
{
    return m_actuatorActivationWeight;
}

void Tracker::setActuatorActivationWeight(double newActuatorActivationWeight)
{
    m_actuatorActivationWeight = newActuatorActivationWeight;
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

int Tracker::meshIntervals() const
{
    return m_meshIntervals;
}

void Tracker::setMeshIntervals(int newMeshIntervals)
{
    m_meshIntervals = newMeshIntervals;
}

bool Tracker::addReserves() const
{
    return m_addReserves;
}

void Tracker::setAddReserves(bool newAddReserves)
{
    m_addReserves = newAddReserves;
}

bool Tracker::removeMuscles() const
{
    return m_removeMuscles;
}

void Tracker::setRemoveMuscles(bool newRemoveMuscles)
{
    m_removeMuscles = newRemoveMuscles;
}

double Tracker::reservesOptimalForce() const
{
    return m_reservesOptimalForce;
}

void Tracker::setReservesOptimalForce(double newReservesOptimalForce)
{
    m_reservesOptimalForce = newReservesOptimalForce;
}

double Tracker::markerTrackingWeight() const
{
    return m_markerTrackingWeight;
}

void Tracker::setMarkerTrackingWeight(double newMarkerTrackingWeight)
{
    m_markerTrackingWeight = newMarkerTrackingWeight;
}

double Tracker::convergenceTolerance() const
{
    return m_convergenceTolerance;
}

void Tracker::setConvergenceTolerance(double newConvergenceTolerance)
{
    m_convergenceTolerance = newConvergenceTolerance;
}

double Tracker::constraintTolerance() const
{
    return m_constraintTolerance;
}

void Tracker::setConstraintTolerance(double newConstraintTolerance)
{
    m_constraintTolerance = newConstraintTolerance;
}

std::string Tracker::outputFolder() const
{
    return m_outputFolder;
}

void Tracker::setOutputFolder(const std::string &newOutputFolder)
{
    m_outputFolder = newOutputFolder;
}

std::string Tracker::weightsFile() const
{
    return m_weightsFile;
}

void Tracker::setWeightsFile(const std::string &newWeightsFile)
{
    m_weightsFile = newWeightsFile;
}


