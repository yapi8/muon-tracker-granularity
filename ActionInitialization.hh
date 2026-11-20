#ifndef ActionInitialization_h
#define ActionInitialization_h 1

#include "G4VUserActionInitialization.hh"
#include "globals.hh"

class ActionInitialization : public G4VUserActionInitialization
{
public:
  explicit ActionInitialization(G4int nLayers);
  ~ActionInitialization() override = default;

  void BuildForMaster() const override;
  void Build() const override;

private:
  G4int fNLayers;
};

#endif
