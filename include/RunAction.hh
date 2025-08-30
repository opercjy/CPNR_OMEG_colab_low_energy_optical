#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"
#include "globals.hh"

/**
 * @class RunAction
 * @brief Run의 시작과 끝에서 수행할 작업을 정의하는 클래스입니다.
 *
 * 주로 데이터 파일(ROOT)을 열고 닫으며, 생성자에서 저장할 TTree의 구조를 정의합니다.
 */
class RunAction : public G4UserRunAction
{
public:
  RunAction();
  virtual ~RunAction();

  virtual void BeginOfRunAction(const G4Run*) override;
  virtual void EndOfRunAction(const G4Run*) override;
};

#endif
