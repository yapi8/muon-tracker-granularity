#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class G4LogicalVolume;
class G4VPhysicalVolume;

// Geometry constants shared with the primary generator.
// The TOTAL STACK HEIGHT is fixed for every configuration; the spacing
// between layers shrinks as N grows.  This isolates granularity (N) as
// the only variable, keeping the detector's lever arm constant.
namespace GeomConst {
    const double kStackHeight_cm   = 100.0;   // top layer centre -> bottom layer centre
    const double kLayerThick_cm    = 0.1;     // 1 mm per scintillator layer
    const double kPlateHalfSize_cm = 50.0;    // 100 x 100 cm plates
    const double kSourceHalf_cm    = 20.0;    // muon source area half-width
    const double kThetaMax_deg     = 30.0;    // zenith angle cut
}

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    DetectorConstruction(G4int nLayers);
    virtual ~DetectorConstruction();

    virtual G4VPhysicalVolume* Construct();
    virtual void ConstructSDandField();

    static G4double LayerSpacing_cm(G4int nLayers);

private:
    G4int fNumLayers;                 // independent variable
    G4LogicalVolume* fLogicScint;
};

#endif
