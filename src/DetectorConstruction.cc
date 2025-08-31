#include "DetectorConstruction.hh"

// --- Geant4 헤더 파일 ---
#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4Element.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4SDManager.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

// --- 사용자 정의 클래스 헤더 ---
#include "PMTSD.hh"
#include "LSSD.hh"

// --- Messenger (UI Command) 헤더 ---
#include "G4UImessenger.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"

// std::vector 및 오류 메시지 구성을 위한 헤더
#include <vector>
#include <sstream>
#include <string>
#include <utility>
#include <cmath>

using namespace CLHEP;

// ============================================================================
// === DetectorMessenger 클래스 구현 (변경 없음) ===
// ============================================================================
class DetectorMessenger : public G4UImessenger
{
public:
  DetectorMessenger(DetectorConstruction* det) : fDetector(det)
  {
    fDirectory = new G4UIdirectory("/myApp/detector/");
    fDirectory->SetGuidance("Detector geometry control commands.");

    fSetAngleCmd = new G4UIcmdWithADoubleAndUnit("/myApp/detector/setMovableAngle", this);
    // [!개선!] Guidance 메시지 명확화
    fSetAngleCmd->SetGuidance("Set the angle (theta) of the movable PMT (CCW from +X towards +Z).");
    fSetAngleCmd->SetParameterName("Angle", false);
    fSetAngleCmd->SetUnitCategory("Angle");
    fSetAngleCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

    fSetDistanceCmd = new G4UIcmdWithADoubleAndUnit("/myApp/detector/setDistance", this);
    fSetDistanceCmd->SetGuidance("Set the distance (R) between the source and detectors.");
    fSetDistanceCmd->SetParameterName("Distance", false);
    fSetDistanceCmd->SetUnitCategory("Length");
    fSetDistanceCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
  }
 
  virtual ~DetectorMessenger()
  {
    delete fSetAngleCmd;
    delete fSetDistanceCmd;
    delete fDirectory;
  }

  virtual void SetNewValue(G4UIcommand* command, G4String newValue) override
  {
    if (command == fSetAngleCmd) {
      fDetector->SetMovablePMTAngle(fSetAngleCmd->GetNewDoubleValue(newValue));
    } else if (command == fSetDistanceCmd) {
      fDetector->SetDetectorDistance(fSetDistanceCmd->GetNewDoubleValue(newValue));
    }
  }

private:
  DetectorConstruction* fDetector;
  G4UIdirectory* fDirectory;
  G4UIcmdWithADoubleAndUnit* fSetAngleCmd;
  G4UIcmdWithADoubleAndUnit* fSetDistanceCmd;
};


// ============================================================================
// === DetectorConstruction 클래스 구현 ===
// ============================================================================

/**
 * @brief 생성자 (이전과 동일)
 */
DetectorConstruction::DetectorConstruction()
 : G4VUserDetectorConstruction(),
   // 모든 포인터는 nullptr로 명시적 초기화
   fAirMaterial(nullptr), fLsMaterial(nullptr), fCo60Material(nullptr),
   fGlassMaterial(nullptr), fPmtBodyMaterial(nullptr), fSiliconeMaterial(nullptr),
   fVacuumMaterial(nullptr), fEpoxyMaterial(nullptr),
   // 제어 변수는 헤더에 정의된 상수로 초기화
   fMovablePMTAngle(kDefaultAngle),
   fDetectorDistance(kDefaultDistance),
   fDetectorMessenger(nullptr), logicLS(nullptr), logicPhotocathode(nullptr)
{
  // 안전한 실행 흐름 제어
  DefineMaterials();          // 1. 기본 물질 정의
  ValidateMaterials();        // 2. 물질 유효성 검사
  DefineOpticalProperties();  // 3. 광학 속성 정의
  
  fDetectorMessenger = new DetectorMessenger(this); // 4. 메신저 생성
}

DetectorConstruction::~DetectorConstruction()
{
  delete fDetectorMessenger;
}

