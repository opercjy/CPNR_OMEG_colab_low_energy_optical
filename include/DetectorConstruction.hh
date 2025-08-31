#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

// 클래스 전방 선언
class G4VPhysicalVolume;
class G4LogicalVolume;
class G4Material;
class DetectorMessenger;

/**
 * @class DetectorConstruction
 * @brief 시뮬레이션 환경의 모든 물질과 기하학적 구조를 생성하는 클래스.
 */
class DetectorConstruction : public G4VUserDetectorConstruction
{
// ... (Public/Private 함수 및 멤버 변수 선언 생략, 이전 버전과 동일하게 유지) ...
public:
  DetectorConstruction();
  virtual ~DetectorConstruction();
  virtual G4VPhysicalVolume* Construct() override;
  virtual void ConstructSDandField() override;
  void SetMovablePMTAngle(G4double angle);
  void SetDetectorDistance(G4double distance);

private:
  void DefineMaterials();
  void ValidateMaterials();
  void DefineOpticalProperties();
  G4LogicalVolume* ConstructDetectorUnit();
  G4LogicalVolume* ConstructPMT();

  // 물질 포인터 및 멤버 변수들...
  G4Material* fAirMaterial; G4Material* fLsMaterial; G4Material* fCo60Material;
  G4Material* fGlassMaterial; G4Material* fPmtBodyMaterial; G4Material* fSiliconeMaterial;
  G4Material* fVacuumMaterial; G4Material* fEpoxyMaterial; 

  G4double fMovablePMTAngle; // 사잇각 (θ)
  G4double fDetectorDistance; // 거리 (r): 선원 중심 -> LS 중심

  DetectorMessenger* fDetectorMessenger;
  G4LogicalVolume* logicLS;
  G4LogicalVolume* logicPhotocathode;

public:
  // === [!재구성 완료!] 지오메트리 상수 정의 (static constexpr) ===

  // --- 1. World & 기본 설정 (변경 없음) ---
  static constexpr G4double kWorldHalfSize = 1.5*CLHEP::m;
  static constexpr G4double kDefaultDistance = 20.0*CLHEP::cm;
  static constexpr G4double kDefaultAngle = 45.0*CLHEP::deg;

  // --- 2. 선원 및 캡슐 (변경 없음) ---
  static constexpr G4double kSourceRadius = 2.5*CLHEP::mm;
  static constexpr G4double kSourceHalfZ = 0.5*CLHEP::mm;
  static constexpr G4double kEpoxyRadius = 12.5*CLHEP::mm;
  static constexpr G4double kEpoxyHalfZ = 1.5*CLHEP::mm;

  // --- 4, 5, 6. LS, Grease, PMT 구성요소 (변경 없음) ---
  static constexpr G4double kBottleOuterRadius = 28.0*CLHEP::mm;
  static constexpr G4double kBottleThickness = 2.0*CLHEP::mm;
  static constexpr G4double kLSHalfZ = 40.0*CLHEP::mm;
  static constexpr G4double kBottleInnerRadius = kBottleOuterRadius - kBottleThickness;
  static constexpr G4double kGreaseHalfZ = 0.5*CLHEP::mm;
  static constexpr G4double kPmtAssemblyRadius = 30.0*CLHEP::mm;
  static constexpr G4double kPmtAssemblyHalfZ = 107.5*CLHEP::mm;
  // PMT 내부 상수들 (변경 없음)
  static constexpr G4double kPmtWindowRadius = 26.5*CLHEP::mm;
  static constexpr G4double kPmtWindowHalfZ = 2.5*CLHEP::mm;
  static constexpr G4double kPhotocathodeHalfZ = 0.05*CLHEP::mm;
  static constexpr G4double kPmtBodyRadius = 29.5*CLHEP::mm;
  static constexpr G4double kPmtBodyThickness = 1.5*CLHEP::mm;
  static constexpr G4double kPmtInnerRadius = kPmtBodyRadius - kPmtBodyThickness;
  static constexpr G4double kPmtBodyHalfZ = kPmtAssemblyHalfZ - kPmtWindowHalfZ;

  // --- 3. [!핵심 수정!] 검출기 유닛 조립체 (Envelope) 최적화 ---
  static constexpr G4double kAssemblyRadius = 35*CLHEP::mm;
  
  // [!개선!] 조립체 길이 및 중심 오프셋 계산 (Tightly Wrapping)
  // 전체 길이 계산: LS(80) + Grease(1) + PMT(215) = 296 mm
  static constexpr G4double kAssemblyTotalLength = 2*kLSHalfZ + 2*kGreaseHalfZ + 2*kPmtAssemblyHalfZ;
  // 조립체 절반 길이 (148 mm)
  static constexpr G4double kAssemblyHalfZ = kAssemblyTotalLength / 2.0;

  // LS 중심(기준점)과 조립체 기하학적 중심(배치점) 사이의 오프셋 계산
  // 오프셋 = 조립체 절반 길이 - LS 절반 길이 = 148 - 40 = 108 mm
  static constexpr G4double kAssemblyCenterOffset = kAssemblyHalfZ - kLSHalfZ;
  // ==============================================================
};

#endif
