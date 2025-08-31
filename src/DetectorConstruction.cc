// DetectorConstruction.cc (리팩토링 완료)

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

// --- [!리팩토링 핵심!] Messenger 관련 헤더 변경 ---
// 기존 G4UImessenger 관련 헤더 대신, G4GenericMessenger 헤더 하나만 포함하면 된다.
#include "G4GenericMessenger.hh"

// std::vector 및 오류 메시지 구성을 위한 헤더
#include <vector>
#include <sstream>
#include <string>
#include <utility>
#include <cmath>

// ============================================================================
// === [!삭제!] DetectorMessenger 클래스 구현부 ===
// ============================================================================
// G4GenericMessenger를 사용하므로, 이 파일 상단에 있던 DetectorMessenger 클래스의
// 전체 구현(생성자, 소멸자, SetNewValue 함수 등)을 삭제한다.
// 코드가 훨씬 간결해지고 유지보수가 용이해진다.


// ============================================================================
// === DetectorConstruction 클래스 구현 ===
// ============================================================================

/**
 * @brief 생성자
 */
DetectorConstruction::DetectorConstruction()
 : G4VUserDetectorConstruction(),
   // 모든 포인터는 nullptr로, 변수는 기본값으로 명시적 초기화하는 것이 좋은 관행이다.
   fAirMaterial(nullptr), fLsMaterial(nullptr), fCo60Material(nullptr),
   fGlassMaterial(nullptr), fPmtBodyMaterial(nullptr), fSiliconeMaterial(nullptr),
   fVacuumMaterial(nullptr), fEpoxyMaterial(nullptr),
   fMovablePMTAngle(kDefaultAngle),
   fDetectorDistance(kDefaultDistance),
   fMessenger(nullptr), // [수정] fMessenger 포인터 초기화
   logicLS(nullptr), logicPhotocathode(nullptr)
{
    // 객체 생성 시 수행할 작업을 명확한 순서로 호출한다.
    DefineMaterials();
    ValidateMaterials();
    DefineOpticalProperties();
    
    // [수정] 기존 메신저 생성 코드 대신, 새로 만든 DefineCommands() 함수를 호출한다.
    DefineCommands();
}

/**
 * @brief 소멸자
 */
DetectorConstruction::~DetectorConstruction()
{
    // 생성자에서 할당한 메신저 객체의 메모리를 해제한다.
    delete fMessenger;
}

/**
 * @brief [!신규!] UI 커맨드를 정의하고 메신저를 설정하는 함수
 * @details G4GenericMessenger를 사용하여 이 클래스의 멤버 함수를 매크로 명령어와 연결한다.
 *          이 방식은 코드를 간결하게 만들고 별도의 메신저 클래스가 필요 없게 한다.
 */
void DetectorConstruction::DefineCommands()
{
    // 1. 메신저 객체를 생성한다.
    //    - this: 이 메신저가 제어할 대상이 DetectorConstruction 클래스 자신임을 의미한다.
    //    - "/myApp/detector/": 생성될 모든 명령어의 기본 디렉토리 경로이다.
    //    - "Detector geometry control commands.": 이 디렉토리에 대한 도움말 설명이다.
    fMessenger = new G4GenericMessenger(this, "/myApp/detector/", "Detector geometry control commands.");

    // 2. 'setMovableAngle' 명령어를 정의하고 Setter 함수와 연결한다.
    //    - DeclareMethod: UI 커맨드가 호출될 때 실행될 클래스 멤버 함수(메서드)를 지정한다.
    //    - &DetectorConstruction::SetMovablePMTAngle: 명령어와 연결할 함수의 포인터이다.
    //    - auto& angleCmd: DeclareMethod가 반환하는 Command 객체에 대한 참조이다.
    //                      이를 통해 명령어의 속성을 연쇄적으로 설정(chaining)할 수 있다.
    auto& angleCmd = fMessenger->DeclareMethod("setMovableAngle", &DetectorConstruction::SetMovablePMTAngle, 
                                               "Set the angle (theta) of the movable PMT (CCW from +X towards +Z).");
    angleCmd.SetUnitCategory("Angle"); // 이 명령어의 파라미터가 '각도' 단위임을 지정한다.
    angleCmd.SetParameterName("Angle", false); // 파라미터 이름은 "Angle"이고, 생략 불가능(false)하다.
    angleCmd.SetStates(G4State_PreInit, G4State_Idle); // /run/beamOn 중에는 이 명령어를 사용할 수 없도록 제한한다. (안전장치)

    // 3. 'setDistance' 명령어를 정의하고 Setter 함수와 연결한다.
    auto& distCmd = fMessenger->DeclareMethod("setDistance", &DetectorConstruction::SetDetectorDistance, 
                                              "Set the distance (R) between the source and detectors.");
    distCmd.SetUnitCategory("Length"); // 이 명령어의 파라미터가 '길이' 단위임을 지정한다.
    distCmd.SetParameterName("Distance", false);
    distCmd.SetStates(G4State_PreInit, G4State_Idle);
}


