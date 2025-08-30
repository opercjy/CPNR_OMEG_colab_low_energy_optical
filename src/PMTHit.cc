#include "PMTHit.hh"

G4ThreadLocal G4Allocator<PMTHit>* PMTHitAllocator = nullptr;

PMTHit::PMTHit() : G4VHit(), fPMTID(-1), fTime(0.) {}
PMTHit::~PMTHit() {}
