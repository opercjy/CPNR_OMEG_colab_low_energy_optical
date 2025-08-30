#ifndef LSSD_h
#define LSSD_h 1

#include "G4VSensitiveDetector.hh"
#include "LSHit.hh"

class G4Step;
class G4HCofThisEvent;

/**
 * @class LSSD
 * @brief LS와 PMT 윈도우의 에너지 증착을 감지하는 Sensitive Detector 클래스입니다.
 */
class LSSD : public G4VSensitiveDetector
{
public:
  LSSD(const G4String& name);
  virtual ~LSSD();

  virtual void Initialize(G4HCofThisEvent* hce) override;
  virtual G4bool ProcessHits(G4Step* aStep, G4TouchableHistory* ROhist) override;

private:
  LSHitsCollection* fHitsCollection;
};

#endif