// ============================================================================
// === Setter 함수들 (원본 코드의 핵심 로직 유지) ===
// ============================================================================

/**
 * @brief 이동형 PMT의 각도를 설정하고, 지오메트리 변경 사실을 RunManager에 알린다.
 * @param angle 새로운 각도 값 (단위 포함 가능, 예: 90*deg)
 */
void DetectorConstruction::SetMovablePMTAngle(G4double angle)
{
    fMovablePMTAngle = angle;
    
    // [!!!가장 중요한 부분!!!]
    // RunManager에게 지오메트리가 변경될 예정임을 알린다.
    // 이 함수가 호출되면, RunManager는 다음 /run/beamOn 명령이 실행되기 직전에
    // 자동으로 DetectorConstruction::Construct() 함수를 다시 호출하여 지오메트리를 업데이트한다.
    G4RunManager::GetRunManager()->GeometryHasBeenModified();
    
    // 사용자 피드백을 위해 변경된 값을 콘솔에 출력한다.
    G4cout << "--> Movable PMT Angle has been set to: " 
           << G4BestUnit(fMovablePMTAngle, "Angle") << G4endl;
}

/**
 * @brief 선원과 검출기 사이의 거리를 설정하고, 지오메트리 변경 사실을 RunManager에 알린다.
 * @param distance 새로운 거리 값 (단위 포함 가능, 예: 20*cm)
 */
void DetectorConstruction::SetDetectorDistance(G4double distance)
{
    fDetectorDistance = distance;
    
    // 위와 동일하게, 지오메트리 변경 사실을 RunManager에 통지한다.
    G4RunManager::GetRunManager()->GeometryHasBeenModified();
    
    G4cout << "--> Detector Distance has been set to: " 
           << G4BestUnit(fDetectorDistance, "Length") << G4endl;
}


// ============================================================================
// === 나머지 함수들 (DefineMaterials, Construct 등) ===
// ============================================================================
//
// DefineMaterials(), ValidateMaterials(), DefineOpticalProperties(),
// Construct(), ConstructDetectorUnit(), ConstructPMT(), ConstructSDandField()
// 함수들은 이미 잘 구현되어 있으므로, 어떠한 변경도 필요하지 않다.
// (이하 코드는 원본과 동일)
//

