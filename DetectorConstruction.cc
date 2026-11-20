#include "DetectorConstruction.hh"
#include "ScintillatorSD.hh"

#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4VPhysicalVolume.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

#include <iostream>

G4double DetectorConstruction::LayerSpacing_cm(G4int nLayers)
{
    if (nLayers < 2) return 0.;
    return GeomConst::kStackHeight_cm / (nLayers - 1);
}

DetectorConstruction::DetectorConstruction(G4int nLayers)
 : G4VUserDetectorConstruction(),
   fNumLayers(nLayers),
   fLogicScint(nullptr)
{}

DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    G4NistManager* nist = G4NistManager::Instance();
    G4Material* air      = nist->FindOrBuildMaterial("G4_AIR");
    G4Material* scintMat = nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");

    const G4double stackH  = GeomConst::kStackHeight_cm * cm;   // FIXED for all N
    const G4double thick   = GeomConst::kLayerThick_cm  * cm;
    const G4double halfXY  = GeomConst::kPlateHalfSize_cm * cm;
    const G4double spacing = LayerSpacing_cm(fNumLayers) * cm;  // shrinks as N grows

    std::cout << "  Stack height : " << GeomConst::kStackHeight_cm << " cm (fixed)\n"
              << "  Layers       : " << fNumLayers << "\n"
              << "  Spacing      : " << LayerSpacing_cm(fNumLayers) << " cm\n"
              << "  Layer thick  : " << GeomConst::kLayerThick_cm << " cm" << std::endl;

    // ---------- world volume ----------
    G4double worldZ = stackH + 60.*cm;
    G4Box* solidWorld = new G4Box("World", halfXY + 10.*cm, halfXY + 10.*cm, worldZ/2.);
    G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, air, "World");
    G4VPhysicalVolume* physWorld =
        new G4PVPlacement(nullptr, G4ThreeVector(), logicWorld,
                          "World", nullptr, false, 0, true);
    logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());

    // ---------- one scintillator layer ----------
    G4Box* solidScint = new G4Box("Scintillator", halfXY, halfXY, thick/2.);
    fLogicScint = new G4LogicalVolume(solidScint, scintMat, "Scintillator");

    G4VisAttributes* blue = new G4VisAttributes(G4Colour(0.2, 0.5, 1.0, 0.4));
    blue->SetForceSolid(true);
    fLogicScint->SetVisAttributes(blue);

    // ---------- place N layers ----------
    // copy number 0 = TOP layer, N-1 = BOTTOM layer (muons enter from above)
    G4double zTop = stackH / 2.;
    for (G4int i = 0; i < fNumLayers; i++) {
        G4double z = zTop - i * spacing;
        new G4PVPlacement(nullptr, G4ThreeVector(0., 0., z),
                          fLogicScint, "Scintillator",
                          logicWorld, false, i, false);
    }

    return physWorld;
}

void DetectorConstruction::ConstructSDandField()
{
    ScintillatorSD* sd = new ScintillatorSD("ScintillatorSD");
    G4SDManager::GetSDMpointer()->AddNewDetector(sd);
    SetSensitiveDetector("Scintillator", sd);
}
