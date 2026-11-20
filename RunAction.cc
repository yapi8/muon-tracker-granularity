#include "RunAction.hh"

#include "G4Run.hh"
#include "G4SystemOfUnits.hh"
#include "globals.hh"
#include <filesystem>
#include <iomanip>
#include <sstream>

RunAction::RunAction(G4int nLayers)
  : fNLayers(nLayers), fAccepted(0), fRejected(0) {}

void RunAction::BeginOfRunAction(const G4Run*)
{
  fAccepted = 0;
  fRejected = 0;

  std::error_code ec;
  std::filesystem::create_directories("results", ec);

  std::ostringstream name;
  name << "results/hits_N" << fNLayers << ".csv";

  fOut.open(name.str());
  if (!fOut.is_open()) {
    G4cerr << "[RunAction] ERROR: cannot open " << name.str() << G4endl;
    return;
  }
  fOut << "event,layer,x_mm,y_mm,z_mm\n";
  fOut << std::fixed << std::setprecision(6);

  G4cout << "[RunAction] writing " << name.str() << G4endl;
}

void RunAction::WriteEvent(G4int eventID, const std::vector<HitRecord>& hits)
{
  if (!fOut.is_open()) return;
  for (const auto& h : hits) {
    fOut << eventID << ',' << h.layer << ','
         << h.x << ',' << h.y << ',' << h.z << '\n';
  }
  ++fAccepted;
}

void RunAction::EndOfRunAction(const G4Run* run)
{
  if (fOut.is_open()) fOut.close();

  const G4long total = run->GetNumberOfEvent();
  const G4double acc = (total > 0) ? 100.0 * G4double(fAccepted) / G4double(total) : 0.0;

  G4cout << "\n---------------- run summary ----------------\n"
         << "  layers N        : " << fNLayers  << "\n"
         << "  events thrown   : " << total     << "\n"
         << "  events accepted : " << fAccepted << "\n"
         << "  events rejected : " << fRejected << "\n"
         << "  acceptance      : " << std::fixed << std::setprecision(2)
         << acc << " %\n"
         << "---------------------------------------------\n" << G4endl;
}