// === 1. 물질 정의 (DefineMaterials) ===
void DetectorConstruction::DefineMaterials()
{
    auto nist = G4NistManager::Instance();
    G4Element* H = nist->FindOrBuildElement("H");
    G4Element* C = nist->FindOrBuildElement("C");
    G4Element* N = nist->FindOrBuildElement("N");
    G4Element* O = nist->FindOrBuildElement("O");

    if (!H ||!C ||!N ||!O) {
        G4Exception("DetectorConstruction::DefineMaterials()",
                    "FatalError_ElementNotFound", FatalException,
                    "필수 기본 원소(H, C, N, O) 로드에 실패했습니다. 실행 환경 변수(geant4.sh) 설정을 확인하십시오.");
    }

    fAirMaterial = nist->FindOrBuildMaterial("G4_AIR");
    fVacuumMaterial = nist->FindOrBuildMaterial("G4_Galactic");
    fCo60Material = nist->FindOrBuildMaterial("G4_Co");
    fGlassMaterial = nist->FindOrBuildMaterial("G4_Pyrex_Glass");
    fPmtBodyMaterial = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
    fSiliconeMaterial = nist->FindOrBuildMaterial("G4_SILICON_DIOXIDE");

    fEpoxyMaterial = new G4Material("Epoxy", 1.25*g/cm3, 3);
    fEpoxyMaterial->AddElement(H, 8.6*perCent);
    fEpoxyMaterial->AddElement(C, 69.0*perCent);
    fEpoxyMaterial->AddElement(O, 22.4*perCent);

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
        
        G4Exception("DetectorConstruction::ValidateMaterials()", 
                    "FatalError_MaterialNotFound", FatalException, errMsg.str().c_str());
    }
}

// === 3. 광학 속성 정의 (DefineOpticalProperties) ===
void DetectorConstruction::DefineOpticalProperties()
{
    const std::vector<G4double> photonEnergies = {
        2.38*eV, 2.48*eV, 2.58*eV, 2.70*eV, 2.76*eV, 2.82*eV, 
        2.92*eV, 2.95*eV, 3.02*eV, 3.10*eV, 3.26*eV, 3.44*eV
    };

    std::vector<G4double> rindex_air(photonEnergies.size(), 1.0);
    std::vector<G4double> rindex_glass(photonEnergies.size(), 1.47);
    std::vector<G4double> rindex_ls(photonEnergies.size(), 1.50);
    std::vector<G4double> rindex_silicone(photonEnergies.size(), 1.45);

    auto airMPT = new G4MaterialPropertiesTable();
    airMPT->AddProperty("RINDEX", photonEnergies, rindex_air);
    fAirMaterial->SetMaterialPropertiesTable(airMPT);
    fVacuumMaterial->SetMaterialPropertiesTable(airMPT);

    auto glassMPT = new G4MaterialPropertiesTable();
    glassMPT->AddProperty("RINDEX", photonEnergies, rindex_glass);
    fGlassMaterial->SetMaterialPropertiesTable(glassMPT);

    auto siliconeMPT = new G4MaterialPropertiesTable();
    siliconeMPT->AddProperty("RINDEX", photonEnergies, rindex_silicone);
    fSiliconeMaterial->SetMaterialPropertiesTable(siliconeMPT);

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
    
    lsMPT->AddConstProperty("SCINTILLATIONYIELD", 10000./MeV);
    lsMPT->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 10.*ns);
    lsMPT->AddConstProperty("RESOLUTIONSCALE", 1.0);
    
    const std::vector<G4double> birks_energies = { 1.0*eV, 100.0*MeV };
    const std::vector<G4double> birks_constant = { 0.07943*mm/MeV, 0.07943*mm/MeV };
    lsMPT->AddProperty("BIRKSCONSTANT", birks_energies, birks_constant, true);
    
    fLsMaterial->SetMaterialPropertiesTable(lsMPT);
}

