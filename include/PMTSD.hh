#ifndef PMTSD_h
#define PMTSD_h 1

#include "G4VSensitiveDetector.hh"
#include "PMTHit.hh"

class G4Step;
class G4HCofThisEvent;

/**
 * @class PMTSD
 * @brief PMT의 광음극(photocathode) 역할을 하는 Sensitive Detector 입니다.
 */
class PMTSD : public G4VSensitiveDetector
{
public:
  PMTSD(const G4String& name);
  virtual ~PMTSD();

  virtual void Initialize(G4HCofThisEvent* hce) override;
  virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override;

private:
  PMTHitsCollection* fHitsCollection;
};

#endif
