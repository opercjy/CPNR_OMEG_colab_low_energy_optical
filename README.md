# CPNR-OMEG_colab_low_energy_optical: Geant4 기반 광학 시뮬레이션

## 1. 프로젝트 개요 (Overview)

본 프로젝트는 ⁶⁰Co 선원을 이용한 검출 시스템의 **저에너지 광학 물리 현상**을 정밀하게 전산모사하기 위한 Geant4 시뮬레이션입니다. 실제 실험 환경과 유사하게 모델링된 검출기 유닛(액체섬광체, 듀란 병, PMT) 2개를 배치하여, **입자의 각도 상관관계(Angular Dependence)**를 측정하는 것을 주요 목표로 합니다.

Geant4의 고급 기능을 적극 활용하여 다음과 같은 상세한 데이터 출력이 가능합니다.
* **정밀 이벤트 데이터:** `SensitiveDetector`를 이용해 각 검출기 부품에서 발생하는 모든 물리적 상호작용(Hit) 정보를 TTree 형태로 저장합니다.
* **복셀화 선량 데이터:** `ScoringMesh`를 이용해 액체섬광체(LS) 내부의 3차원 선량 분포를 CSV 파일로 저장합니다.
* **광자 계수 데이터:** 각 PMT에 도달한 광학 광자의 수와 시간 정보를 TTree에 기록하여 동시 계수(Coincidence) 분석을 지원합니다.


---
## 2. 시스템 요구사항 (Prerequisites)

본 시뮬레이션을 실행하기 위해 **호스트 머신(사용자 PC)**에 다음 소프트웨어가 설치되어 있어야 합니다.

* **Docker:** 격리된 리눅스 환경을 제공하여 라이브러리 충돌 문제를 원천적으로 방지합니다.
    * [Docker 공식 홈페이지 (설치 안내)](https://www.docker.com/get-started)
* **X11 서버 (GUI 모드용):** Docker 컨테이너의 그래픽 출력을 사용자 화면에 표시하기 위해 필요합니다.
    * **Windows:** [VcXsrv](https://sourceforge.net/projects/vcxsrv/) 또는 [Xming](https://sourceforge.net/projects/xming/) 설치를 권장합니다.
    * **macOS:** [XQuartz](https://www.xquartz.org/) 설치가 필요합니다.
    * **Linux:** 대부분의 데스크탑 배포판에 기본적으로 포함되어 있습니다.

---
## 3. 환경 구축: Docker (최초 1회 설정)

이 과정은 Geant4와 ROOT가 완벽하게 설치된 Docker 이미지를 만들고, 편리한 실행 환경을 구축하는 과정입니다. **컴퓨터에 처음 환경을 설정할 때 한 번만 수행하면 됩니다.**

### 3.1. Docker 이미지 빌드

AlmaLinux 9 컨테이너 위에서 Geant4와 ROOT를 소스 코드로 직접 컴파일하여 우리 프로젝트만을 위한 맞춤형 OS 이미지를 만듭니다.

**1) AlmaLinux 9 컨테이너 시작**
```bash
# 이 프로젝트의 최상위 디렉토리에서 아래 명령어를 실행하여 빌드용 컨테이너를 시작합니다.
docker run -it --name g4builder -v "$(pwd)":/work -w /work almalinux:9 bash
````

**2) 컨테이너 내부에서 필수 패키지 설치**
컨테이너 터미널(`[root@... work]#`) 안에서 아래 명령어를 실행하여 모든 개발 도구와 라이브러리를 설치합니다.

```bash
# 시스템 업데이트 및 기본 개발 도구 설치
dnf update -y && dnf groupinstall "Development Tools" -y && dnf install -y epel-release && dnf config-manager --set-enabled crb

# 그래픽 빌드에 필요한 기본 라이브러리 설치 
dnf install -y cmake expat-devel xerces-c-devel libX11-devel libXext-devel libXmu-devel libXpm-devel libXft-devel mesa-libGL-devel mesa-libGLU-devel qt6-qtbase-devel 'qt6-*-devel' python3-devel openssl-devel python3-pip
```

**3) ROOT 소스 코드 빌드 및 설치**

```bash
cd /usr/local/src
# 최신 버전은 ROOT 웹사이트에서 확인: [https://root.cern/install/](https://root.cern/install/)
wget [https://root.cern/download/root_v6.36.04.source.tar.gz](https://root.cern/download/root_v6.36.04.source.tar.gz)
tar -xzvf root_v6.36.04.source.tar.gz
mkdir root_build && cd root_build
cmake ../root-6.36.04 -DCMAKE_INSTALL_PREFIX=/usr/local/root -Dall=ON
make -j$(nproc) && make install
```

**4) Geant4 소스 코드 빌드 및 설치**