// === 지오메트리 구성 (Construct) ===
G4VPhysicalVolume* DetectorConstruction::Construct()
{
    auto solidWorld = new G4Box("SolidWorld", kWorldHalfSize, kWorldHalfSize, kWorldHalfSize);
    auto logicWorld = new G4LogicalVolume(solidWorld, fVacuumMaterial, "LogicWorld");
    auto physWorld = new G4PVPlacement(nullptr, G4ThreeVector(), logicWorld, "PhysWorld", nullptr, false, 0, true);
    logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());

    auto solidEpoxy = new G4Tubs("SolidEpoxy", 0., kEpoxyRadius, kEpoxyHalfZ, 0., CLHEP::twopi);
    auto logicEpoxy = new G4LogicalVolume(solidEpoxy, fEpoxyMaterial, "LogicEpoxy");
    logicEpoxy->SetVisAttributes(new G4VisAttributes(G4Colour(0.8, 0.8, 0.8, 0.3)));

    auto solidSource = new G4Tubs("SolidSource", 0., kSourceRadius, kSourceHalfZ, 0., CLHEP::twopi);
    auto logicSource = new G4LogicalVolume(solidSource, fCo60Material, "LogicSource");
    logicSource->SetVisAttributes(new G4VisAttributes(G4Colour::Blue()));

    new G4PVPlacement(nullptr, G4ThreeVector(), logicSource, "PhysSource", logicEpoxy, false, 0, true);

    auto source_rot = new G4RotationMatrix();
    source_rot->rotateY(90. * deg);
    new G4PVPlacement(source_rot, G4ThreeVector(), logicEpoxy, "PhysEpoxy", logicWorld, false, 0, true);

    G4LogicalVolume* logicDetectorUnit = ConstructDetectorUnit();

    G4double R_ls_center = fDetectorDistance;
    G4double theta = fMovablePMTAngle;
    G4double offset = kAssemblyCenterOffset;
    G4double R_placement = R_ls_center + offset;

    G4ThreeVector pos_fixed(R_placement, 0, 0);
    auto rot_fixed = new G4RotationMatrix();
    rot_fixed->rotateY(90. * deg);
    new G4PVPlacement(rot_fixed, pos_fixed, logicDetectorUnit, "PhysDetectorUnit_Fixed", logicWorld, false, 0, true);

    G4ThreeVector pos_movable(R_placement * std::cos(theta), 0., R_placement * std::sin(theta));
    G4double orientation_angle = 90.*deg + theta;
    auto rot_movable = new G4RotationMatrix();
    rot_movable->rotateY(orientation_angle);
    new G4PVPlacement(rot_movable, pos_movable, logicDetectorUnit, "PhysDetectorUnit_Movable", logicWorld, false, 1, true);

    return physWorld;
}

// === 검출기 유닛 구성 (ConstructDetectorUnit) ===
G4LogicalVolume* DetectorConstruction::ConstructDetectorUnit()
{
    auto solidAssembly = new G4Tubs("SolidAssembly", 0, kAssemblyRadius, kAssemblyHalfZ, 0, CLHEP::twopi);
    auto logicAssembly = new G4LogicalVolume(solidAssembly, fAirMaterial, "LogicAssembly");
    logicAssembly->SetVisAttributes(G4VisAttributes::GetInvisible());

    G4double offset = kAssemblyCenterOffset;

    auto solidBottleBody = new G4Tubs("SolidBottleBody", kBottleInnerRadius, kBottleOuterRadius, kLSHalfZ, 0, CLHEP::twopi);
    auto logicBottleBody = new G4LogicalVolume(solidBottleBody, fGlassMaterial, "LogicBottleBody");
    logicBottleBody->SetVisAttributes(new G4VisAttributes(G4Colour(0.0, 1.0, 1.0, 0.2)));
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, offset), logicBottleBody, "PhysBottleBody", logicAssembly, false, 0, true);

    auto solidLS = new G4Tubs("SolidLS", 0, kBottleInnerRadius, kLSHalfZ, 0, CLHEP::twopi);
    logicLS = new G4LogicalVolume(solidLS, fLsMaterial, "LogicLS");
    logicLS->SetVisAttributes(new G4VisAttributes(G4Colour(1.0, 1.0, 0.0, 0.3)));
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, offset), logicLS, "PhysLS", logicAssembly, false, 0, true);

    auto solidGrease = new G4Tubs("SolidGrease", 0, kBottleOuterRadius, kGreaseHalfZ, 0, CLHEP::twopi);
    auto logicGrease = new G4LogicalVolume(solidGrease, fSiliconeMaterial, "LogicGrease");
    logicGrease->SetVisAttributes(new G4VisAttributes(G4Colour(0.8, 0.8, 0.8, 0.2)));
    G4double grease_Z = offset - kLSHalfZ - kGreaseHalfZ;
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, grease_Z), logicGrease, "PhysGrease", logicAssembly, false, 0, true);

    auto logicPMT = ConstructPMT();
    G4double pmt_Z = grease_Z - kGreaseHalfZ - kPmtAssemblyHalfZ;
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, pmt_Z), logicPMT, "PhysPMT", logicAssembly, false, 0, true);

    return logicAssembly;
}

