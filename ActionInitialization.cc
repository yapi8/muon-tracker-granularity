#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"

ActionInitialization::ActionInitialization(G4int nLayers)
  : fNLayers(nLayers) {}

void ActionInitialization::BuildForMaster() const
{
  SetUserAction(new RunAction(fNLayers));
}

void ActionInitialization::Build() const
{
  SetUserAction(new PrimaryGeneratorAction);

  auto* runAction = new RunAction(fNLayers);
  SetUserAction(runAction);

  auto* eventAction = new EventAction(runAction);
  SetUserAction(eventAction);

  SetUserAction(new SteppingAction(eventAction));
}