```bash
cd /usr/local/src
# 최신 버전은 Geant4 웹사이트에서 확인: [https://geant4.web.cern.ch/download](https://geant4.web.cern.ch/download)
wget [https://geant4-data.web.cern.ch/geant4-data/releases/geant4-v11.3.2.tar.gz](https://geant4-data.web.cern.ch/geant4-data/releases/geant4-v11.3.2.tar.gz)
tar -xzvf geant4-v11.3.2.tar.gz
mkdir geant4_build && cd geant4_build
source /usr/local/root/bin/thisroot.sh # ROOT 환경을 먼저 활성화
cmake ../geant4-v11.3.2 -DCMAKE_INSTALL_PREFIX=/usr/local/geant4 -DGEANT4_BUILD_MULTITHREADED=ON -DGEANT4_USE_QT_QT6=ON -DGEANT4_USE_ROOT=ON -DGEANT4_INSTALL_DATA=ON
make -j$(nproc) && make install
```

**5) 완성된 환경을 새 이미지로 저장 (매우 중요)**
모든 설치가 완료되면, `exit`으로 컨테이너를 종료하면 안됩니다. 이 경우 그 내용들은 저장되지 않습니다. 따라서 컨테이너 (커널 수준에서 프로그램 설치)를 저장 하고 싶다면, 반드시 호스트 새 터미널 창에서 이미지화 (commit) 해야 합니다. 
즉 다음과 같이 **호스트 머신의 새 터미널 창**에서 아래 명령어를 실행하여 지금까지의 모든 작업 내용을 `my-g4-optical-env:1.0`이라는 새로운 이미지로 저장합니다. ** 

```bash
docker commit g4builder my-g4-optical-env:1.0
# (선택) 임시 빌드 컨테이너 삭제
docker rm g4builder
```

이제부터는 위 과정을 반복할 필요 없이, `my-g4-optical-env:1.0` 이미지 하나로 언제든지 동일한 환경을 불러올 수 있습니다.

### 3.2. 편리한 실행 환경 설정

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

**2) 컨테이너 환경 변수 영구 설정 (선택 사항이지만 강력히 추천)**
매번 컨테이너에 접속할 때마다 `source` 명령어를 입력하는 것을 피하기 위해, **컨테이너 내부의** `~/.bashrc`에 환경 변수를 미리 등록합니다.

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
# (만약 3.2-2 단계를 수행했다면 아래 source 명령어들은 생략 가능)
# source /usr/local/geant4/bin/geant4.sh
# source /usr/local/root/bin/thisroot.sh
rm -rf build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

#### 4.2. 실행

##### GUI 모드 (시각화 및 디버깅)

`build` 디렉토리에서 인자 없이 실행한다. `init_vis_angular.mac`이 실행된다.

```bash
./CPNR_OMEG_colab_low_energy_optical
```
<img width="1320" height="1020" alt="image" src="https://github.com/user-attachments/assets/184fce5d-0825-4880-9981-9fb102ea57f1" />
<img width="1320" height="1020" alt="image" src="https://github.com/user-attachments/assets/1c476ba6-a9c7-4cf7-bee3-38e6a198d53d" />
<img width="1320" height="1020" alt="image" src="https://github.com/user-attachments/assets/5430377a-6c3b-45b3-b856-c981e3c0f188" />


##### 배치 모드 (데이터 생산 자동화)

