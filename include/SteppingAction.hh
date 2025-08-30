#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "G4UserSteppingAction.hh"

/**
 * @class SteppingAction
 * @brief 입자의 모든 스텝(step)마다 호출되는 클래스입니다.
 *
 * 이 프로젝트에서는 데이터 수집 로직을 G4VSensitiveDetector (LSSD)로 이전했기 때문에,
 * 이 클래스의 내용은 의도적으로 비워두었습니다.
 */
class SteppingAction : public G4UserSteppingAction
{
public:
  SteppingAction();
  virtual ~SteppingAction();
  virtual void UserSteppingAction(const G4Step*) override;
};

#endif
