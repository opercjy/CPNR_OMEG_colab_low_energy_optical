
# CPNR_OMEG_colab_low_energy_optical: Geant4 기반 광학 시뮬레이션

## 1. 프로젝트 개요 (Overview)

본 프로젝트는 `CPNR-OMEG-colab`의 후속 프로젝트로, ⁶⁰Co 선원에서 발생하는 방사선을 액체섬광체(LS)와 두 개의 광전증배관(PMT)으로 측정하는 **저에너지 광학 시뮬레이션**입니다. Geant4의 광학 물리(Optical Physics) 프로세스를 적용하여 섬광(Scintillation) 현상을 정밀하게 모사하고, PMT의 위치와 각도를 변경하며 각도 의존성(Angular Dependence)을 분석하는 것을 목표로 합니다.

### 주요 특징 (Key Features)

* **상세한 지오메트리:** 듀란(Duran) 병, 뚜껑, Hamamatsu H7195 PMT 모델, 광학 커플링(실리콘) 등 실제 실험 환경과 유사한 구조를 구현합니다.
* **광학 물리 시뮬레이션:** `G4OpticalPhysics`를 통해 섬광 광자의 생성, 전파, 반사, 굴절, 흡수 등 모든 광학 현상을 시뮬레이션합니다.
* **각도 의존성 연구:** 두 개의 PMT 중 하나는 고정하고, 다른 하나는 선원을 중심으로 회전시켜 각도에 따른 광자 검출 효율을 측정할 수 있습니다.
* **동시 계수 분석 (Coincidence Counting):** 두 PMT에 특정 시간 범위(예: 50ns) 내에 동시에 신호가 들어온 사건만 필터링하여 분석하는 기능을 포함합니다.
* **정밀 데이터 수집:**
    * **TTree:** LS와 PMT에서 발생하는 모든 물리적 상호작용 및 각 PMT에 검출된 광자 정보를 이벤트 단위로 저장합니다.
    * **CSV:** LS 영역을 3차원 복셀로 나누어 각 셀의 흡수 선량을 저장합니다.

---
## 2. 시스템 요구사항 (Prerequisites)

본 시뮬레이션은 모든 사용자에게 동일한 결과를 보장하기 위해 Docker 환경을 표준으로 사용합니다.

* **Docker:** 컨테이너 기술을 실행하기 위해 필수적입니다.
    * [Docker 공식 홈페이지 (설치 안내)](https://www.docker.com/get-started)
* **X11 서버 (GUI 모드용):** Docker 컨테이너의 그래픽 출력을 호스트 PC 화면에 표시하기 위해 필요합니다.
    * **Windows:** [VcXsrv](https://sourceforge.net/projects/vcxsrv/) 또는 [Xming](https://sourceforge.net/projects/xming/) 설치를 권장합니다.
    * **macOS:** [XQuartz](https://www.xquartz.org/) 설치가 필요합니다.
    * **Linux:** 대부분의 데스크탑 환경에 기본적으로 포함되어 있습니다.

---
## 3. 환경 구축 (Environment Setup)

아래 과정은 최초 한 번만 수행하면, 언제든지 동일한 시뮬레이션 환경을 불러올 수 있습니다.

### 3.1. Docker 이미지 빌드 (최초 1회)

이 과정은 AlmaLinux 9 컨테이너 위에서 Geant4와 ROOT를 소스 코드로 직접 컴파일하여 우리 프로젝트만을 위한 완벽한 맞춤형 OS 이미지를 만드는 과정입니다.

**1) AlmaLinux 9 컨테이너 시작**
```bash
# 이 프로젝트의 최상위 디렉토리에서 아래 명령어를 실행
docker run -it --name g4builder -v "$(pwd)":/work -w /work almalinux:9 bash
```


**2) 컨테이너 내부에서 필수 패키지 설치**
컨테이너 터미널(`[root@... work]#`) 안에서 아래 명령어를 실행하여 모든 개발 도구와 라이브러리를 설치합니다.

```bash
# 시스템 업데이트 및 기본 개발 도구 설치
dnf update -y && dnf groupinstall "Development Tools" -y && dnf install -y epel-release && dnf config-manager --set-enabled crb

# Geant4와 ROOT 빌드에 필요한 모든 라이브러리 설치
dnf install -y cmake expat-devel xerces-c-devel libX11-devel libXext-devel libXmu-devel libXpm-devel libXft-devel mesa-libGL-devel mesa-libGLU-devel qt6-qtbase-devel 'qt6-*-devel' python3-devel openssl-devel python3-pip
```

**3) ROOT 소스 코드 빌드 및 설치**

