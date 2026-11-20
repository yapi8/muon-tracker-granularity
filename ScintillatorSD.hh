#ifndef ScintillatorSD_h
#define ScintillatorSD_h 1

#include "G4VSensitiveDetector.hh"
#include "globals.hh"
#include <fstream>

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

class ScintillatorSD : public G4VSensitiveDetector
{
public:
    ScintillatorSD(const G4String& name);
    virtual ~ScintillatorSD();

    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history);

    static void OpenFile(const G4String& filename);
    static void CloseFile();

private:
    static std::ofstream fOut;
};

#endif