// === 1. 물질 정의 (DefineMaterials) ===
void DetectorConstruction::DefineMaterials()
{
  auto nist = G4NistManager::Instance();

  // 1-0. 필수 원소 로드 (LS 및 Epoxy 정의에 필요)
  G4Element* H = nist->FindOrBuildElement("H");
  G4Element* C = nist->FindOrBuildElement("C");
  G4Element* N = nist->FindOrBuildElement("N");
  G4Element* O = nist->FindOrBuildElement("O");

  // 원소 로드 실패 시 즉시 종료
  if (!H || !C || !N || !O) {
      G4Exception("DetectorConstruction::DefineMaterials()",
                  "FatalError_ElementNotFound", FatalException,
                  "필수 기본 원소(H, C, N, O) 로드에 실패했습니다. 실행 환경 변수(geant4.sh) 설정을 확인하십시오.");
  }

  // 1-1. NIST 표준 물질 로드
  fAirMaterial = nist->FindOrBuildMaterial("G4_AIR");
  fVacuumMaterial = nist->FindOrBuildMaterial("G4_Galactic");
  fCo60Material = nist->FindOrBuildMaterial("G4_Co");
  fGlassMaterial = nist->FindOrBuildMaterial("G4_Pyrex_Glass");
  fPmtBodyMaterial = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
  fSiliconeMaterial = nist->FindOrBuildMaterial("G4_SILICON_DIOXIDE");

  // 1-2. Epoxy 수동 정의 (NIST 의존성 제거, 재현성 보장)
  // 표준 Epoxy 정의 (Density: 1.25 g/cm3)
  fEpoxyMaterial = new G4Material("Epoxy", 1.25*g/cm3, 3);
  // 일반적인 에폭시 수지의 질량비 사용 (H: 8.6%, C: 69.0%, O: 22.4%)
  fEpoxyMaterial->AddElement(H, 8.6*perCent);
  fEpoxyMaterial->AddElement(C, 69.0*perCent);
  fEpoxyMaterial->AddElement(O, 22.4*perCent);

  // 1-3. 액체 섬광체(LS) 조성 정의 (LAB + PPO + bisMSB)
  G4Material* lab = new G4Material("LAB", 0.86*g/cm3, 2);
  lab->AddElement(C, 18); lab->AddElement(H, 30);
  
  G4Material* ppo = new G4Material("PPO", 1.1*g/cm3, 4);
  ppo->AddElement(C, 15); ppo->AddElement(H, 11); ppo->AddElement(N, 1); ppo->AddElement(O, 1);
  
  G4Material* bismsb = new G4Material("bisMSB", 1.05*g/cm3, 2);
  bismsb->AddElement(C, 24); bismsb->AddElement(H, 22);
  
  fLsMaterial = new G4Material("LS", 0.863*g/cm3, 3);
  fLsMaterial->AddMaterial(lab, 99.64*perCent);
  fLsMaterial->AddMaterial(ppo, 0.35*perCent);
  fLsMaterial->AddMaterial(bismsb, 0.01*perCent);
}

// === 2. 물질 검증 (ValidateMaterials) ===
void DetectorConstruction::ValidateMaterials()
{
  // 검사할 필수 물질 목록 정의 (포인터와 이름 쌍)
  // [!수정 완료!] Epoxy 이름을 Custom으로 변경하여 수동 정의되었음을 명시
  std::vector<std::pair<G4Material*, std::string>> materials = {
      {fAirMaterial, "G4_AIR"}, {fLsMaterial, "LS (Custom)"}, 
      {fCo60Material, "G4_Co"}, {fGlassMaterial, "G4_Pyrex_Glass"},
      {fPmtBodyMaterial, "G4_STAINLESS-STEEL"}, {fSiliconeMaterial, "G4_SILICON_DIOXIDE"},
      {fVacuumMaterial, "G4_Galactic"}, {fEpoxyMaterial, "Epoxy (Custom)"}
  };

  bool allValid = true;
  std::stringstream errMsg;
  errMsg << "\n\n========================= [치명적 오류 발생] =========================\n";
  errMsg << "[오류] 하나 이상의 필수 물질(Material) 로드 또는 생성에 실패했습니다 (NULL 포인터).\n";
  
  // 누락된 물질 식별
  for (const auto& pair : materials) {
    if (!pair.first) {
      allValid = false;
      errMsg << "  - 실패: " << pair.second << "을(를) 찾거나 생성할 수 없습니다.\n";
    }
  }

  if (!allValid) {
    errMsg << "\n[원인] NIST 데이터베이스 접근 실패 또는 메모리 할당 실패일 수 있습니다.\n";
    errMsg << "[해결] 실행 전 'source geant4.sh' 환경 변수 설정 및 시스템 메모리를 확인하십시오.\n";
    errMsg << "========================================================================\n\n";
    
    // 치명적 예외 발생 및 프로그램 종료
    G4Exception("DetectorConstruction::ValidateMaterials()", 
                "FatalError_MaterialNotFound", FatalException, errMsg.str().c_str());
  }
}

