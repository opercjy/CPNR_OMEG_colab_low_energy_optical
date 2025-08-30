#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

/**
 * @class EventAction
 * @brief 각 이벤트(Event)의 시작과 끝에서 필요한 작업을 수행하는 클래스입니다.
 *
 * 이벤트가 끝날 때마다 LSHitsCollection과 PMTHitsCollection을 분석하여
 * 정의된 모든 TTree에 데이터를 기록하는 핵심적인 역할을 합니다.
 */
class EventAction : public G4UserEventAction
{
public:
  EventAction();
  virtual ~EventAction();

  virtual void EndOfEventAction(const G4Event*) override;
};

#endif
