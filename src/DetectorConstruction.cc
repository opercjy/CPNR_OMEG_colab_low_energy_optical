#include "DetectorConstruction.hh"

// --- Geant4 헤더 파일 ---
#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Sphere.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

// --- 사용자 정의 클래스 헤더 ---
#include "PMTSD.hh"
#include "LSSD.hh"

// --- Messenger (UI Command) 헤더 ---
#include "G4UImessenger.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"

/**
 * @class DetectorMessenger
 * @brief DetectorConstruction의 멤버 변수(회전 각도 등)를
 * 매크로 UI 명령어로 제어할 수 있게 해주는 헬퍼 클래스입니다.
 */
class DetectorMessenger : public G4UImessenger
{
public:
  DetectorMessenger(DetectorConstruction* det) : fDetector(det)
  {
    fDirectory = new G4UIdirectory("/myApp/detector/");
    fDirectory->SetGuidance("Detector geometry control commands.");
    
    fSetAngleCmd = new G4UIcmdWithADoubleAndUnit("/myApp/detector/setMovableAngle", this);
    fSetAngleCmd->SetGuidance("Set the rotation angle of the movable PMT.");
    fSetAngleCmd->SetParameterName("Angle", false);
    fSetAngleCmd->SetUnitCategory("Angle");
    fSetAngleCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
  }
  
  virtual ~DetectorMessenger()
  {
    delete fSetAngleCmd;
    delete fDirectory;
  }
  
  // 매크로에서 명령어가 입력되었을 때 DetectorConstruction의 public 함수를 호출합니다.
  virtual void SetNewValue(G4UIcommand* command, G4String newValue) override
  {
    if (command == fSetAngleCmd) {
      fDetector->SetMovablePMTAngle(fSetAngleCmd->GetNewDoubleValue(newValue));
    }
  }

private:
  DetectorConstruction* fDetector;
  G4UIdirectory* fDirectory;
  G4UIcmdWithADoubleAndUnit* fSetAngleCmd;
};


// === DetectorConstruction 클래스 구현 ===

DetectorConstruction::DetectorConstruction()
 : G4VUserDetectorConstruction(), 
   fMovablePMTAngle(90.0*deg), // 회전 PMT 각도 기본값
   logicLS(nullptr), logicPhotocathode(nullptr)
{
  fDetectorMessenger = new DetectorMessenger(this);
}

DetectorConstruction::~DetectorConstruction()
{
  delete fDetectorMessenger;
}

void DetectorConstruction::SetMovablePMTAngle(G4double angle)
{
  fMovablePMTAngle = angle;
  // Geant4 커널에 지오메트리가 변경되었음을 알려 재구성하도록 요청합니다.
  G4RunManager::GetRunManager()->GeometryHasBeenModified();
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  DefineMaterials();

  // --- World Volume ---
  auto solidWorld = new G4Box("SolidWorld", 0.5*m, 0.5*m, 0.5*m);
  auto logicWorld = new G4LogicalVolume(solidWorld, fAirMaterial, "LogicWorld");
  auto physWorld = new G4PVPlacement(0, G4ThreeVector(), "PhysWorld", logicWorld, nullptr, false, 0);
  logicWorld->SetVisAttributes(new G4VisAttributes(false));

  // --- 선원 (Source) at (0,0,0) ---
  auto solidSource = new G4Sphere("SolidSource", 0, 1.0*mm, 0, 360*deg, 0, 180*deg);
  auto logicSource = new G4LogicalVolume(solidSource, fCo60Material, "LogicSource");
  new G4PVPlacement(0, G4ThreeVector(), "PhysSource", logicSource, physWorld, false, 0);
  logicSource->SetVisAttributes(new G4VisAttributes(G4Colour::Blue()));

  // --- 단일 검출기 유닛 생성 ---
  G4LogicalVolume* logicDetectorUnit = ConstructDetectorUnit();
  G4double R = 20.0 * cm; // 선원과 검출기 유닛 중심 사이의 거리

  // --- 검출기 유닛 배치 ---
  // 1. 고정된 검출기 유닛 (Fixed Detector Unit)
  G4RotationMatrix rot_fixed; // 회전 없음
  G4ThreeVector pos_fixed(R, 0, 0);
  new G4PVPlacement(G4Transform3D(rot_fixed, pos_fixed),
                    logicDetectorUnit, "PhysDetectorUnit_Fixed", logicWorld, false, 0, true); // Copy Number 0

  // 2. 회전하는 검출기 유닛 (Movable Detector Unit)
  G4RotationMatrix rot_movable;
  rot_movable.rotateZ(fMovablePMTAngle); // Z축을 중심으로 각도만큼 회전
  G4ThreeVector pos_movable(R*std::cos(fMovablePMTAngle),
                            R*std::sin(fMovablePMTAngle), 0);
  new G4PVPlacement(G4Transform3D(rot_movable, pos_movable),
                    logicDetectorUnit, "PhysDetectorUnit_Movable", logicWorld, false, 1, true); // Copy Number 1

  return physWorld;
}

