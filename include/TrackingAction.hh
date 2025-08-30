#ifndef TrackingAction_h
#define TrackingAction_h 1

#include "G4UserTrackingAction.hh"
#include "globals.hh"

/**
 * @class TrackingAction
 * @brief 입자 하나의 트랙(생성부터 소멸까지) 단위로 작업을 수행하는 클래스입니다.
 *
 * 현재 프로젝트에서는 사용하고 있지 않지만, 향후 확장을 위한 틀(placeholder)로 남겨두었습니다.
 */
class TrackingAction : public G4UserTrackingAction
{
public:
  TrackingAction();
  virtual ~TrackingAction();

  virtual void PreUserTrackingAction(const G4Track* track) override;
  virtual void PostUserTrackingAction(const G4Track* track) override;
};

#endif
