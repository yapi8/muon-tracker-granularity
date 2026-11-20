#include "PrimaryGeneratorAction.hh"
#include "DetectorConstruction.hh"   // for GeomConst

#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4Event.hh"
#include "Randomize.hh"

#include <cmath>

PrimaryGeneratorAction::PrimaryGeneratorAction(G4int /*nLayers*/)
 : G4VUserPrimaryGeneratorAction(),
   fGun(nullptr)
{
    fGun = new G4ParticleGun(1);

    G4ParticleDefinition* muon =
        G4ParticleTable::GetParticleTable()->FindParticle("mu-");
    fGun->SetParticleDefinition(muon);
    fGun->SetParticleEnergy(4.*GeV);   // representative sea-level muon energy

    // Source plane sits 10 cm above the top layer.
    // Stack height is FIXED, so the source position is the same for every N.
    fSourceZ = (GeomConst::kStackHeight_cm/2.0 + 10.0) * cm;
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
    // ---- zenith angle from the cos^2(theta) cosmic-ray distribution ----
    // Flux through a horizontal plane  ~ cos^2(theta) sin(theta) dtheta,
    // which integrates in closed form:
    //     cos(theta) = cbrt( 1 - u * (1 - cos^3(theta_max)) )
    const G4double cosMax  = std::cos(GeomConst::kThetaMax_deg * deg);
    const G4double cosMax3 = cosMax * cosMax * cosMax;

    G4double u    = G4UniformRand();
    G4double cosT = std::cbrt(1.0 - u * (1.0 - cosMax3));
    G4double sinT = std::sqrt(std::max(0.0, 1.0 - cosT*cosT));
    G4double phi  = 2.0 * CLHEP::pi * G4UniformRand();

    // direction points DOWNWARD (negative z)
    G4ThreeVector dir(sinT*std::cos(phi), sinT*std::sin(phi), -cosT);

    // ---- uniform start position over the source area ----
    G4double half = GeomConst::kSourceHalf_cm * cm;
    G4double x0 = (G4UniformRand() - 0.5) * 2.0 * half;
    G4double y0 = (G4UniformRand() - 0.5) * 2.0 * half;

    fGun->SetParticlePosition(G4ThreeVector(x0, y0, fSourceZ));
    fGun->SetParticleMomentumDirection(dir);
    fGun->GeneratePrimaryVertex(event);
}
