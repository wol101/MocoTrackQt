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
    argparse.Get("--trcFile"s, &trcFile);
    argparse.Get("--osimFile"s, &osimFile);
    argparse.Get("--outputFolder"s, &outputFolder);
    argparse.Get("--experimentName"s, &experimentName);
    argparse.Get("--startTime"s, &startTime);
    argparse.Get("--endTime"s, &endTime);
    argparse.Get("--meshInterval"s, &meshInterval);

    Tracker tracker;
    tracker.setTrcFile(trcFile);
    tracker.setOsimFile(osimFile);
    tracker.setOutputFolder(outputFolder);
    tracker.setExperimentName(experimentName);
    tracker.setStartTime(startTime);
    tracker.setEndTime(endTime);
    tracker.setMeshInterval(meshInterval);

    std::string *errPtr = tracker.run();
    if (err) std::cerr << *errPtr;

    return 0;
}

