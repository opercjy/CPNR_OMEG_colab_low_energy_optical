#include "PrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4GeneralParticleSource.hh" // GPS 헤더 파일 포함

/**
 * @brief 생성자 (Constructor)
 *
 * PrimaryGeneratorAction 객체가 생성될 때 호출됩니다.
 * G4GeneralParticleSource(GPS) 객체를 생성하여 멤버 변수에 할당합니다.
 * GPS는 Geant4에서 제공하는 가장 강력하고 유연한 입자 생성기입니다.
 */
PrimaryGeneratorAction::PrimaryGeneratorAction()
: G4VUserPrimaryGeneratorAction(), fGPS(nullptr)
{
  fGPS = new G4GeneralParticleSource();
}

/**
 * @brief 소멸자 (Destructor)
 *
 * PrimaryGeneratorAction 객체가 소멸될 때 호출됩니다.
 * 생성했던 GPS 객체를 삭제하여 메모리 누수를 방지합니다.
 */
PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fGPS;
}

/**
 * @brief GeneratePrimaries
 * @param anEvent 현재 이벤트에 대한 정보를 담고 있는 객체 포인터
 *
 * 매 이벤트 시작 시 Geant4 커널에 의해 호출됩니다.
 * GPS의 GeneratePrimaryVertex() 함수를 호출하여, 매크로 파일(.mac)에 정의된
 * 설정에 따라 초기 입자를 생성하고 현재 이벤트(anEvent)에 주입합니다.
 * 이 방식은 C++ 코드를 재컴파일하지 않고도 소스의 종류, 위치, 에너지, 방출 각도 등
 * 복잡한 설정을 자유롭게 변경할 수 있게 해주는 장점이 있습니다.
 */
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  fGPS->GeneratePrimaryVertex(anEvent);
}