```bash
cd /usr/local/src
# 최신 버전은 ROOT 웹사이트에서 확인: [https://root.cern/install/](https://root.cern/install/)
wget [https://root.cern/download/root_v6.32.04.source.tar.gz](https://root.cern/download/root_v6.32.04.source.tar.gz)
tar -xzvf root_v6.32.04.source.tar.gz
mkdir root_build && cd root_build
cmake ../root-6.32.04 -DCMAKE_INSTALL_PREFIX=/usr/local/root -Dall=ON
make -j$(nproc) && make install
```

**4) Geant4 소스 코드 빌드 및 설치**

```bash
cd /usr/local/src
# 최신 버전은 Geant4 웹사이트에서 확인: [https://geant4.web.cern.ch/download](https://geant4.web.cern.ch/download)
wget [https://geant4-data.web.cern.ch/geant4-data/releases/geant4-v11.2.2.tar.gz](https://geant4-data.web.cern.ch/geant4-data/releases/geant4-v11.2.2.tar.gz)
tar -xzvf geant4-v11.2.2.tar.gz
mkdir geant4_build && cd geant4_build
source /usr/local/root/bin/thisroot.sh # ROOT 환경을 먼저 활성화
cmake ../geant4-v11.2.2 -DCMAKE_INSTALL_PREFIX=/usr/local/geant4 -DGEANT4_BUILD_MULTITHREADED=ON -DGEANT4_USE_QT=ON -DGEANT4_USE_ROOT=ON -DGEANT4_INSTALL_DATA=ON
make -j$(nproc) && make install
```

**5) 완성된 환경을 새 이미지로 저장 (매우 중요)**
모든 설치가 완료되면, `exit`으로 컨테이너를 종료합니다. 그 다음, **호스트 머신의 새 터미널 창**에서 아래 명령어를 실행하여 지금까지의 모든 작업 내용을 `my-g4-optical-env:1.0`이라는 새로운 이미지로 저장합니다.

```bash
docker commit g4builder my-g4-optical-env:1.0
```

(선택) 생성된 이미지를 Docker Hub에 push하면 다른 컴퓨터에서도 쉽게 내려받아 사용할 수 있습니다.
`docker login`, `docker tag my-g4-optical-env:1.0 your-dockerhub-id/my-g4-optical-env:1.0`, `docker push your-dockerhub-id/my-g4-optical-env:1.0`

### 3.2. 호스트 및 컨테이너 환경 설정

**1) 호스트 머신에 Helper 함수 추가 (`.bashrc`)**
\*\*호스트 머신(로컬 PC)\*\*의 `~/.bashrc` 또는 `.zshrc` 파일에 아래 함수를 추가하면, 복잡한 Docker 실행 명령어를 `rung4`라는 별칭(alias)으로 간단하게 사용할 수 있습니다.

```bash
# Geant4 개발용 Docker 컨테이너를 실행하는 함수
function rung4() {
    local image_name=${1:-"my-g4-optical-env:1.0"}
    echo "Starting container with image: ${image_name}"

    xhost +

    local runtime_dir="/tmp/runtime-$(id -u)"
    mkdir -p "${runtime_dir}"
    chmod 0700 "${runtime_dir}"

    docker run -it --rm \
        -e DISPLAY=$DISPLAY \
        -v /tmp/.X11-unix:/tmp/.X11-unix \
        -e XDG_RUNTIME_DIR="${runtime_dir}" \
        -v "${runtime_dir}":"${runtime_dir}" \
        --name g4dev \
        -v "$(pwd)":/work \
        -w /work \
        "${image_name}" bash
}
```

추가 후 `source ~/.bashrc` 명령어로 적용하거나 새 터미널을 엽니다.

**2) 컨테이너 환경 변수 영구 설정 (`.bashrc` in Container)**
매번 `source` 명령어를 입력하는 것을 피하기 위해, **컨테이너 내부의** `~/.bashrc`에 환경 변수를 등록합니다.

```bash
# rung4 명령으로 컨테이너에 최초 접속한 뒤, 아래 명령어들을 한 번만 실행
echo '' >> ~/.bashrc
echo '# Auto-load Geant4 and ROOT environments' >> ~/.bashrc
echo 'source /usr/local/geant4/bin/geant4.sh' >> ~/.bashrc
echo 'source /usr/local/root/bin/thisroot.sh' >> ~/.bashrc
```

