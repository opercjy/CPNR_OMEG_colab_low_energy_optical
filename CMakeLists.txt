# ============================================================================
# CPNR_OMEG_colab_low_energy_optical Geant4 프로젝트용 CMakeLists.txt
# ============================================================================

# 최소 CMake 버전 요구사항 지정
cmake_minimum_required(VERSION 3.16)

# 프로젝트 이름 설정 (실행 파일 이름으로 사용됨)
project(CPNR_OMEG_colab_low_energy_optical)

# --- 패키지 검색 ---
# Geant4와 ROOT 라이브러리를 찾고, 필요한 컴포넌트를 지정합니다.
# ui_all, vis_all: Geant4의 모든 UI 및 시각화 라이브러리를 포함합니다.
find_package(Geant4 REQUIRED ui_all vis_all)
find_package(ROOT REQUIRED COMPONENTS Core Graf Tree)

# --- Geant4 및 프로젝트 헤더 파일 경로 설정 ---
# Geant4의 헤더 파일 및 라이브러리 설정을 현재 프로젝트에 포함시킵니다.
include(${Geant4_USE_FILE})

# 우리 프로젝트의 헤더 파일이 있는 'include' 디렉토리를 경로에 추가합니다.
include_directories(${PROJECT_SOURCE_DIR}/include)

# --- 소스 파일 목록 정의 ---
# GLOB 대신 소스 파일 목록을 명시적으로 나열하여,
# 의도치 않은 파일이 포함되는 것을 방지하고 빌드 시스템의 안정성을 높입니다.
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/ActionInitialization.cc
    ${PROJECT_SOURCE_DIR}/src/DetectorConstruction.cc
    ${PROJECT_SOURCE_DIR}/src/EventAction.cc
    ${PROJECT_SOURCE_DIR}/src/LSHit.cc
    ${PROJECT_SOURCE_DIR}/src/LSSD.cc
    ${PROJECT_SOURCE_DIR}/src/PMTHit.cc
    ${PROJECT_SOURCE_DIR}/src/PMTSD.cc
    ${PROJECT_SOURCE_DIR}/src/PhysicsList.cc
    ${PROJECT_SOURCE_DIR}/src/PrimaryGeneratorAction.cc
    ${PROJECT_SOURCE_DIR}/src/RunAction.cc
    ${PROJECT_SOURCE_DIR}/src/SteppingAction.cc
    ${PROJECT_SOURCE_DIR}/src/TrackingAction.cc
)

# --- 실행 파일 생성 및 라이브러리 연결 ---
# 메인 소스 파일과 위에서 정의한 소스 파일 목록을 합쳐 실행 파일을 생성합니다.
add_executable(${PROJECT_NAME} ${PROJECT_NAME}.cc ${PROJECT_SOURCES})

# 생성된 실행 파일에 Geant4와 ROOT 라이브러리를 연결(link)합니다.
target_link_libraries(${PROJECT_NAME} PRIVATE ${Geant4_LIBRARIES} ${ROOT_LIBRARIES})

# --- 매크로 파일 복사 ---
# 시뮬레이션 실행에 필요한 매크로(.mac) 파일들을
# 소스 디렉토리에서 빌드 디렉토리로 자동으로 복사합니다.
set(PROJECT_SCRIPTS
  init_vis_angular.mac
  run_angular_template.mac
)
foreach(_script ${PROJECT_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
  )
endforeach()

# --- 설치 (선택 사항) ---
# 'make install' 명령을 사용할 경우, 실행 파일과 매크로를 지정된 위치에 설치합니다.
install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
)
install(FILES ${PROJECT_SCRIPTS}
  DESTINATION share/
)
