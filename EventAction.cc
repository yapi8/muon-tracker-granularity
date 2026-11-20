#include "EventAction.hh"
#include "RunAction.hh"
#include "DetectorConstruction.hh"

#include "G4Event.hh"
#include "G4RunManager.hh"
#include <algorithm>
#include <set>

EventAction::EventAction(RunAction* runAction)
  : fRunAction(runAction) {}

void EventAction::BeginOfEventAction(const G4Event*)
{
  fHits.clear();
}

void EventAction::EndOfEventAction(const G4Event* event)
{
  const auto* det = static_cast<const DetectorConstruction*>(
      G4RunManager::GetRunManager()->GetUserDetectorConstruction());
  const G4int nLayers = det->GetNLayers();

  // ---- coincidence trigger -----------------------------------------
  // Require exactly one hit in every plate. Muons that leave through the
  // side of the stack produce fewer, and are rejected.
  if (G4int(fHits.size()) != nLayers) {
    fRunAction->CountRejected();
    return;
  }

  std::set<G4int> seen;
  for (const auto& h : fHits) seen.insert(h.layer);
  if (G4int(seen.size()) != nLayers) {
    fRunAction->CountRejected();
    return;
  }

  // Sort top -> bottom so the analysis can split halves by index.
  std::sort(fHits.begin(), fHits.end(),
            [](const HitRecord& a, const HitRecord& b) {
              return a.layer < b.layer;
            });

  fRunAction->WriteEvent(event->GetEventID(), fHits);
}
