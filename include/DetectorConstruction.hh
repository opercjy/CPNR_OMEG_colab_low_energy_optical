// DetectorConstruction.hh (리팩토링 완료)

#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

// --- [리팩토링] 클래스 전방 선언 수정 ---
// G4GenericMessenger를 사용할 것이므로, 기존 DetectorMessenger 대신 이를 전방 선언.
// 이렇게 하면 헤더 파일에 불필요한 #include를 줄여 컴파일 속도를 향상시킬 수 있다.
class G4GenericMessenger; 

class G4VPhysicalVolume;
class G4LogicalVolume;
class G4Material;

/**
 * @class DetectorConstruction
 * @brief 시뮬레이션 환경의 모든 물질과 기하학적 구조를 생성하는 클래스.
 *        UI 커맨드를 통해 런타임에 검출기 각도와 거리를 동적으로 변경할 수 있다.
 */
class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
    // --- Public 인터페이스 (변경 없음) ---
    // 이 클래스의 생성, 소멸, 지오메트리 구성, SD 설정 함수들은 외부에서 호출되므로 public으로 유지.
    DetectorConstruction();
    virtual ~DetectorConstruction();
    virtual G4VPhysicalVolume* Construct() override;
    virtual void ConstructSDandField() override;

    // --- Public Setter 함수 (변경 없음) ---
    // 이 함수들은 UI 커맨드에 의해 직접 호출될 중요한 인터페이스.
    // 매크로에서 /myApp/detector/setMovableAngle 명령어를 사용하면 이 함수가 호출.
    void SetMovablePMTAngle(G4double angle);
    // 매크로에서 /myApp/detector/setDistance 명령어를 사용하면 이 함수가 호출.
    void SetDetectorDistance(G4double distance);

private:
    // --- Private 도우미 함수 (Helper Methods) ---
    // 복잡한 로직을 작은 단위의 함수로 분리하여 코드의 가독성과 재사용성을 높임.
    void DefineMaterials();
    void ValidateMaterials();
    void DefineOpticalProperties();
    G4LogicalVolume* ConstructDetectorUnit();
    G4LogicalVolume* ConstructPMT();
    
    // [!리팩토링 핵심!] UI 커맨드 정의를 위한 전용 함수를 선언.
    // 생성자 로직을 깔끔하게 유지하고, UI 관련 코드를 한 곳에 모아 관리하기 위함.
    void DefineCommands();

    // --- 물질 포인터 및 멤버 변수들 (변경 없음) ---
    G4Material* fAirMaterial; G4Material* fLsMaterial; G4Material* fCo60Material;
    G4Material* fGlassMaterial; G4Material* fPmtBodyMaterial; G4Material* fSiliconeMaterial;
    G4Material* fVacuumMaterial; G4Material* fEpoxyMaterial; 

    // 지오메트리 제어 변수
    G4double fMovablePMTAngle; // 사잇각 (θ)
    G4double fDetectorDistance;  // 거리 (r): 선원 중심 -> LS 중심

    // --- [리팩토링] Messenger 포인터 교체 ---
    // 기존 DetectorMessenger* 대신 G4GenericMessenger*를 사용합니다.
    // 이를 통해 별도의 Messenger 클래스 파일 없이 DetectorConstruction 내에서 직접 UI 커맨드를 정의할 수 있다.
    G4GenericMessenger* fMessenger;

    // Sensitive Detector 할당을 위한 논리 볼륨 포인터 (변경 없음)
    G4LogicalVolume* logicLS;
    G4LogicalVolume* logicPhotocathode;

public:
    // --- 지오메트리 상수 정의 (변경 없음) ---
    // static constexpr을 사용하여 컴파일 타임 상수를 정의하는 것은 매우 좋은 현대 C++ 관행.
    // 코드의 가독성을 높이고, "매직 넘버" 사용을 방지.
    static constexpr G4double kWorldHalfSize = 1.5*CLHEP::m;
    static constexpr G4double kDefaultDistance = 20.0*CLHEP::cm;
    static constexpr G4double kDefaultAngle = 45.0*CLHEP::deg;
    static constexpr G4double kSourceRadius = 2.5*CLHEP::mm;
    static constexpr G4double kSourceHalfZ = 0.5*CLHEP::mm;
    static constexpr G4double kEpoxyRadius = 12.5*CLHEP::mm;
    static constexpr G4double kEpoxyHalfZ = 1.5*CLHEP::mm;
    static constexpr G4double kBottleOuterRadius = 28.0*CLHEP::mm;
    static constexpr G4double kBottleThickness = 2.0*CLHEP::mm;
    static constexpr G4double kLSHalfZ = 40.0*CLHEP::mm;
    static constexpr G4double kBottleInnerRadius = kBottleOuterRadius - kBottleThickness;
    static constexpr G4double kGreaseHalfZ = 0.5*CLHEP::mm;
    static constexpr G4double kPmtAssemblyRadius = 30.0*CLHEP::mm;
    static constexpr G4double kPmtAssemblyHalfZ = 107.5*CLHEP::mm;
    static constexpr G4double kPmtWindowRadius = 26.5*CLHEP::mm;
    static constexpr G4double kPmtWindowHalfZ = 2.5*CLHEP::mm;
    static constexpr G4double kPhotocathodeHalfZ = 0.05*CLHEP::mm;
    static constexpr G4double kPmtBodyRadius = 29.5*CLHEP::mm;
    static constexpr G4double kPmtBodyThickness = 1.5*CLHEP::mm;
    static constexpr G4double kPmtInnerRadius = kPmtBodyRadius - kPmtBodyThickness;
    static constexpr G4double kPmtBodyHalfZ = kPmtAssemblyHalfZ - kPmtWindowHalfZ;
    static constexpr G4double kAssemblyRadius = 35*CLHEP::mm;
    static constexpr G4double kAssemblyTotalLength = 2*kLSHalfZ + 2*kGreaseHalfZ + 2*kPmtAssemblyHalfZ;
    static constexpr G4double kAssemblyHalfZ = kAssemblyTotalLength / 2.0;
    static constexpr G4double kAssemblyCenterOffset = kAssemblyHalfZ - kLSHalfZ;
};

#endif
