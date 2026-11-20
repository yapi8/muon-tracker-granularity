#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"

class G4ParticleGun;
class G4Event;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
    PrimaryGeneratorAction(G4int nLayers);
    virtual ~PrimaryGeneratorAction();

    virtual void GeneratePrimaries(G4Event* event);

private:
    G4ParticleGun* fGun;
    G4double fSourceZ;    // z position of the muon source plane
};

#endif
