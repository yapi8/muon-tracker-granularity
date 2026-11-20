#ifndef HitRecord_h
#define HitRecord_h 1

#include "globals.hh"

/// One primary-muon crossing of one scintillator plate.
struct HitRecord {
  G4int    layer;  // 0 = top plate
  G4double x;      // mm, TRUE crossing position (no smearing applied here)
  G4double y;      // mm
  G4double z;      // mm
};

#endif
