// CPNR_OMEG_colab_low_energy_optical.cc (최종 안정화 버전)
// 시뮬레이션의 시작점(entry point)이며, 전체 실행 흐름을 제어합니다.

#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4Threading.hh"
#include "G4ScoringManager.hh" // 복셀화(Scoring Mesh) 기능을 위해 필요

#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "ActionInitialization.hh"

int main(int argc, char** argv)
{
  // 1. UI 세션 감지 (터미널 인자가 없으면 GUI 모드로 판단)
  G4UIExecutive* ui = nullptr;
  if (argc == 1) {
    ui = new G4UIExecutive(argc, argv);
  }

  // 2. 실행 모드에 따라 적합한 RunManager 생성
  // GUI 모드에서는 시각화 문제 방지를 위해 단일 스레드(SerialOnly)로 실행하고,
  // 배치 모드에서는 성능을 위해 멀티 스레드(Default)로 실행합니다.
  auto* runManager = G4RunManagerFactory::CreateRunManager(
      (ui) ? G4RunManagerType::SerialOnly : G4RunManagerType::Default
  );
  if (!ui) {
    runManager->SetNumberOfThreads(G4Threading::G4GetNumberOfCores());
  }

  // 3. 스코어링 매니저 활성화
  // 이 객체를 활성화해야 매크로에서 /score/ UI 명령어들을 사용할 수 있습니다.
  G4ScoringManager::GetScoringManager();

  // 4. 시각화 관리자 생성 및 초기화
  G4VisManager* visManager = new G4VisExecutive;
  visManager->Initialize();

  // 5. 필수 사용자 클래스들을 RunManager에 등록
  runManager->SetUserInitialization(new DetectorConstruction());
  runManager->SetUserInitialization(new PhysicsList());
  runManager->SetUserInitialization(new ActionInitialization());
  
  // 6. Geant4 커널 초기화
  // 이 함수가 호출된 이후에야 /run/beamOn, /gps/... 등의 명령어를 사용할 수 있습니다.
  runManager->Initialize();

  // 7. UI 관리자 포인터 가져오기
  G4UImanager* UImanager = G4UImanager::GetUIpointer();

  // 8. 실행 모드에 따라 적절한 매크로 실행
  if (ui) {
    // ## 인터랙티브(GUI) 모드 ##
    UImanager->ApplyCommand("/control/execute init_vis_angular.mac");
    ui->SessionStart(); // 사용자가 GUI를 닫을 때까지 대기
    delete ui;
  }
  else {
    // ## 배치 모드 ##
    G4String command = "/control/execute ";
    G4String fileName = argv[1]; // 터미널에서 두 번째 인자로 받은 파일 이름
    UImanager->ApplyCommand(command + fileName);
  }

  // 9. 프로그램 종료 전 메모리 해제
  delete visManager;
  delete runManager;

  return 0;
}
