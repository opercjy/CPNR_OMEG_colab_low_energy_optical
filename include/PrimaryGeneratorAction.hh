#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"

class G4GeneralParticleSource;
class G4Event;

/**
 * @class PrimaryGeneratorAction
 * @brief 각 이벤트의 초기 입자를 생성하는 클래스입니다.
 *
 * G4GeneralParticleSource(GPS)를 사용하여, 매크로 파일에서 소스를 유연하게 제어합니다.
 */
class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
  PrimaryGeneratorAction();
  virtual ~PrimaryGeneratorAction();

  virtual void GeneratePrimaries(G4Event* anEvent) override;

private:
  G4GeneralParticleSource* fGPS;
};

#endif