// === 3. 광학 속성 정의 (DefineOpticalProperties) ===
// (이하 함수들은 이전 리팩토링 버전과 동일합니다.)

void DetectorConstruction::DefineOpticalProperties()
{
  //  C 스타일 배열 대신 std::vector 사용
  const std::vector<G4double> photonEnergies = {
      2.38*eV, 2.48*eV, 2.58*eV, 2.70*eV, 2.76*eV, 2.82*eV, 
      2.92*eV, 2.95*eV, 3.02*eV, 3.10*eV, 3.26*eV, 3.44*eV
  };
  const G4int nEntries = photonEnergies.size();

  // --- 굴절률 (RINDEX) 정의 ---
  std::vector<G4double> rindex_air(nEntries, 1.0);
  std::vector<G4double> rindex_glass(nEntries, 1.47); // Pyrex
  std::vector<G4double> rindex_ls(nEntries, 1.50);    // LAB 기반
  std::vector<G4double> rindex_silicone(nEntries, 1.45);

  // --- MPT 설정 ---
  // (ValidateMaterials()가 먼저 호출되었으므로 물질 포인터는 유효함이 보장됨)

  // Air 및 Vacuum MPT 설정
  auto airMPT = new G4MaterialPropertiesTable();
  // [!현대화!] Geant4 최신 버전은 std::vector를 직접 인자로 받을 수 있습니다.
  airMPT->AddProperty("RINDEX", photonEnergies, rindex_air);
  fAirMaterial->SetMaterialPropertiesTable(airMPT);
  // 진공(Vacuum)도 굴절률 1.0으로 설정 (Air와 동일 MPT 사용 가능)
  fVacuumMaterial->SetMaterialPropertiesTable(airMPT);

  // Glass MPT 설정
  auto glassMPT = new G4MaterialPropertiesTable();
  glassMPT->AddProperty("RINDEX", photonEnergies, rindex_glass);
  fGlassMaterial->SetMaterialPropertiesTable(glassMPT);

  // Silicone MPT 설정
  auto siliconeMPT = new G4MaterialPropertiesTable();
  siliconeMPT->AddProperty("RINDEX", photonEnergies, rindex_silicone);
  fSiliconeMaterial->SetMaterialPropertiesTable(siliconeMPT);

  // LS MPT 설정 (섬광 속성)
  const std::vector<G4double> absorption_ls = {
      20*m, 20*m, 20*m, 20*m, 20*m, 18*m, 15*m, 15*m, 10*m, 5*m, 2*m, 1*m
  };
  const std::vector<G4double> emission_ls = {
      0.05, 0.15, 0.45, 0.85, 1.00, 0.85, 0.45, 0.35, 0.20, 0.10, 0.05, 0.02
  };

  auto lsMPT = new G4MaterialPropertiesTable();
  lsMPT->AddProperty("RINDEX", photonEnergies, rindex_ls);
  lsMPT->AddProperty("ABSLENGTH", photonEnergies, absorption_ls);
  lsMPT->AddProperty("SCINTILLATIONCOMPONENT1", photonEnergies, emission_ls);
  
  // 섬광 상수 설정
  lsMPT->AddConstProperty("SCINTILLATIONYIELD", 10000./MeV);
  lsMPT->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 10.*ns);
  lsMPT->AddConstProperty("RESOLUTIONSCALE", 1.0);
  
  // 버크스 상수 (Birks Constant) 설정
  const std::vector<G4double> birks_energies = { 1.0*eV, 100.0*MeV };
  const std::vector<G4double> birks_constant = { 0.07943*mm/MeV, 0.07943*mm/MeV };
  // 마지막 인자 true는 스플라인 보간 사용을 의미함.
  lsMPT->AddProperty("BIRKSCONSTANT", birks_energies, birks_constant, true);
  
  fLsMaterial->SetMaterialPropertiesTable(lsMPT);
}

