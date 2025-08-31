#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"

// 클래스 전방 선언
class G4VPhysicalVolume;
class G4LogicalVolume;
class G4Material;
class DetectorMessenger;

/**
 * @class DetectorConstruction
 * @brief 시뮬레이션 환경의 모든 물질과 기하학적 구조를 생성하는 클래스.
 *
 * [수정사항]
 * 1. 검출기-선원 간 거리(fDetectorDistance)를 멤버 변수로 추가.
 * 2. 매크로에서 거리와 각도를 모두 제어하는 Setter 함수 제공.
 */
class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
  DetectorConstruction();
  virtual ~DetectorConstruction();

  virtual G4VPhysicalVolume* Construct() override;
  virtual void ConstructSDandField() override;

  // Messenger에서 호출할 Public Setter 함수
  void SetMovablePMTAngle(G4double angle);
  void SetDetectorDistance(G4double distance);

private:
  // 내부 헬퍼 함수 선언
  void DefineMaterials();
  G4LogicalVolume* ConstructDetectorUnit();
  G4LogicalVolume* ConstructPMT();

  // --- 물질 포인터 ---
  G4Material* fAirMaterial;
  G4Material* fLsMaterial;
  G4Material* fCo60Material;
  G4Material* fGlassMaterial;
  G4Material* fPmtBodyMaterial;
  G4Material* fSiliconeMaterial;
  G4Material* fVacuumMaterial;
  G4Material* fEpoxyMaterial; 

  // --- 지오메트리 제어용 멤버 변수 ---
  G4double fMovablePMTAngle;
  G4double fDetectorDistance;

  // --- 기타 멤버 변수 ---
  DetectorMessenger* fDetectorMessenger;

  // 다른 함수에서 SD를 부착할 수 있도록 논리 볼륨 포인터를 멤버 변수로 선언
  G4LogicalVolume* logicLS;
  G4LogicalVolume* logicPhotocathode;
};

#endif
