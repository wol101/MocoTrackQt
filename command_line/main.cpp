#include "ArgParse.h"
#include "Tracker.h"

#include <iostream>

using namespace std::string_literals;

int main(int argc, const char **argv)
{
    std::string compileDate(__DATE__);
    std::string compileTime(__TIME__);
    ArgParse argparse;
    argparse.Initialise(argc, argv, "mocotrack command line front end to OpenSim MocoTrack"s + compileDate + " "s + compileTime, 0, 0);
    // required arguments
    argparse.AddArgument("-t"s, "--trcFile"s, "Input TRC file"s, ""s, 1, true, ArgParse::String);
    argparse.AddArgument("-i"s, "--osimFile"s, "Input OSIM file"s, ""s, 1, true, ArgParse::String);
    argparse.AddArgument("-o"s, "--outputFolder"s, "Output folder"s, ""s, 1, true, ArgParse::String);
    argparse.AddArgument("-e"s, "--experimentName"s, "Experiment descriptor"s, ""s, 1, true, ArgParse::String);
    // optional arguments
    argparse.AddArgument("-wf"s, "--weightsFile"s, "File containing weights for markers and actuators"s, ""s, 1, false, ArgParse::String);
    argparse.AddArgument("-st"s, "--startTime"s, "Start time"s, "0.0"s, 1, false, ArgParse::Double);
    argparse.AddArgument("-et"s, "--endTime"s, "End time"s, "1.0"s, 1, false, ArgParse::Double);
    argparse.AddArgument("-min"s, "--meshIntervals"s, "Number of mesh intervals"s, "50"s, 1, false, ArgParse::Int);
    argparse.AddArgument("-min2"s, "--meshInterval"s, "Duration of mesh interval"s, "0"s, 1, false, ArgParse::Double);
    argparse.AddArgument("-mit"s, "--maxIterations"s, "Maximum number of iterations"s, "3000"s, 1, false, ArgParse::Int);
    argparse.AddArgument("-rof"s, "--reservesOptimalForce"s, "Reserves optimal force"s, "100.0"s, 1, false, ArgParse::Double);
    argparse.AddArgument("-mtw"s, "--markerTrackingWeight"s, "Global marker tracking weight"s, "10.0"s, 1, false, ArgParse::Double);
    argparse.AddArgument("-aaw"s, "--actuatorActivationWeight"s, "Global actuator activation weight"s, "0.001"s, 1, false, ArgParse::Double);
    argparse.AddArgument("-cvt"s, "--convergenceTolerance"s, "Convergence tolerance"s, "1e-3"s, 1, false, ArgParse::Double);
    argparse.AddArgument("-cst"s, "--constraintTolerance"s, "Constraint tolerance"s, "1e-4"s, 1, false, ArgParse::Double);
    argparse.AddArgument("-ar"s, "--addReserves"s, "Add reserve actuators to the model"s, "false"s, 1, false, ArgParse::Bool);
    argparse.AddArgument("-rm"s, "--removeMuscles"s, "Remove all muscles from the model"s, "false"s, 1, false, ArgParse::Bool);

    int err = argparse.Parse();
    if (err)
    {
        argparse.Usage();
        exit(1);
    }

    std::string trcFile;
    std::string osimFile;
    std::string outputFolder;
    std::string experimentName;
    std::string weightsFile;
    double startTime = 0;
    double endTime = 1.0;
    int meshIntervals = 50;
    int maxIterations = 3000;
    double reservesOptimalForce = 100.0;
    double markerTrackingWeight = 10.0;
    double actuatorActivationWeight = 0.001;
    double convergenceTolerance = 1e-3;
    double constraintTolerance = 1e-4;
    bool addReserves = false;
    bool removeMuscles = false;
    double meshInterval = 0.0;

    argparse.Get("--trcFile"s, &trcFile);
    argparse.Get("--osimFile"s, &osimFile);
    argparse.Get("--outputFolder"s, &outputFolder);
    argparse.Get("--experimentName"s, &experimentName);
    argparse.Get("--weightsFile"s, &weightsFile);
    argparse.Get("--startTime"s, &startTime);
    argparse.Get("--endTime"s, &endTime);
    argparse.Get("--meshIntervals"s, &meshIntervals);
    argparse.Get("--maxIterations"s, &maxIterations);
    argparse.Get("--reservesOptimalForce"s, &reservesOptimalForce);
    argparse.Get("--markerTrackingWeight"s, &markerTrackingWeight);
    argparse.Get("--actuatorActivationWeight"s, &actuatorActivationWeight);
    argparse.Get("--convergenceTolerance"s, &convergenceTolerance);
    argparse.Get("--constraintTolerance"s, &constraintTolerance);
    argparse.Get("--addReserves"s, &addReserves);
    argparse.Get("--removeMuscles"s, &removeMuscles);

    Tracker tracker;
    tracker.setTrcFile(trcFile);
    tracker.setOsimFile(osimFile);
    tracker.setOutputFolder(outputFolder);
    tracker.setExperimentName(experimentName);
    tracker.setWeightsFile(weightsFile);
    tracker.setStartTime(startTime);
    tracker.setEndTime(endTime);
    tracker.setMeshIntervals(meshIntervals);
    tracker.setMaxInterations(maxIterations);
    tracker.setReservesOptimalForce(reservesOptimalForce);
    tracker.setMarkerTrackingWeight(markerTrackingWeight);
    tracker.setActuatorActivationWeight(actuatorActivationWeight);
    tracker.setConvergenceTolerance(convergenceTolerance);
    tracker.setConstraintTolerance(constraintTolerance);
    tracker.setAddReserves(addReserves);
    tracker.setRemoveMuscles(removeMuscles);

    // these only get set if they are specified
    if (argparse.Get("--meshInterval"s, &meshInterval)) { tracker.setMeshInterval(meshInterval); }

    std::string *errPtr = tracker.run();
    if (err) std::cerr << *errPtr;

    return 0;
}

