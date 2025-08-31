#!/bin/bash

# =============================================================
# Geant4 시뮬레이션 자동 실행 스크립트 (거리/각도 제어)
# =============================================================

# --- 시뮬레이션 매개변수 설정 ---
DISTANCES=(10 20 30) # [!+] 테스트할 거리 목록 (cm 단위)
ANGLES=(0 30 60 90 120 150 180)
NUM_EVENTS=100000
TEMPLATE_MAC="run_angular_template.mac"
BUILD_DIR="./build"

# --- 시뮬레이션 실행 루프 ---
echo "===== Starting Batch Simulations for Distance/Angular Dependence ====="

# [!+] 거리(distance)에 대한 외부 루프 추가
for distance in "${DISTANCES[@]}"; do
  # 각도(angle)에 대한 내부 루프
  for angle in "${ANGLES[@]}"; do
    
    echo ""
    echo "--- Running for Distance = ${distance} cm, Angle = ${angle} deg ---"

    # 1. 현재 설정에 맞는 결과 파일 이름 생성
    output_filename="output_dist_${distance}_angle_${angle}"
    
    # 2. 임시 매크로 파일 생성 (거리/각도 플레이스홀더 모두 치환)
    sed -e "s/DISTANCE_PLACEHOLDER/${distance}/" \
        -e "s/ANGLE_PLACEHOLDER/${angle}/" \
        -e "s/FILENAME_PLACEHOLDER/${output_filename}/" \
        -e "s/beamOn 100000/beamOn ${NUM_EVENTS}/" \
        "${TEMPLATE_MAC}" > "${BUILD_DIR}/tmp_run.mac"

    # 3. Geant4 시뮬레이션 실행
    (cd "${BUILD_DIR}" && ./CPNR_OMEG_colab_low_energy_optical tmp_run.mac)
    
    echo "--- Finished: ${output_filename}.root created ---"

  done
done

# 4. 임시 매크로 파일 삭제
rm "${BUILD_DIR}/tmp_run.mac"

echo ""
echo "===== All simulations completed. ====="
