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
    argparse.AddArgument("-st"s, "--startTime"s, "Start time"s, "0.0"s, 1, false, ArgParse::Double);
    argparse.AddArgument("-et"s, "--endTime"s, "End time"s, "1.0"s, 1, false, ArgParse::Double);
    argparse.AddArgument("-mi"s, "--meshInterval"s, "Mesh interval"s, "0.02"s, 1, false, ArgParse::Double);
    argparse.AddArgument("-rof"s, "--reservesOptimalForce"s, "Reserves optimal force"s, "100.0"s, 1, false, ArgParse::Double);
    argparse.AddArgument("-gtw"s, "--globalTrackingWeight"s, "Global tracking weight"s, "10.0"s, 1, false, ArgParse::Double);
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
    double startTime = 0;
    double endTime = 1.0;
    double meshInterval = 0.02;
    double reservesOptimalForce = 100.0;
    double globalTrackingWeight = 10.0;
    double convergenceTolerance = 1e-3;
    double constraintTolerance = 1e-4;
    bool addReserves = false;
    bool removeMuscles = false;

    argparse.Get("--trcFile"s, &trcFile);
    argparse.Get("--osimFile"s, &osimFile);
    argparse.Get("--outputFolder"s, &outputFolder);
    argparse.Get("--experimentName"s, &experimentName);
    argparse.Get("--startTime"s, &startTime);
    argparse.Get("--endTime"s, &endTime);
    argparse.Get("--meshInterval"s, &meshInterval);
    argparse.Get("--reservesOptimalForce"s, &reservesOptimalForce);
    argparse.Get("--globalTrackingWeight"s, &globalTrackingWeight);
    argparse.Get("--convergenceTolerance"s, &convergenceTolerance);
    argparse.Get("--constraintTolerance"s, &constraintTolerance);
    argparse.Get("--addReserves"s, &addReserves);
    argparse.Get("--removeMuscles"s, &removeMuscles);

    Tracker tracker;
    tracker.setTrcFile(trcFile);
    tracker.setOsimFile(osimFile);
    tracker.setOutputFolder(outputFolder);
    tracker.setExperimentName(experimentName);
    tracker.setStartTime(startTime);
    tracker.setEndTime(endTime);
    tracker.setMeshInterval(meshInterval);
    tracker.setReservesOptimalForce(reservesOptimalForce);
    tracker.setGlobalTrackingWeight(globalTrackingWeight);
    tracker.setConvergenceTolerance(convergenceTolerance);
    tracker.setConstraintTolerance(constraintTolerance);
    tracker.setAddReserves(addReserves);
    tracker.setRemoveMuscles(removeMuscles);

    std::string *errPtr = tracker.run();
    if (err) std::cerr << *errPtr;

    return 0;
}

