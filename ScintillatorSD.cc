#include "ScintillatorSD.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4StepPoint.hh"
#include "G4StepStatus.hh"
#include "G4TouchableHistory.hh"
#include "G4RunManager.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"

#include <iostream>

std::ofstream ScintillatorSD::fOut;

ScintillatorSD::ScintillatorSD(const G4String& name)
 : G4VSensitiveDetector(name)
{}

ScintillatorSD::~ScintillatorSD() {}

void ScintillatorSD::OpenFile(const G4String& filename)
{
    fOut.open(filename.c_str());
    if (!fOut.is_open()) {
        std::cerr << "ERROR: could not open output file " << filename << std::endl;
        return;
    }
    fOut << "eventID,layerID,x_cm,y_cm,z_cm\n";
}

void ScintillatorSD::CloseFile()
{
    if (fOut.is_open()) fOut.close();
}

G4bool ScintillatorSD::ProcessHits(G4Step* step, G4TouchableHistory*)
{
    // only the primary muon (track ID 1), ignore secondaries
    if (step->GetTrack()->GetTrackID() != 1) return false;

    G4StepPoint* pre = step->GetPreStepPoint();

    // record only the ENTRY step into the layer, not every step inside it
    if (pre->GetStepStatus() != fGeomBoundary) return false;

    G4int layerID = pre->GetTouchableHandle()->GetCopyNumber();
    G4ThreeVector pos = pre->GetPosition();

    G4int eventID =
        G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID();

    if (fOut.is_open()) {
        fOut << eventID << ","
             << layerID << ","
             << pos.x()/cm << ","
             << pos.y()/cm << ","
             << pos.z()/cm << "\n";
    }

    return true;
}
