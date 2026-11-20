#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "HitRecord.hh"
#include "globals.hh"
#include <vector>

class RunAction;

/// Collects this event's hits and applies the coincidence trigger.
/// An event is kept only if the primary muon left exactly one hit in
/// EVERY one of the N plates. Slanted muons that exit through the side
/// of the stack are therefore rejected, which is what a real coincidence
/// trigger does.
class EventAction : public G4UserEventAction
{
public:
  explicit EventAction(RunAction* runAction);
  ~EventAction() override = default;

  void BeginOfEventAction(const G4Event*) override;
  void EndOfEventAction(const G4Event*) override;

  void AddHit(const HitRecord& h) { fHits.push_back(h); }

private:
  RunAction* fRunAction;
  std::vector<HitRecord> fHits;
};

#endif
