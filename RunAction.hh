#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"
#include "HitRecord.hh"
#include "globals.hh"
#include <fstream>
#include <vector>

/// Opens results/hits_N<N>.csv and streams accepted events into it.
class RunAction : public G4UserRunAction
{
public:
  explicit RunAction(G4int nLayers);
  ~RunAction() override = default;

  void BeginOfRunAction(const G4Run*) override;
  void EndOfRunAction(const G4Run*) override;

  void WriteEvent(G4int eventID, const std::vector<HitRecord>& hits);
  void CountRejected() { ++fRejected; }

private:
  G4int         fNLayers;
  std::ofstream fOut;
  G4long        fAccepted;
  G4long        fRejected;
};

#endif