-----

## 4\. 빌드 및 실행

### 4.1. 프로젝트 빌드

C++ 코드를 수정한 후에는 항상 다시 빌드해야 합니다. 특히 헤더 파일(`.hh`) 수정 시에는 **클린 빌드**를 권장합니다.

```bash
# 1. 프로젝트 최상위 디렉토리에서 컨테이너 시작
rung4

# 2. 컨테이너 내부에서 클린 빌드 수행
# (기존 빌드 디렉토리가 있다면 삭제)
rm -rf build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 4.2. 실행

#### GUI 모드 (시각화 및 디버깅)

`build` 디렉토리에서 인자 없이 실행합니다. `init_vis_angular.mac`이 실행됩니다.

```bash
./CPNR_OMEG_colab_low_energy_optical
```

#### 배치 모드 (각도별 데이터 생산)

`build` 디렉토리에서 `run_angular_template.mac` 파일을 인자로 실행합니다. 쉘 스크립트를 이용하면 여러 각도와 거에 대한 실행을 자동화할 수 있습니다.

```bash
#!/bin/bash

# =============================================================
# Geant4 시뮬레이션 자동 실행 스크립트
# 거리와 각도를 변경하며 여러 배치 작업을 수행합니다.
# =============================================================

# --- 시뮬레이션 매개변수 설정 ---
# 테스트할 거리 목록 (cm 단위)
DISTANCES=(15 20 25 30)
# 테스트할 각도 목록 (degree 단위)
ANGLES=(0 30 60 90 120 150 180)

# Geant4 실행 파일이 있는 빌드 디렉토리
BUILD_DIR="./build"
# 템플릿 매크로 파일 이름
TEMPLATE_MAC="run_angular_template.mac"

# --- 시뮬레이션 실행 루프 ---
echo "===== Starting Batch Simulations ====="

# 거리(R)에 대한 외부 루프
for dist in "${DISTANCES[@]}"; do
  # 각도(theta)에 대한 내부 루프
  for angle in "${ANGLES[@]}"; do
    
    echo ""
    echo "--- Running for Distance = ${dist} cm, Angle = ${angle} deg ---"

    # 1. 현재 설정에 맞는 결과 파일 이름 생성
    output_filename="output_R${dist}_A${angle}"
    
    # 2. 임시 매크로 파일 생성
    #    sed 명령어를 사용하여 템플릿의 플레이스홀더를 실제 값으로 치환
    sed -e "s/DISTANCE_PLACEHOLDER/${dist}/" \
        -e "s/ANGLE_PLACEHOLDER/${angle}/" \
        -e "s/FILENAME_PLACEHOLDER/${output_filename}/" \
        "${TEMPLATE_MAC}" > "${BUILD_DIR}/tmp_run.mac"

    # 3. Geant4 시뮬레이션 실행
    #    build 디렉토리로 이동하여, 생성된 임시 매크로를 인자로 실행
    (cd "${BUILD_DIR}" && ./CPNR_OMEG_colab_low_energy_optical tmp_run.mac)
    
    echo "--- Finished: ${output_filename}.root created ---"

  done
done

# 4. 임시 매크로 파일 삭제
rm "${BUILD_DIR}/tmp_run.mac"

echo ""
echo "===== All simulations completed. ====="
```

-----

## 5\. 데이터 분석

Python 스크립트를 사용하여 `.root` 파일에 저장된 결과를 분석하고 시각화합니다.

```bash
# 1. build 디렉토리로 이동
cd build

# 2. 필요한 Python 라이브러리 설치 (최초 1회)
pip install uproot pandas matplotlib seaborn

# 3. 분석 스크립트 실행
python3 plot_coincidence.py
```

실행이 완료되면 `angular_correlation_plot.png` 파일이 생성됩니다.

-----

## 6\. 주요 학습 내용 및 명령어

  * **Geant4 v11 방사성 붕괴:** 반감기 1년 이상 핵종(Co-60 등)은 붕괴가 기본적으로 비활성화됩니다. `/run/initialize` 이후 `/process/had/rdm/thresholdForVeryLongDecayTime 1.0e+60 year` 명령어로 붕괴를 활성화해야 합니다.
  * **사용자 정의 명령어:** `G4UImessenger` 클래스를 구현하여, `/myApp/detector/setMovableAngle`과 같이 사용자가 직접 매크로 명령어를 만들고 C++ 코드의 변수를 제어할 수 있습니다.

<!-- end list -->
