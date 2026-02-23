# 🐳 Docker 환경 가이드 (조교용)

이 문서는 수업용 Docker 이미지를 **빌드하고 배포**하기 위한 조교용 가이드입니다.

---

## Docker 이미지 정보

| 항목 | 값 |
|---|---|
| Docker Hub 이미지 | `chwoong/cpp-dev:13` |
| 베이스 이미지 | `ubuntu:24.04` |
| 주요 도구 | gcc-13, g++-13, gdb, cmake, git |

> [!IMPORTANT]
> 현재 `build.sh`에는 이미지 태그가 `cpp-dev:13`(로컬 태그)으로 되어 있습니다.
> Docker Hub에 push할 때는 `chwoong/cpp-dev:13`으로 태그를 변경하여 올린 상태이니 참고해 주세요.

---

## Dockerfile 설명

```dockerfile
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /workspace

RUN apt-get update && apt-get install -y \
    gcc-13 \
    g++-13 \
    gdb \
    cmake \
    git \
    build-essential \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100 \
    && rm -rf /var/lib/apt/lists/*

CMD ["/bin/bash"]
```

### 주요 구성 요소

| 항목 | 설명 |
|---|---|
| `FROM ubuntu:24.04` | Ubuntu 24.04 LTS를 베이스 이미지로 사용 |
| `ENV DEBIAN_FRONTEND=noninteractive` | apt 설치 시 대화형 프롬프트를 비활성화 (자동 빌드를 위해 필수) |
| `WORKDIR /workspace` | 컨테이너 내 기본 작업 디렉토리를 `/workspace`로 설정 |
| `gcc-13`, `g++-13` | C/C++ 컴파일러 (GCC 13 버전) |
| `gdb` | 디버거 |
| `cmake` | 빌드 시스템 |
| `git` | 버전 관리 |
| `build-essential` | 빌드에 필요한 기본 패키지 모음 |
| `update-alternatives` | `gcc`, `g++` 명령어가 13 버전을 가리키도록 설정 |
| `rm -rf /var/lib/apt/lists/*` | apt 캐시를 삭제하여 이미지 크기 최소화 |
| `CMD ["/bin/bash"]` | 컨테이너 실행 시 bash 셸로 진입 |

---

## build.sh 설명

```bash
docker build --platform linux/amd64 -t cpp-dev:13 .
```

| 옵션 | 설명 |
|---|---|
| `--platform linux/amd64` | **필수!** Linux x86_64 아키텍처로 빌드 (아래 주의사항 참고) |
| `-t cpp-dev:13` | 빌드된 이미지에 `cpp-dev:13` 태그 부여 |
| `.` | 현재 디렉토리의 `Dockerfile`을 사용하여 빌드 |

### 실행 방법

```bash
cd docker
chmod +x build.sh
./build.sh
```

---

## ⚠️ `--platform linux/amd64` 관련 주의사항

> [!CAUTION]
> **반드시 `--platform linux/amd64` 옵션을 사용해야 합니다!**
>
> - macOS (Apple Silicon, M1/M2/M3 등)에서 빌드하면 기본적으로 `arm64` 이미지가 생성됩니다.
> - 이 수업의 Docker 이미지는 **Linux x86_64 환경에서 동작**해야 하므로, 빌드와 실행 시 항상 `--platform linux/amd64`를 명시해야 합니다.
> - 이 옵션을 빠뜨리면 아키텍처 불일치로 인해 **예기치 않은 오류**가 발생할 수 있습니다.

---

## Docker Hub에 Push하기

로컬에서 빌드한 이미지를 Docker Hub에 올리려면:

```bash
# 1. 로컬 이미지에 Docker Hub 태그 추가
docker tag cpp-dev:13 chwoong/cpp-dev:13

# 2. Docker Hub에 push
docker push chwoong/cpp-dev:13
```

학생들은 `docker pull chwoong/cpp-dev:13` 명령으로 이미지를 받아 사용할 수 있습니다.

---

## 🔨 main.cpp 컴파일 및 실행

Docker 컨테이너에 진입한 후, 아래 과정을 통해 `main.cpp`를 컴파일하고 실행할 수 있습니다.

### 1. 컨테이너 진입

프로젝트 루트 디렉토리에서 `run.sh`를 실행합니다:

```bash
./docker/run.sh
```

컨테이너 내부의 `/workspace` 디렉토리로 진입되며, 호스트의 프로젝트 파일들이 마운트되어 있습니다.

### 2. 컴파일

```bash
g++ -static -o main main.cpp
```

| 옵션 | 설명 |
|---|---|
| `g++` | C++ 컴파일러 (컨테이너 내 g++-13이 기본으로 설정됨) |
| `-static` | **정적 링크** — 모든 라이브러리를 실행 파일에 포함시켜 다른 환경에서도 실행 가능 |
| `-o main` | 출력 실행 파일 이름을 `main`으로 지정 |
| `main.cpp` | 컴파일할 소스 파일 |

### 3. 실행

```bash
./main
```

실행하면 농장 시뮬레이션 게임이 시작됩니다. `W/A/S/D`로 이동하고, `Q`로 종료할 수 있습니다.

### 전체 흐름 요약

```bash
# 호스트에서
./docker/run.sh

# 컨테이너 내부에서
g++ -static -o main main.cpp
./main
```
