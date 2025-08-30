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
  // ... (다른 모든 Setters/Getters)

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
// ... (new/delete 구현은 이전 답변과 동일)
#endif