// === PMT 구성 (ConstructPMT) ===
G4LogicalVolume* DetectorConstruction::ConstructPMT()
{
    auto solidPmtAssembly = new G4Tubs("SolidPmtAssembly", 0, kPmtAssemblyRadius, kPmtAssemblyHalfZ, 0, CLHEP::twopi);
    auto logicPmtAssembly = new G4LogicalVolume(solidPmtAssembly, fVacuumMaterial, "LogicPmtAssembly");
    logicPmtAssembly->SetVisAttributes(G4VisAttributes::GetInvisible());

    auto solidPmtWindow = new G4Tubs("SolidPmtWindow", 0, kPmtWindowRadius, kPmtWindowHalfZ, 0, CLHEP::twopi);
    auto logicPmtWindow = new G4LogicalVolume(solidPmtWindow, fGlassMaterial, "LogicPmtWindow");
    logicPmtWindow->SetVisAttributes(new G4VisAttributes(G4Colour(0.0, 1.0, 1.0, 0.1)));
    G4double window_Z_center = kPmtAssemblyHalfZ - kPmtWindowHalfZ;
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, window_Z_center), logicPmtWindow, "PhysPmtWindow", logicPmtAssembly, false, 0, true);

    auto solidPmtBody = new G4Tubs("SolidPmtBody", kPmtInnerRadius, kPmtBodyRadius, kPmtBodyHalfZ, 0, CLHEP::twopi);
    auto logicPmtBody = new G4LogicalVolume(solidPmtBody, fPmtBodyMaterial, "LogicPmtBody");
    logicPmtBody->SetVisAttributes(new G4VisAttributes(G4Colour(0.7, 0.7, 0.7)));
    G4double body_Z_center = window_Z_center - kPmtWindowHalfZ - kPmtBodyHalfZ;
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, body_Z_center), logicPmtBody, "PhysPmtBody", logicPmtAssembly, false, 0, true);
    
    auto solidPhotocathode = new G4Tubs("SolidPhotocathode", 0, kPmtWindowRadius, kPhotocathodeHalfZ, 0, CLHEP::twopi);
    logicPhotocathode = new G4LogicalVolume(solidPhotocathode, fPmtBodyMaterial, "LogicPhotocathode");
    logicPhotocathode->SetVisAttributes(new G4VisAttributes(G4Colour::Red()));
    G4double cathode_Z_center = window_Z_center - kPmtWindowHalfZ - kPhotocathodeHalfZ;
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, cathode_Z_center), logicPhotocathode, "PhysPhotocathode", logicPmtAssembly, false, 0, true);

    auto photocathode_opsurf = new G4OpticalSurface("Photocathode_OpSurface");
    photocathode_opsurf->SetType(dielectric_metal);
    photocathode_opsurf->SetModel(unified);
    photocathode_opsurf->SetFinish(polished);

    const std::vector<G4double> energies_qe = {1.9*eV, 2.1*eV, 2.4*eV, 2.7*eV, 2.9*eV, 3.1*eV, 3.3*eV, 3.5*eV, 3.7*eV, 4.1*eV};
    const std::vector<G4double> efficiency = {0.02, 0.08, 0.18, 0.24, 0.28, 0.26, 0.22, 0.15, 0.05, 0.01};
    
    auto pmtMPT = new G4MaterialPropertiesTable();
    pmtMPT->AddProperty("EFFICIENCY", energies_qe, efficiency);
    photocathode_opsurf->SetMaterialPropertiesTable(pmtMPT);

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
