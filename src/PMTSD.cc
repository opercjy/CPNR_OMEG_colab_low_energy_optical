#include "PMTSD.hh"
#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4OpticalPhoton.hh"
#include "G4SystemOfUnits.hh"
#include "G4SDManager.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4ios.hh"
#include "Randomize.hh"

PMTSD::PMTSD(const G4String& name)
: G4VSensitiveDetector(name), fHitsCollection(nullptr)
{
  collectionName.insert("PMTHitsCollection");
}

PMTSD::~PMTSD() {}

void PMTSD::Initialize(G4HCofThisEvent* hce)
{
  fHitsCollection = new PMTHitsCollection(SensitiveDetectorName, collectionName[0]);
  G4int hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  hce->AddHitsCollection(hcID, fHitsCollection);
}

G4bool PMTSD::ProcessHits(G4Step* aStep, G4TouchableHistory* /*ROhist*/)
{
  G4Track* track = aStep->GetTrack();
  if (track->GetDefinition() != G4OpticalPhoton::Definition()) return false;
  if (aStep->GetPreStepPoint()->GetStepStatus() != fGeomBoundary) return false;

  // 광음극 표면의 광학 속성 테이블에서 양자 효율(QE)을 읽어옵니다.
  G4MaterialPropertiesTable* pmtMPT = aStep->GetPreStepPoint()->GetMaterial()->GetMaterialPropertiesTable();
  if (!pmtMPT) return false;

  G4MaterialPropertyVector* qeVector = pmtMPT->GetProperty("EFFICIENCY");
  if (!qeVector) return false;

  G4double photonEnergy = track->GetKineticEnergy();
  G4double quantumEfficiency = qeVector->Value(photonEnergy);

  if (G4UniformRand() > quantumEfficiency) {
    track->SetTrackStatus(fStopAndKill);
    return false;
  }

  PMTHit* newHit = new PMTHit();
  newHit->SetPMTID(aStep->GetPreStepPoint()->GetTouchable()->GetCopyNumber());
  newHit->SetTime(aStep->GetPostStepPoint()->GetGlobalTime() / ns);
  fHitsCollection->insert(newHit);
  track->SetTrackStatus(fStopAndKill);

  return true;
}
