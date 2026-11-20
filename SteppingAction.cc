#include "SteppingAction.hh"
#include "EventAction.hh"

#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"
#include "G4TouchableHandle.hh"
#include "G4SystemOfUnits.hh"

SteppingAction::SteppingAction(EventAction* eventAction)
  : fEventAction(eventAction) {}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
  const auto* track = step->GetTrack();

  // Primary muon only. Secondaries (delta rays) are not part of the
  // trajectory a real tracker would fit.
  if (track->GetTrackID() != 1) return;

  auto* pre = step->GetPreStepPoint();

  // Only the step that ENTERS a volume, so each plate yields one hit.
  if (pre->GetStepStatus() != fGeomBoundary) return;

  const auto* touch = pre->GetTouchableHandle()();
  if (!touch) return;
  const auto* vol = touch->GetVolume();
  if (!vol || vol->GetName() != "Layer") return;

  const G4ThreeVector p = pre->GetPosition();

  HitRecord h;
  h.layer = vol->GetCopyNo();
  h.x = p.x() / mm;
  h.y = p.y() / mm;
  h.z = p.z() / mm;

  fEventAction->AddHit(h);
}
