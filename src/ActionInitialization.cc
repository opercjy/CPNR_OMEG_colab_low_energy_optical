#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"
#include "TrackingAction.hh"

ActionInitialization::ActionInitialization() : G4VUserActionInitialization() {}

ActionInitialization::~ActionInitialization() {}

/**
 * @brief Master 스레드용 Action 클래스를 생성합니다.
 * Master 스레드는 전체 Run의 시작과 끝만 관리하므로 RunAction만 필요합니다.
 */
void ActionInitialization::BuildForMaster() const
{
  SetUserAction(new RunAction());
}

/**
 * @brief Worker 스레드용 Action 클래스들을 생성합니다.
 * 각 Worker 스레드는 독립적으로 이벤트를 처리하므로 모든 Action 클래스가 필요합니다.
 */
void ActionInitialization::Build() const
{
  SetUserAction(new PrimaryGeneratorAction());
  SetUserAction(new RunAction());
  SetUserAction(new EventAction());
  SetUserAction(new SteppingAction());
  SetUserAction(new TrackingAction());
}
