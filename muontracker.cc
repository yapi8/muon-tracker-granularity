// ===================================================================
//  muontracker -- detector granularity scan for cosmic-ray muon
//                 track reconstruction
//
//  Usage:
//     muontracker <N_layers> <N_events>          batch, writes CSV
//     muontracker <N_layers> <N_events> vis      opens the OpenGL viewer
//
//  Examples:
//     muontracker 64 20000
//     muontracker 16 1 vis
// ===================================================================

#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"

#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

#include "QGSP_BERT.hh"
#include "G4EmStandardPhysics_option4.hh"

#include <string>
#include <sstream>
#include <iostream>
#include <ctime>

int main(int argc, char** argv)
{
  if (argc < 3) {
    std::cout <<
      "usage: muontracker <N_layers> <N_events> [vis]\n"
      "   e.g. muontracker 64 20000\n"
      "        muontracker 16 1 vis\n";
    return 1;
  }

  const G4int nLayers = std::stoi(argv[1]);
  const G4int nEvents = std::stoi(argv[2]);
  const G4bool visMode = (argc > 3 && std::string(argv[3]) == "vis");

  // Reproducible by default. Change or seed from the clock if you want
  // independent repeats.
  CLHEP::HepRandom::setTheSeed(1234);

  // Serial run manager: the CSV writer is a single stream, and a serial
  // run keeps event IDs and the random sequence reproducible.
  auto* runManager =
      G4RunManagerFactory::CreateRunManager(G4RunManagerType::Serial);

  runManager->SetUserInitialization(new DetectorConstruction(nLayers));

  // ---- physics ------------------------------------------------------
  // QGSP_BERT is the general-purpose hadronic list; the EM part is
  // replaced with option4, the "most accurate" EM set, because multiple
  // Coulomb scattering is the effect this study is measuring.
  auto* physics = new QGSP_BERT;
  physics->ReplacePhysics(new G4EmStandardPhysics_option4);
  physics->SetDefaultCutValue(1.0 * mm);
  runManager->SetUserInitialization(physics);

  runManager->SetUserInitialization(new ActionInitialization(nLayers));

  auto* ui = G4UImanager::GetUIpointer();

  if (visMode) {
    auto* visManager = new G4VisExecutive;
    visManager->Initialize();

    auto* session = new G4UIExecutive(argc, argv);
    ui->ApplyCommand("/control/execute macros/vis.mac");

    std::ostringstream cmd;
    cmd << "/run/beamOn " << nEvents;
    ui->ApplyCommand(cmd.str());

    session->SessionStart();

    delete session;
    delete visManager;
  }
  else {
    ui->ApplyCommand("/run/initialize");
    ui->ApplyCommand("/run/printProgress 2000");

    std::ostringstream cmd;
    cmd << "/run/beamOn " << nEvents;
    ui->ApplyCommand(cmd.str());
  }

  delete runManager;
  return 0;
}