/**
 * @brief 듀란 병, LS, 실리콘, PMT를 포함하는 단일 검출기 유닛을 생성하는 헬퍼 함수
 * @return G4LogicalVolume* 생성된 검출기 유닛의 어머니 볼륨 포인터
 */
G4LogicalVolume* DetectorConstruction::ConstructDetectorUnit()
{
  // --- 검출기 유닛 어머니 볼륨 ---
  G4double assemblyRadius = 60*mm;
  G4double assemblyHalfZ = 150*mm;
  auto solidAssembly = new G4Tubs("SolidAssembly", 0, assemblyRadius, assemblyHalfZ, 0, twopi);
  auto logicAssembly = new G4LogicalVolume(solidAssembly, fAirMaterial, "LogicAssembly");
  logicAssembly->SetVisAttributes(new G4VisAttributes(false));

  // --- 듀란 병, LS, 뚜껑 등 배치 ---
  G4double bottle_outer_radius = 28.0 * mm;
  G4double bottle_inner_radius = 26.0 * mm;
  G4double bottle_body_hz = 40.0 * mm;
  auto solidBottleBody = new G4Tubs("SolidBottleBody", bottle_inner_radius, bottle_outer_radius, bottle_body_hz, 0, twopi);
  auto logicBottleBody = new G4LogicalVolume(solidBottleBody, fGlassMaterial, "LogicBottleBody");
  new G4PVPlacement(0, G4ThreeVector(), "PhysBottleBody", logicBottleBody, logicAssembly, false, 0);
  
  auto solidLS = new G4Tubs("SolidLS", 0, bottle_inner_radius, bottle_body_hz, 0, twopi);
  logicLS = new G4LogicalVolume(solidLS, fLsMaterial, "LogicLS");
  new G4PVPlacement(0, G4ThreeVector(), "PhysLS", logicLS, logicAssembly, false, 0);

  // --- 실리콘 및 PMT 배치 ---
  G4double grease_hz = 0.5 * mm;
  auto solidGrease = new G4Tubs("SolidGrease", 0, bottle_outer_radius, grease_hz, 0, twopi);
  auto logicGrease = new G4LogicalVolume(solidGrease, fSiliconeMaterial, "LogicGrease");
  new G4PVPlacement(0, G4ThreeVector(0, 0, -bottle_body_hz - grease_hz), "PhysGrease", logicGrease, logicAssembly, false, 0);

  auto logicPMT = ConstructPMT();
  G4double pmt_full_length = 215. * mm;
  G4ThreeVector pmt_pos(0, 0, -bottle_body_hz - 2*grease_hz - 0.5*pmt_full_length);
  new G4PVPlacement(0, pmt_pos, "PhysPMT", logicPMT, logicAssembly, false, 0);

  return logicAssembly;
}

G4LogicalVolume* DetectorConstruction::ConstructPMT()
{
  auto solidPmtAssembly = new G4Tubs("SolidPmtAssembly", 0, 30.*mm, 107.5*mm, 0, twopi);
  auto logicPmtAssembly = new G4LogicalVolume(solidPmtAssembly, fVacuumMaterial, "LogicPmtAssembly");
  logicPmtAssembly->SetVisAttributes(new G4VisAttributes(false));

  auto solidPmtBody = new G4Tubs("SolidPmtBody", 28*mm, 29.5*mm, 105*mm, 0, twopi);
  auto logicPmtBody = new G4LogicalVolume(solidPmtBody, fPmtBodyMaterial, "LogicPmtBody");
  new G4PVPlacement(0, G4ThreeVector(), "PhysPmtBody", logicPmtBody, logicPmtAssembly, false, 0);

  auto solidPmtWindow = new G4Tubs("SolidPmtWindow", 0, 26.5*mm, 2.5*mm, 0, twopi);
  auto logicPmtWindow = new G4LogicalVolume(solidPmtWindow, fGlassMaterial, "LogicPmtWindow");
  new G4PVPlacement(0, G4ThreeVector(0, 0, 105*mm), "PhysPmtWindow", logicPmtWindow, logicPmtAssembly, false, 0);

  auto solidPhotocathode = new G4Tubs("SolidPhotocathode", 0, 26.5*mm, 0.05*mm, 0, twopi);
  logicPhotocathode = new G4LogicalVolume(solidPhotocathode, fPmtBodyMaterial, "LogicPhotocathode");
  new G4PVPlacement(0, G4ThreeVector(0, 0, 102.45*mm), "PhysPhotocathode", logicPhotocathode, logicPmtAssembly, false, 0);

  auto photocathode_opsurf = new G4OpticalSurface("Photocathode_OpSurface");
  photocathode_opsurf->SetType(dielectric_metal);
  photocathode_opsurf->SetModel(unified);
  photocathode_opsurf->SetFinish(polished);

  const G4int NUMENTRIES = 10;
  G4double energies[] = {1.9*eV, 2.1*eV, 2.4*eV, 2.7*eV, 2.9*eV, 3.1*eV, 3.3*eV, 3.5*eV, 3.7*eV, 4.1*eV};
  G4double efficiency[] = {0.02, 0.08, 0.18, 0.24, 0.28, 0.26, 0.22, 0.15, 0.05, 0.01};
  auto pmtMPT = new G4MaterialPropertiesTable();
  pmtMPT->AddProperty("EFFICIENCY", energies, efficiency, NUMENTRIES);
  photocathode_opsurf->SetMaterialPropertiesTable(pmtMPT);

  new G4LogicalSkinSurface("Photocathode_SkinSurface", logicPhotocathode, photocathode_opsurf);

  return logicPmtAssembly;
}