// ============================================================================
// === [!수정 완료!] 지오메트리 구성 (Construct) ===
// ============================================================================
G4VPhysicalVolume* DetectorConstruction::Construct()
{
  // --- World Volume 및 선원 설정 (변경 없음) ---
  auto solidWorld = new G4Box("SolidWorld", kWorldHalfSize, kWorldHalfSize, kWorldHalfSize);
  auto logicWorld = new G4LogicalVolume(solidWorld, fVacuumMaterial, "LogicWorld");
  auto physWorld = new G4PVPlacement(nullptr, G4ThreeVector(), logicWorld, "PhysWorld", nullptr, false, 0, true);
  logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());

  // ... (선원 및 에폭시 설정 코드, 이전과 동일하게 유지) ...
  auto solidEpoxy = new G4Tubs("SolidEpoxy", 0., kEpoxyRadius, kEpoxyHalfZ, 0., twopi);
  auto logicEpoxy = new G4LogicalVolume(solidEpoxy, fEpoxyMaterial, "LogicEpoxy");
  logicEpoxy->SetVisAttributes(new G4VisAttributes(G4Colour(0.8, 0.8, 0.8, 0.3)));

  auto solidSource = new G4Tubs("SolidSource", 0., kSourceRadius, kSourceHalfZ, 0., twopi);
  auto logicSource = new G4LogicalVolume(solidSource, fCo60Material, "LogicSource");
  logicSource->SetVisAttributes(new G4VisAttributes(G4Colour::Blue()));

  new G4PVPlacement(nullptr, G4ThreeVector(), logicSource, "PhysSource", logicEpoxy, false, 0, true);

  // 선원 캡슐 배치 (X축 방향으로 눕힘, 이전과 동일)
  auto source_rot = new G4RotationMatrix();
  source_rot->rotateY(90. * deg); // CW 시스템이므로 +90 호출 시 -90(CCW) 적용되어 X축 방향이 됨.
  new G4PVPlacement(source_rot, G4ThreeVector(), logicEpoxy, "PhysEpoxy", logicWorld, false, 0, true);

  // ========================================================================
  // === [!버그 수정 및 논리 제어 완성!] 검출기 유닛 배치 ===
  // ========================================================================
  G4LogicalVolume* logicDetectorUnit = ConstructDetectorUnit();

  G4double R_ls_center = fDetectorDistance;     // 거리 R (목표: LS 중심까지의 거리)
  G4double theta = fMovablePMTAngle;            // 사잇각 (θ)
  G4double offset = kAssemblyCenterOffset;      // 오프셋 (LS 중심과 조립체 중심의 차이, 108mm)

  // [!개선!] 배치 기준 거리 계산: R_placement = R_ls_center + offset
  G4double R_placement = R_ls_center + offset;

  // [!중요!] 회전 방향 수정: 시각화 결과, Geant4/CLHEP는 시계 방향(CW)을 양수로 처리합니다.
  // 표준 수학 정의(CCW 양수)의 각도 'alpha'를 얻으려면, rotateY(-alpha)를 호출해야 합니다.

  // --- 1. 고정형 검출기 (Fixed Detector) 배치 (θ=0, CopyNumber 0) ---

  // 위치: +X축 상 (R_placement, 0, 0)
  G4ThreeVector pos_fixed(R_placement, 0, 0);

  // 방향: 선원(-X 방향)을 향함. 원하는 표준 회전 각도는 -90도입니다.
  // 구현: rotateY(+90도)를 호출합니다.
  auto rot_fixed = new G4RotationMatrix();
  rot_fixed->rotateY(90. * deg); // [!수정!] -90도에서 +90도로 변경

  new G4PVPlacement(rot_fixed, pos_fixed, logicDetectorUnit, "PhysDetectorUnit_Fixed", logicWorld, false, 0, true);

  // --- 2. 이동형 검출기 (Movable Detector) 배치 (CopyNumber 1) ---

  // 위치 계산: P = (R_placement*cos(θ), 0, R_placement*sin(θ))
  G4ThreeVector pos_movable(R_placement * std::cos(theta), 0., R_placement * std::sin(theta));

  // 방향 계산 (Tidal Locking): 원하는 표준 회전 각도는 alpha = -(90도 + θ) 입니다.
  // 구현: rotateY(-alpha) = rotateY(90도 + θ)를 호출합니다.
  G4double orientation_angle = 90.*deg + theta; // [!수정!] 부호 변경
  auto rot_movable = new G4RotationMatrix();
  rot_movable->rotateY(orientation_angle);

  new G4PVPlacement(rot_movable, pos_movable, logicDetectorUnit, "PhysDetectorUnit_Movable", logicWorld, false, 1, true);
  // ========================================================================

  return physWorld;
}

