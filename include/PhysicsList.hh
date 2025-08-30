#ifndef PhysicsList_h
#define PhysicsList_h 1

#include "G4VModularPhysicsList.hh"

/**
 * @class PhysicsList
 * @brief 시뮬레이션에 사용될 모든 물리 프로세스를 정의하고 등록하는 클래스입니다.
 *
 * G4VModularPhysicsList를 상속받아, 필요한 물리 모듈(전자기, 방사성 붕괴, 광학 등)을
 * 독립적으로 조합하여 사용합니다. Geant4에서 권장하는 현대적인 방식입니다.
 */
class PhysicsList : public G4VModularPhysicsList
{
public:
  PhysicsList();
  virtual ~PhysicsList();
};

#endif
