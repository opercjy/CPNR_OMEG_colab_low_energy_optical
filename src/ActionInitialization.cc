#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"
#include "TrackingAction.hh"

/**
 * @brief 생성자: 기본 생성자를 호출합니다.
 */
ActionInitialization::ActionInitialization() : G4VUserActionInitialization() {}

/**
 * @brief 소멸자: 관련된 메모리 해제는 Geant4 커널이 담당합니다.
 */
ActionInitialization::~ActionInitialization() {}

/**
 * @brief Master 스레드용 Action 클래스를 생성합니다.
 * Master 스레드는 전체 Run의 시작과 끝만 관리하므로 RunAction만 필요합니다.
 * 데이터 파일의 생성과 저장이 여기서 한 번만 일어나도록 보장합니다.
 */
void ActionInitialization::BuildForMaster() const
{
  SetUserAction(new RunAction());
}

/**
 * @brief Worker 스레드용 Action 클래스들을 생성합니다.
 * 각 Worker 스레드는 독립적으로 이벤트를 처리하므로, 이벤트 생성부터 데이터 수집까지
 * 모든 Action 클래스들이 필요합니다.
 */
void ActionInitialization::Build() const
{
  SetUserAction(new PrimaryGeneratorAction());
  SetUserAction(new RunAction());
  SetUserAction(new EventAction());
  SetUserAction(new SteppingAction());
  SetUserAction(new TrackingAction());
}
