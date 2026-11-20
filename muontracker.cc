// muontracker.cc - main entry point
//
// Batch mode (unchanged, this is what produced the results):
//     MuonTracker.exe <numberOfLayers> <numberOfEvents>
//     e.g.  MuonTracker.exe 16 20000
//
// Visualisation mode (new - adds a third argument):
//     MuonTracker.exe 16 0 vis
//     opens a Geant4 viewer, runs vis.mac, and lets you save an image.
//
// The only changes from the original file are the three vis-related
// includes and the block marked "visualisation mode" near the bottom.

#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4SystemOfUnits.hh"

#include "G4VisExecutive.hh"      // <-- added
#include "G4UIExecutive.hh"       // <-- added

#include "QGSP_BERT.hh"
#include "G4EmStandardPhysics_option4.hh"

#include "DetectorConstruction.hh"
#include "PrimaryGeneratorAction.hh"
#include "ScintillatorSD.hh"

#include <cstdlib>
#include <string>
#include <iostream>

int main(int argc, char** argv)
{
    // ---- read command line arguments ----
    G4int nLayers = (argc > 1) ? std::atoi(argv[1]) : 4;
    G4int nEvents = (argc > 2) ? std::atoi(argv[2]) : 10000;
    G4bool visMode = (argc > 3 && std::string(argv[3]) == "vis");   // <-- added

    std::cout << "=== MuonTracker: N = " << nLayers
              << " layers, " << nEvents << " events ===" << std::endl;

    // ---- run manager (serial mode keeps CSV writing simple) ----
    auto* runManager =
        G4RunManagerFactory::CreateRunManager(G4RunManagerType::Serial);

    // ---- 1. geometry ----
    runManager->SetUserInitialization(new DetectorConstruction(nLayers));

    // ---- 2. physics ----
    auto* physics = new QGSP_BERT;
    physics->ReplacePhysics(new G4EmStandardPhysics_option4());
    physics->SetDefaultCutValue(1.*mm);
    runManager->SetUserInitialization(physics);

    // ---- 3. primary muons ----
    runManager->SetUserAction(new PrimaryGeneratorAction(nLayers));

    runManager->Initialize();

    G4UImanager* UI = G4UImanager::GetUIpointer();

    // ================= visualisation mode ==========================
    if (visMode) {
        auto* visManager = new G4VisExecutive;
        visManager->Initialize();

        auto* ui = new G4UIExecutive(argc, argv);
        UI->ApplyCommand("/control/execute vis.mac");
        ui->SessionStart();

        delete ui;
        delete visManager;
        delete runManager;
        return 0;
    }
    // ===============================================================

    // ---- open the output CSV for this configuration ----
    std::string fname = "results/hits_N" + std::to_string(nLayers) + ".csv";
    ScintillatorSD::OpenFile(fname);

    // ---- run ----
    UI->ApplyCommand("/run/printProgress 1000");
    UI->ApplyCommand("/run/beamOn " + std::to_string(nEvents));

    ScintillatorSD::CloseFile();

    std::cout << "=== Finished. Data written to " << fname << " ===" << std::endl;

    delete runManager;
    return 0;
}
