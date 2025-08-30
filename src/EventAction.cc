#include "EventAction.hh"
#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4SystemOfUnits.hh"

// 데이터 저장을 위해 Hit 클래스 헤더들을 포함합니다.
#include "LSHit.hh"
#include "PMTHit.hh"

// std::set을 사용하여 중복된 트랙을 효율적으로 제거하기 위해 헤더를 포함합니다.
#include <set>

EventAction::EventAction() : G4UserEventAction() {}
EventAction::~EventAction() {}

/**
 * @brief 각 이벤트가 끝날 때마다 호출되는 함수입니다.
 * @param event 현재 이벤트에 대한 정보를 담고 있는 G4Event 객체 포인터
 *
 * 이 함수는 LSSD와 PMTSD에서 수집된 HitsCollection을 분석하여,
 * 1) 이벤트 요약 정보(LS 입자 수)를 계산하여 'EventSummary' TTree에 저장하고,
 * 2) 상세 Hit 정보(에너지 증착)를 'Hits' TTree에 저장하며,
 * 3) PMT에서 검출된 광자 정보를 'PMTHits' TTree에 저장하는 역할을 수행합니다.
 */
void EventAction::EndOfEventAction(const G4Event* event)
{
  auto analysisManager = G4AnalysisManager::Instance();
  G4int eventID = event->GetEventID();

  // --- LS 데이터 처리 (LSHitsCollection) ---
  G4int lsHcID = G4SDManager::GetSDMpointer()->GetCollectionID("LSHitsCollection");
  if (lsHcID >= 0) {
    auto lsHitsCollection = static_cast<LSHitsCollection*>(event->GetHCofThisEvent()->GetHC(lsHcID));
    if (lsHitsCollection && lsHitsCollection->entries() > 0) {
      // 1-1. 이벤트 요약 정보 계산
      G4int primaryCount = 0;
      G4int secondaryCount = 0;
      std::set<G4int> countedTracks;

      for (size_t i = 0; i < lsHitsCollection->entries(); ++i) {
        auto hit = (*lsHitsCollection)[i];
        G4int trackID = hit->GetTrackID();
        if (countedTracks.find(trackID) == countedTracks.end()) {
          countedTracks.insert(trackID);
          if (hit->GetParentID() == 0) primaryCount++;
          else secondaryCount++;
        }
      }
      
      // 1-2. EventSummary TTree (Ntuple ID=1)에 저장
      analysisManager->FillNtupleIColumn(1, 0, eventID);
      analysisManager->FillNtupleIColumn(1, 1, primaryCount);
      analysisManager->FillNtupleIColumn(1, 2, secondaryCount);
      analysisManager->AddNtupleRow(1);

      // 1-3. Hits TTree (Ntuple ID=0)에 상세 정보 저장
      for (size_t i = 0; i < lsHitsCollection->entries(); ++i) {
        auto hit = (*lsHitsCollection)[i];
        analysisManager->FillNtupleIColumn(0, 0, eventID);
        analysisManager->FillNtupleIColumn(0, 1, hit->GetTrackID());
        analysisManager->FillNtupleIColumn(0, 2, hit->GetParentID());
        analysisManager->FillNtupleSColumn(0, 3, hit->GetParticleName());
        analysisManager->FillNtupleSColumn(0, 4, hit->GetProcessName());
        analysisManager->FillNtupleSColumn(0, 5, hit->GetVolumeName());
        analysisManager->FillNtupleDColumn(0, 6, hit->GetPosition().x() / mm);
        analysisManager->FillNtupleDColumn(0, 7, hit->GetPosition().y() / mm);
        analysisManager->FillNtupleDColumn(0, 8, hit->GetPosition().z() / mm);
        analysisManager->FillNtupleDColumn(0, 9, hit->GetTime());
        analysisManager->FillNtupleDColumn(0, 10, hit->GetKineticEnergy());
        analysisManager->FillNtupleDColumn(0, 11, hit->GetEnergyDeposit());
        analysisManager->AddNtupleRow(0);
      }
    }
  }
  
  // --- PMT 데이터 처리 (PMTHitsCollection) ---
  G4int pmtHcID = G4SDManager::GetSDMpointer()->GetCollectionID("PMTHitsCollection");
  if (pmtHcID >= 0) {
    auto pmtHitsCollection = static_cast<PMTHitsCollection*>(event->GetHCofThisEvent()->GetHC(pmtHcID));
    if (pmtHitsCollection && pmtHitsCollection->entries() > 0) {
      // 모든 PMT Hit을 순회하며 TTree에 직접 저장
      for (size_t i = 0; i < pmtHitsCollection->entries(); ++i) {
        auto pmtHit = (*pmtHitsCollection)[i];
        analysisManager->FillNtupleIColumn(2, 0, eventID);
        analysisManager->FillNtupleIColumn(2, 1, pmtHit->GetPMTID());
        analysisManager->FillNtupleDColumn(2, 2, pmtHit->GetTime());
        analysisManager->AddNtupleRow(2);
      }
    }
  }
}
