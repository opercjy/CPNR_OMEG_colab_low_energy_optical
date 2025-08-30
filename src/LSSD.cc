#include "LSSD.hh"
#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"
#include "G4SystemOfUnits.hh"
#include "G4SDManager.hh"

LSSD::LSSD(const G4String& name)
: G4VSensitiveDetector(name), fHitsCollection(nullptr)
{
  collectionName.insert("LSHitsCollection");
}

LSSD::~LSSD()
{}

void LSSD::Initialize(G4HCofThisEvent* hce)
{
  fHitsCollection = new LSHitsCollection(SensitiveDetectorName, collectionName[0]);
  G4int hcID = GetCollectionID(0);
  hce->AddHitsCollection(hcID, fHitsCollection);
}

G4bool LSSD::ProcessHits(G4Step* aStep, G4TouchableHistory* /*ROhist*/)
{
  if (aStep->GetTotalEnergyDeposit() == 0.) return false;

  LSHit* newHit = new LSHit();
  G4Track* track = aStep->GetTrack();
  
  newHit->SetTrackID(track->GetTrackID());
  newHit->SetParentID(track->GetParentID());
  newHit->SetParticleName(track->GetDefinition()->GetParticleName());
  newHit->SetVolumeName(track->GetVolume()->GetLogicalVolume()->GetName());

  const G4VProcess* creatorProcess = track->GetCreatorProcess();
  if (creatorProcess) {
    newHit->SetProcessName(creatorProcess->GetProcessName());
  } else {
    newHit->SetProcessName("primary");
  }

  newHit->SetPosition(aStep->GetPreStepPoint()->GetPosition());
  newHit->SetTime(aStep->GetPreStepPoint()->GetGlobalTime() / ns);
  newHit->SetKineticEnergy(aStep->GetPreStepPoint()->GetKineticEnergy() / MeV);
  newHit->SetEnergyDeposit(aStep->GetTotalEnergyDeposit() / MeV);

  fHitsCollection->insert(newHit);

  return true;
}
