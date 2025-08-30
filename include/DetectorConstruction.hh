#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;
class G4Material;
class DetectorMessenger; // UI 명령어를 위한 메신저 클래스 전방 선언

/**
 * @class DetectorConstruction
 * @brief 시뮬레이션 환경의 모든 물질과 기하학적 구조를 생성하는 클래스입니다.
 *
 * 두 개의 PMT를 배치하며, DetectorMessenger를 통해 매크로에서
 * 각 PMT의 거리와 회전 PMT의 각도를 제어하는 기능을 포함합니다.
 */
class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
  DetectorConstruction();
  virtual ~DetectorConstruction();

  virtual G4VPhysicalVolume* Construct() override;
  virtual void ConstructSDandField() override;
  
  // --- Messenger에서 호출할 Public Setter 함수들 ---
  void SetMovablePMTAngle(G4double angle);

private:
  void DefineMaterials();
  //검출기 유닛 전체를 생성하는 헬퍼 함수로 변경
  G4LogicalVolume* ConstructDetectorUnit();

  // --- 물질 포인터 ---
  G4Material* fAirMaterial;
  G4Material* fLsMaterial;
  G4Material* fCo60Material;
  G4Material* fGlassMaterial;
  G4Material* fPmtBodyMaterial;
  G4Material* fSiliconeMaterial;
  G4Material* fVacuumMaterial;

  // --- 지오메트리 제어용 멤버 변수 ---
  G4double fMovablePMTAngle; // 회전 PMT의 각도 (Theta)
  
  // --- 기타 멤버 변수 ---
  DetectorMessenger* fDetectorMessenger;
  G4LogicalVolume* logicPhotocathode; // 여러 함수에서 공유
};

#endif