// ============================================================================
// === [!수정 완료!] 검출기 유닛 구성 (ConstructDetectorUnit) ===
// ============================================================================
G4LogicalVolume* DetectorConstruction::ConstructDetectorUnit()
{
  // 검출기 조립체 (Envelope)
  // [!수정!] kAssemblyHalfZ는 최적화된 절반 길이(148mm)를 사용합니다.
  auto solidAssembly = new G4Tubs("SolidAssembly", 0, kAssemblyRadius, kAssemblyHalfZ, 0, twopi);
  auto logicAssembly = new G4LogicalVolume(solidAssembly, fAirMaterial, "LogicAssembly");
  logicAssembly->SetVisAttributes(G4VisAttributes::GetInvisible());

  // [!핵심 수정!] 좌표계 설정
  // 조립체의 기하학적 중심(0,0,0)을 기준으로, LS의 중심이 +Z 방향으로 오프셋만큼 이동하도록 배치합니다.
  // 이는 검출기의 헤드(LS) 방향이 로컬 +Z축이 되도록 보장합니다.
  G4double offset = kAssemblyCenterOffset; // +108 mm

  // --- 유리병 (Glass Bottle) ---
  auto solidBottleBody = new G4Tubs("SolidBottleBody", kBottleInnerRadius, kBottleOuterRadius, kLSHalfZ, 0, twopi);
  auto logicBottleBody = new G4LogicalVolume(solidBottleBody, fGlassMaterial, "LogicBottleBody");
  logicBottleBody->SetVisAttributes(new G4VisAttributes(G4Colour(0.0, 1.0, 1.0, 0.2))); // Cyan
  // 오프셋 적용
  new G4PVPlacement(nullptr, G4ThreeVector(0, 0, offset), logicBottleBody, "PhysBottleBody", logicAssembly, false, 0, true);

  // --- 액체 섬광체 (LS) ---
  auto solidLS = new G4Tubs("SolidLS", 0, kBottleInnerRadius, kLSHalfZ, 0, twopi);
  logicLS = new G4LogicalVolume(solidLS, fLsMaterial, "LogicLS");
  logicLS->SetVisAttributes(new G4VisAttributes(G4Colour(1.0, 1.0, 0.0, 0.3))); // Yellow
  // 오프셋 적용
  new G4PVPlacement(nullptr, G4ThreeVector(0, 0, offset), logicLS, "PhysLS", logicAssembly, false, 0, true);

  // --- 광학 그리스 (Optical Grease) ---
  auto solidGrease = new G4Tubs("SolidGrease", 0, kBottleOuterRadius, kGreaseHalfZ, 0, twopi);
  auto logicGrease = new G4LogicalVolume(solidGrease, fSiliconeMaterial, "LogicGrease");
  logicGrease->SetVisAttributes(new G4VisAttributes(G4Colour(0.8, 0.8, 0.8, 0.2))); // Grey

  // LS 하단(-Z 방향)에 배치 (Z 위치 계산, 오프셋 기준)
  G4double grease_Z = offset - kLSHalfZ - kGreaseHalfZ;
  new G4PVPlacement(nullptr, G4ThreeVector(0, 0, grease_Z), logicGrease, "PhysGrease", logicAssembly, false, 0, true);

  // --- PMT 배치 ---
  auto logicPMT = ConstructPMT();
  // 그리스 하단에 PMT 배치 (Z 위치 계산)
  G4double pmt_Z = grease_Z - kGreaseHalfZ - kPmtAssemblyHalfZ;
  new G4PVPlacement(nullptr, G4ThreeVector(0, 0, pmt_Z), logicPMT, "PhysPMT", logicAssembly, false, 0, true);

  return logicAssembly;
}

