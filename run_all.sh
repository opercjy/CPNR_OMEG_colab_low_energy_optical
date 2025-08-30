#!/bin/bash

# =============================================================
# Geant4 시뮬레이션 자동 실행 스크립트
# =============================================================

# --- 시뮬레이션 매개변수 설정 ---
# 테스트할 각도 목록 (degree 단위)
ANGLES=(0 30 60 90 120 150 180)
# 이벤트 수
NUM_EVENTS=100000
# 템플릿 매크로 파일 이름
TEMPLATE_MAC="run_angular_template.mac"
# Geant4 실행 파일이 있는 빌드 디렉토리
BUILD_DIR="./build"

# --- 시뮬레이션 실행 루프 ---
echo "===== Starting Batch Simulations for Angular Dependence ====="

# 각도(theta)에 대한 루프
for angle in "${ANGLES[@]}"; do
  
  echo ""
  echo "--- Running for Angle = ${angle} deg ---"

  # 1. 현재 설정에 맞는 결과 파일 이름 생성
  output_filename="output_angle_${angle}"
  
  # 2. 임시 매크로 파일 생성
  #    sed 명령어를 사용하여 템플릿의 플레이스홀더를 실제 값으로 치환
  sed -e "s/ANGLE_PLACEHOLDER/${angle}/" \
      -e "s/FILENAME_PLACEHOLDER/${output_filename}/" \
      -e "s/beamOn 100000/beamOn ${NUM_EVENTS}/" \
      "${TEMPLATE_MAC}" > "${BUILD_DIR}/tmp_run.mac"

  # 3. Geant4 시뮬레이션 실행
  (cd "${BUILD_DIR}" && ./CPNR_OMEG_colab_low_energy_optical tmp_run.mac)
  
  echo "--- Finished: ${output_filename}.root created ---"

done

# 4. 임시 매크로 파일 삭제
rm "${BUILD_DIR}/tmp_run.mac"

echo ""
echo "===== All simulations completed. ====="
