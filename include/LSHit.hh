#ifndef LSHit_h
#define LSHit_h 1

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "G4String.hh"

/**
 * @class LSHit
 * @brief LS 또는 PMT 윈도우에서 발생하는 에너지 증착(hit) 정보를 저장하는 데이터 클래스입니다.
 */
class LSHit : public G4VHit
{
public:
  LSHit();
  virtual ~LSHit();

  inline void* operator new(size_t);
  inline void  operator delete(void*);

  // Setters and Getters
  void SetTrackID(G4int id) { fTrackID = id; }
  G4int GetTrackID() const { return fTrackID; }

  void SetParentID(G4int id) { fParentID = id; }
  G4int GetParentID() const { return fParentID; }

  void SetParticleName(const G4String& name) { fParticleName = name; }
  const G4String& GetParticleName() const { return fParticleName; }
  
  void SetProcessName(const G4String& name) { fProcessName = name; }
  const G4String& GetProcessName() const { return fProcessName; }

  void SetVolumeName(const G4String& name) { fVolumeName = name; }
  const G4String& GetVolumeName() const { return fVolumeName; }

  void SetPosition(const G4ThreeVector& pos) { fPosition = pos; }
  const G4ThreeVector& GetPosition() const { return fPosition; }
  
  void SetTime(G4double t) { fTime = t; }
  G4double GetTime() const { return fTime; }

  void SetKineticEnergy(G4double e) { fKineticEnergy = e; }
  G4double GetKineticEnergy() const { return fKineticEnergy; }

  void SetEnergyDeposit(G4double edep) { fEnergyDeposit = edep; }
  G4double GetEnergyDeposit() const { return fEnergyDeposit; }

private:
  G4int         fTrackID;
  G4int         fParentID;
  G4String      fParticleName;
  G4String      fProcessName;
  G4String      fVolumeName;
  G4ThreeVector fPosition;
  G4double      fTime;
  G4double      fKineticEnergy;
  G4double      fEnergyDeposit;
};

typedef G4THitsCollection<LSHit> LSHitsCollection;
extern G4ThreadLocal G4Allocator<LSHit>* LSHitAllocator;

inline void* LSHit::operator new(size_t)
{
  if (!LSHitAllocator) LSHitAllocator = new G4Allocator<LSHit>;
  return (void*)LSHitAllocator->MallocSingle();
}

inline void LSHit::operator delete(void* aHit)
{
  LSHitAllocator->FreeSingle((LSHit*)aHit);
}

#endif