void DetectorConstruction::DefineMaterials()
{
  auto nist = G4NistManager::Instance();

  fAirMaterial = nist->FindOrBuildMaterial("G4_AIR");
  fVacuumMaterial = nist->FindOrBuildMaterial("G4_Galactic");
  fCo60Material = nist->FindOrBuildMaterial("G4_Co");
  fGlassMaterial = nist->FindOrBuildMaterial("G4_Pyrex_Glass");
  fPmtBodyMaterial = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
  fSiliconeMaterial = nist->FindOrBuildMaterial("G4_SILICON_DIOXIDE");
  
  G4Element* H = nist->FindOrBuildElement("H");
  G4Element* C = nist->FindOrBuildElement("C");
  G4Element* N = nist->FindOrBuildElement("N");
  G4Element* O = nist->FindOrBuildElement("O");
  
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

  // --- 광학 속성 정의 ---
  const G4int NUMENTRIES = 12;
  G4double photonEnergies[NUMENTRIES] = {2.38*eV, 2.48*eV, 2.58*eV, 2.70*eV, 2.76*eV, 2.82*eV, 2.92*eV, 2.95*eV, 3.02*eV, 3.10*eV, 3.26*eV, 3.44*eV};
  G4double rindex_air[NUMENTRIES] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
  G4double rindex_glass[NUMENTRIES] = {1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47, 1.47};
  G4double rindex_ls[NUMENTRIES] = {1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5, 1.5};
  G4double rindex_silicone[NUMENTRIES] = {1.45, 1.45, 1.45, 1.45, 1.45, 1.45, 1.45, 1.45, 1.45, 1.45, 1.45, 1.45};
  G4double absorption_ls[NUMENTRIES] = {20*m, 20*m, 20*m, 20*m, 20*m, 18*m, 15*m, 15*m, 10*m, 5*m, 2*m, 1*m};
  G4double emission_ls[NUMENTRIES] = {0.05, 0.15, 0.45, 0.85, 1.00, 0.85, 0.45, 0.35, 0.20, 0.10, 0.05, 0.02};

  auto airMPT = new G4MaterialPropertiesTable();
  airMPT->AddProperty("RINDEX", photonEnergies, rindex_air, NUMENTRIES);
  fAirMaterial->SetMaterialPropertiesTable(airMPT);

  auto glassMPT = new G4MaterialPropertiesTable();
  glassMPT->AddProperty("RINDEX", photonEnergies, rindex_glass, NUMENTRIES);
  fGlassMaterial->SetMaterialPropertiesTable(glassMPT);
  
  auto siliconeMPT = new G4MaterialPropertiesTable();
  siliconeMPT->AddProperty("RINDEX", photonEnergies, rindex_silicone, NUMENTRIES);
  fSiliconeMaterial->SetMaterialPropertiesTable(siliconeMPT);

  auto lsMPT = new G4MaterialPropertiesTable();
  lsMPT->AddProperty("RINDEX", photonEnergies, rindex_ls, NUMENTRIES);
  lsMPT->AddProperty("ABSLENGTH", photonEnergies, absorption_ls, NUMENTRIES);
  lsMPT->AddProperty("SCINTILLATIONCOMPONENT1", photonEnergies, emission_ls, NUMENTRIES)->SetSpline(true);
  lsMPT->AddConstProperty("SCINTILLATIONYIELD", 10000./MeV);
  lsMPT->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 10.*ns);
  lsMPT->AddConstProperty("RESOLUTIONSCALE", 1.0);
  lsMPT->AddConstProperty("BIRKSCONSTANT", 0.126*mm/MeV);
  fLsMaterial->SetMaterialPropertiesTable(lsMPT);
}

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