// === PMT 구성 (ConstructPMT) ===
G4LogicalVolume* DetectorConstruction::ConstructPMT()
{
  // PMT 조립체 (내부는 진공)
  auto solidPmtAssembly = new G4Tubs("SolidPmtAssembly", 0, kPmtAssemblyRadius, kPmtAssemblyHalfZ, 0, twopi);
  auto logicPmtAssembly = new G4LogicalVolume(solidPmtAssembly, fVacuumMaterial, "LogicPmtAssembly");
  logicPmtAssembly->SetVisAttributes(G4VisAttributes::GetInvisible());

  // --- PMT 윈도우 (Glass) ---
  auto solidPmtWindow = new G4Tubs("SolidPmtWindow", 0, kPmtWindowRadius, kPmtWindowHalfZ, 0, twopi);
  auto logicPmtWindow = new G4LogicalVolume(solidPmtWindow, fGlassMaterial, "LogicPmtWindow");
  logicPmtWindow->SetVisAttributes(new G4VisAttributes(G4Colour(0.0, 1.0, 1.0, 0.1)));
  
  // 조립체 상단(+Z)에 배치
  G4double window_Z_center = kPmtAssemblyHalfZ - kPmtWindowHalfZ;
  new G4PVPlacement(nullptr, G4ThreeVector(0, 0, window_Z_center), logicPmtWindow, "PhysPmtWindow", logicPmtAssembly, false, 0, true);

  // --- PMT 몸체 (Stainless Steel) ---
  auto solidPmtBody = new G4Tubs("SolidPmtBody", kPmtInnerRadius, kPmtBodyRadius, kPmtBodyHalfZ, 0, twopi);
  auto logicPmtBody = new G4LogicalVolume(solidPmtBody, fPmtBodyMaterial, "LogicPmtBody");
  logicPmtBody->SetVisAttributes(new G4VisAttributes(G4Colour(0.7, 0.7, 0.7)));
  
  // 윈도우 하단에 배치
  G4double body_Z_center = window_Z_center - kPmtWindowHalfZ - kPmtBodyHalfZ;
  new G4PVPlacement(nullptr, G4ThreeVector(0, 0, body_Z_center), logicPmtBody, "PhysPmtBody", logicPmtAssembly, false, 0, true);
  
  // --- 광음극 (Photocathode) ---
  // 반경은 윈도우와 동일하게 설정
  auto solidPhotocathode = new G4Tubs("SolidPhotocathode", 0, kPmtWindowRadius, kPhotocathodeHalfZ, 0, twopi);
  // 광음극 물질은 Optical Surface로 처리되므로 중요하지 않음. 기존 코드 유지.
  logicPhotocathode = new G4LogicalVolume(solidPhotocathode, fPmtBodyMaterial, "LogicPhotocathode");
  logicPhotocathode->SetVisAttributes(new G4VisAttributes(G4Colour::Red()));
  
  // 윈도우 바로 아래 (진공 영역)에 배치
  G4double cathode_Z_center = window_Z_center - kPmtWindowHalfZ - kPhotocathodeHalfZ;
  new G4PVPlacement(nullptr, G4ThreeVector(0, 0, cathode_Z_center), logicPhotocathode, "PhysPhotocathode", logicPmtAssembly, false, 0, true);

  // --- 광음극 광학 표면 (Optical Surface) 설정 ---
  auto photocathode_opsurf = new G4OpticalSurface("Photocathode_OpSurface");
  photocathode_opsurf->SetType(dielectric_metal); // 금속 표면 모델
  photocathode_opsurf->SetModel(unified);
  photocathode_opsurf->SetFinish(polished);

  // 양자 효율 (QE) 설정 (std::vector 사용)
  const std::vector<G4double> energies_qe = {1.9*eV, 2.1*eV, 2.4*eV, 2.7*eV, 2.9*eV, 3.1*eV, 3.3*eV, 3.5*eV, 3.7*eV, 4.1*eV};
  const std::vector<G4double> efficiency = {0.02, 0.08, 0.18, 0.24, 0.28, 0.26, 0.22, 0.15, 0.05, 0.01};
  
  auto pmtMPT = new G4MaterialPropertiesTable();
  // EFFICIENCY는 광자가 검출(Detected)될 확률을 정의함.
  pmtMPT->AddProperty("EFFICIENCY", energies_qe, efficiency);
  photocathode_opsurf->SetMaterialPropertiesTable(pmtMPT);

  // 광음극 논리 볼륨에 표면 속성 부여 (Skin Surface)
  new G4LogicalSkinSurface("Photocathode_SkinSurface", logicPhotocathode, photocathode_opsurf);

  return logicPmtAssembly;
}

// === Sensitive Detector 및 Field 설정 ===
void DetectorConstruction::ConstructSDandField()
{
  if (logicLS) {
    auto lsSD = new LSSD("LSSD");
    G4SDManager::GetSDMpointer()->AddNewDetector(lsSD);
    SetSensitiveDetector(logicLS, lsSD);
  }
  if (logicPhotocathode) {
    auto pmtSD = new PMTSD("PMTSD");
    G4SDManager::GetSDMpointer()->AddNewDetector(pmtSD);
    SetSensitiveDetector(logicPhotocathode, pmtSD);
  }
}

// === Setter 함수들 ===
void DetectorConstruction::SetMovablePMTAngle(G4double angle)
{
  fMovablePMTAngle = angle;
  G4RunManager::GetRunManager()->GeometryHasBeenModified();
}

void DetectorConstruction::SetDetectorDistance(G4double distance)
{
  fDetectorDistance = distance;
  G4RunManager::GetRunManager()->GeometryHasBeenModified();
}
