#include "LSHit.hh"

// LSHit 객체를 위한 메모리 할당자를 G4ThreadLocal로 선언 및 초기화합니다.
G4ThreadLocal G4Allocator<LSHit>* LSHitAllocator = nullptr;

LSHit::LSHit()
: G4VHit(),
  fTrackID(0), fParentID(0),
  fParticleName(""), fProcessName(""), fVolumeName(""),
  fPosition(0,0,0), fTime(0.),
  fKineticEnergy(0.), fEnergyDeposit(0.)
{}

LSHit::~LSHit()
{}
