#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "G4UserSteppingAction.hh"
#include "globals.hh"

class EventAction;

/// Records the entry point of the PRIMARY muon into each plate.
/// Only trackID == 1 is recorded: delta rays and other secondaries are
/// not what the tracker would reconstruct as the muon trajectory.
class SteppingAction : public G4UserSteppingAction
{
public:
  explicit SteppingAction(EventAction* eventAction);
  ~SteppingAction() override = default;

  void UserSteppingAction(const G4Step*) override;

private:
  EventAction* fEventAction;
};

#endif
