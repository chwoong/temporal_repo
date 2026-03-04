# Farm Simulator — GUI 빌드 & 실행 가이드

터미널 기반 농장 시뮬레이터를 **SFML 2D 그래픽 창**으로 실행하는 방법을 설명합니다.

---

## 목차

1. [프로젝트 구조](#프로젝트-구조)
2. [사전 요구사항](#사전-요구사항)
3. [빌드 방법](#빌드-방법)
4. [실행 방법](#실행-방법)
5. [Docker로 실행](#docker로-실행)
6. [스프라이트 추가](#스프라이트-추가)
7. [조작법](#조작법)
8. [트러블슈팅](#트러블슈팅)

---

## 프로젝트 구조

```
temporal_repo/
├── CMakeLists.txt           # 빌드 설정
├── main.cpp                 # 진입점 (9줄)
├── src/
│   ├── game/
│   │   ├── GameLogic.hpp    # 게임 로직 (Player, Crop, Livestock, Market 등)
│   │   ├── GameState.hpp    # 게임 상태 enum
│   │   ├── Game.hpp         # Game 클래스 선언
│   │   └── Game.cpp         # SFML 루프 + 상태 머신 구현
│   └── render/
│       ├── TextureManager.hpp  # 텍스처 로드 / 컬러 폴백
│       └── Renderer.hpp        # 타일·엔티티·UI·다이얼로그 렌더링
├── assets/
│   └── tiles/
│       ├── README.md        # 스프라이트 파일 안내
│       ├── font.ttf         # (선택) 한국어 지원 폰트
│       └── *.png            # (선택) 타일 스프라이트
└── docker/
    ├── Dockerfile
    ├── build.sh
    └── run.sh
```

---

## 사전 요구사항

### 로컬 환경 (Ubuntu / Debian 계열)

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential cmake g++-13 libsfml-dev
```

### macOS

```bash
brew install cmake sfml
```

### Windows

[SFML 공식 다운로드](https://www.sfml-dev.org/download.php)에서 MinGW 또는 MSVC용 패키지를 받아 CMake에 경로를 지정하세요.

---

## 빌드 방법

```bash
# 1. 빌드 디렉토리 생성 및 이동
mkdir -p build && cd build

# 2. CMake 구성 (Release 권장)
cmake .. -DCMAKE_BUILD_TYPE=Release

# 3. 컴파일 (모든 코어 사용)
make -j$(nproc)
```

빌드 성공 시 `build/` 안에 실행 파일 `farm_sim`이 생성됩니다.
assets 폴더도 자동으로 `build/assets/`로 복사됩니다.

---

## 실행 방법

```bash
# build/ 디렉토리 안에서 실행
./farm_sim
```

> **PNG 파일이 없어도** 컬러 블록으로 바로 실행됩니다.
> 폰트(`assets/tiles/font.ttf`)가 없으면 텍스트는 표시되지 않지만 게임 자체는 동작합니다.

---

## Docker로 실행

### 1. Docker 이미지 빌드 (최초 1회)

```bash
cd docker
bash build.sh
```

### 2. X11 포워딩 허용 (호스트에서)

```bash
# Linux
xhost +local:docker

# macOS — XQuartz 설치 후
xhost +localhost
```

### 3. 컨테이너 실행

```bash
# 프로젝트 루트에서
cd docker
bash run.sh
```

### 4. 컨테이너 안에서 빌드 & 실행

```bash
cd /workspace
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./farm_sim
```

---

## 스프라이트 추가

PNG 파일 없이도 컬러 블록으로 플레이 가능하지만,
`assets/tiles/` 에 아래 파일을 넣으면 스프라이트가 적용됩니다.

| 파일명             | 설명                      | 권장 크기 |
|-------------------|--------------------------|---------|
| `ground.png`      | 바닥 타일                  | 64×64   |
| `wall.png`        | 벽 타일                    | 64×64   |
| `player.png`      | 플레이어                   | 64×64   |
| `home.png`        | 집 (H 타일)                | 64×64   |
| `market.png`      | 시장 (M 타일)               | 64×64   |
| `mayor.png`       | 촌장 (G 타일)               | 64×64   |
| `crop_stage0.png` | 감자 — 씨앗 (성장도 0)      | 64×64   |
| `crop_stage1.png` | 감자 — 새싹 (성장도 1-2)    | 64×64   |
| `crop_stage2.png` | 감자 — 수확 가능            | 64×64   |
| `livestock.png`   | 닭                         | 64×64   |
| `font.ttf`        | 한국어 지원 폰트 (필수)      | —       |

### 무료 폰트 설치 예시

```bash
# NanumGothic 설치 후 복사
sudo apt-get install fonts-nanum
cp /usr/share/fonts/truetype/nanum/NanumGothic.ttf assets/tiles/font.ttf
```

---

## 조작법

| 키          | 동작             |
|------------|-----------------|
| `W / A / S / D` | 이동          |
| `H` 타일 이동  | 수면 확인 다이얼로그 |
| `Y`         | 수면 확인 (다음 날) |
| `N`         | 수면 취소         |
| `M` 타일 이동  | 시장 메뉴 열기    |
| `1`         | 모두 팔기         |
| `2`         | 구매 메뉴         |
| `0`         | 취소             |
| `G` 타일 이동  | 주사위 미니게임   |
| `C / L` 타일 이동 | 작물/닭 상호작용 |
| `Q`         | 게임 종료         |

### 게임 흐름

```
이동 → 엔티티 상호작용(C/L) → 시장(M)에서 판매·구매
  → 촌장(G)과 도박 → 집(H)에서 수면 → 다음 날 성장 + 재해 체크
```

---

## 트러블슈팅

| 증상 | 원인 | 해결 |
|-----|------|------|
| `SFML not found` CMake 오류 | libsfml-dev 미설치 | `sudo apt install libsfml-dev` |
| 창은 뜨는데 텍스트가 안 보임 | font.ttf 없음 | `assets/tiles/font.ttf` 추가 |
| Docker에서 창이 안 열림 | X11 포워딩 미설정 | `xhost +local:docker` 실행 후 재시도 |
| 컬러 블록만 표시됨 | PNG 파일 없음 | `assets/tiles/`에 PNG 추가 (없어도 정상 동작) |
| `Error: cannot connect to X server` | DISPLAY 환경변수 미설정 | `echo $DISPLAY` 확인, `run.sh`의 `-e DISPLAY` 옵션 확인 |
