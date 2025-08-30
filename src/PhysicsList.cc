#include "PhysicsList.hh"

// Geant4에서 제공하는 표준 물리 모듈들을 포함합니다.
#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4EmStandardPhysics.hh"
#include "G4HadronPhysicsFTFP_BERT_HP.hh"
#include "G4HadronElasticPhysicsHP.hh"
#include "G4IonPhysics.hh"
#include "G4StoppingPhysics.hh"
#include "G4OpticalPhysics.hh"
#include "G4StepLimiterPhysics.hh"
#include "G4SystemOfUnits.hh"

/**
 * @brief 생성자: 필요한 물리 프로세스 모듈들을 등록합니다.
 *
 * G4VModularPhysicsList를 상속받아, 필요한 물리 모듈을 '부품'처럼 조립합니다.
 * 이 방식은 Geant4에서 권장하는 현대적인 방식으로, 특정 표준 물리 리스트(예: Shielding)에
 * 종속될 때 발생하는 UI 명령어 비활성화와 같은 예기치 않은 문제를 해결합니다.
 * 각 RegisterPhysics 호출은 시뮬레이션에 새로운 물리적 능력을 부여합니다.
 */
PhysicsList::PhysicsList() : G4VModularPhysicsList()
{
  // 1. 표준 전자기 물리 (Standard Electromagnetic Physics)
  // 감마선의 광전효과, 컴프턴 산란, 쌍생성 및 전자의 이온화, 제동복사, 다중산란 등
  // 모든 기본적인 전자기 상호작용을 처리하는 필수 모듈입니다.
  RegisterPhysics(new G4EmStandardPhysics());

  // 2. 일반 및 방사성 붕괴 물리 (Decay Physics)
  // 파이온, 뮤온 등 불안정한 기본 입자들의 붕괴를 처리합니다.
  RegisterPhysics(new G4DecayPhysics());
  // Co-60과 같은 방사성 동위원소의 붕괴를 처리합니다.
  // 이 모듈을 등록해야 /process/had/rdm/ 과 같은 UI 명령어를 사용할 수 있습니다.
  RegisterPhysics(new G4RadioactiveDecayPhysics());

  // 3. 강입자, 이온, 정지 관련 물리 (Hadronic, Ion, Stopping Physics)
  // FTFP_BERT 모델과 고정밀 중성자 모델(HP)을 결합한 강입자 상호작용을 등록합니다.
  RegisterPhysics(new G4HadronPhysicsFTFP_BERT_HP());
  // 고정밀 중성자 탄성 산란을 처리합니다.
  RegisterPhysics(new G4HadronElasticPhysicsHP());
  // 이온(alpha, 중이온 등)의 물리적 상호작용을 처리합니다.
  RegisterPhysics(new G4IonPhysics());
  // 입자가 정지할 때 일어나는 현상(예: 반양성자 소멸)을 처리합니다.
  RegisterPhysics(new G4StoppingPhysics());
  
  // 4. 광학 물리 및 스텝 제한자 (Optical & Step Limiter Physics)
  // 섬광(Scintillation), 체렌코프(Cerenkov), 흡수(Absorption),
  // 경계면에서의 반사/굴절 등 모든 광학 관련 프로세스를 등록합니다.
  RegisterPhysics(new G4OpticalPhysics());
  
  // 매크로에서 /process/eLoss/stepMax 명령어를 통해 입자의 최대 스텝 길이를
  // 제한할 수 있게 해줍니다. 광자 추적 시 유용하게 사용될 수 있습니다.
  RegisterPhysics(new G4StepLimiterPhysics());
  
  // 2차 입자 생성을 위한 기준 거리(Production Cut)를 1mm로 설정합니다.
  // 이 거리보다 짧은 거리를 날아가는 2차 입자는 생성되지 않고 에너지가 즉시 흡수됩니다.
  SetDefaultCutValue(1.0*mm);
}

/**
 * @brief 소멸자
 */
PhysicsList::~PhysicsList()
{}