배치 모드는 대량의 데이터를 생산하기 위한 핵심 기능이다. 이 프로젝트는 두 가지 자동화 방식을 제공하며, 각각의 목적과 장단점이 명확하다.

**방법 1: Geant4 내부 루프 활용 (강력히 권장)**

이 방식은 우리가 리팩토링한 C++ 코드의 동적 지오메트리 변경 기능을 **가장 효율적으로 활용**하는 방법이다. Geant4 프로그램을 **단 한 번만 실행**하고, 매크로 내의 `/control/loop` 명령어를 통해 여러 파라미터 조합을 초고속으로 스캔한다.

  * **장점:** 각 파라미터 변경 시 프로그램 재시작 오버헤드가 없어 **압도적으로 빠르다.** 단일 컴퓨터에서 파라미터 스캔을 수행할 때 가장 이상적인 방법이다.
  * **실행 방법:** `build` 디렉토리에서 `scan_all.mac` 파일을 인자로 전달하여 실행한다.

<!-- end list -->

```bash
# (build 디렉토리 내부에서 실행)
./CPNR_OMEG_colab_low_energy_optical../scan_all.mac
```

이 명령어 하나로 `scan_all.mac`에 정의된 모든 거리와 각도 조합에 대한 시뮬레이션이 연속적으로 수행되고, 결과는 `build/results.root` 파일 하나에 체계적으로 저장된다.

**방법 2: 외부 쉘 스크립트 활용 (고성능 클러스터 환경용)**

`run_all.sh` 스크립트는 각 파라미터 조합마다 Geant4 프로그램을 **독립적인 프로세스로 새로 실행**한다.

  * **장점:** 각 시뮬레이션이 완벽히 분리되어 있어, 대규모 컴퓨팅 클러스터 환경에서 각 잡(job)을 여러 노드에 분산시켜 **병렬 처리**하는 데 적합하다.
  * **단점:** 단일 컴퓨터에서 실행 시, 매번 프로그램을 새로 시작하는 오버헤드 때문에 매우 비효율적이다.
  * **실행 방법:**

<!-- end list -->

```bash
# 1. 스크립트에 실행 권한 부여 (최초 1회)
chmod +x run_all.sh

# 2. 프로젝트 최상위 디렉토리에서 스크립트 실행
./run_all.sh
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
python3 ../plot_coincidence.py
```

실행이 완료되면 `build` 폴더에 `angular_correlation_plot.png` 파일이 생성됩니다.

-----

## 6\. Geant4 v11 주요 학습 내용

  * **방사성 붕괴 임계값**: Geant4 v11.2부터 반감기가 **1년 이상**인 핵종(Co-60 등)은 붕괴가 기본적으로 비활성화됩니다. `/run/initialize` 이후 `/process/had/rdm/thresholdForVeryLongDecayTime 1.0e+60 year` 명령어로 붕괴를 활성화해야 합니다.
  * **사용자 정의 명령어:** `G4UImessenger` 클래스를 구현하여, `/myApp/detector/setMovableAngle`과 같이 사용자가 직접 매크로 명령어를 만들고 C++ 코드의 변수를 제어할 수 있습니다.
  * **사용자 정의 물리 리스트:** `G4VModularPhysicsList`를 상속받아 필요한 물리 모듈만 조합하면, 표준 리스트 사용 시 발생하는 UI 명령어 비활성화 등의 문제를 피하고 시뮬레이션을 완벽하게 제어할 수 있습니다.
  * **동적 지오메트리 제어:** `G4GenericMessenger`를 사용하여 C++ 코드의 변수(`fMovablePMTAngle` 등)를 매크로 명령어(`/myApp/detector/setMovableAngle`)와 직접 연결할 수 있다. `Setter` 함수 내에서 `G4RunManager::GetRunManager()->GeometryHasBeenModified()` 를 호출하는 것이 핵심이며, 이를 통해 `/run/beamOn` 명령어 사이에서 코드를 재컴파일하지 않고도 지오메트리를 동적으로 변경하고 시뮬레이션을 연속적으로 수행할 수 있다. 이는 파라미터 스캔 시뮬레이션의 효율을 극대화한다.


<!-- end list -->
