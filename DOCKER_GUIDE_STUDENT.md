# 🐳 Docker 환경 가이드 (학생용)

이 문서는 수업에서 사용할 C++ 개발 환경(Docker)을 설정하고 실행하는 방법을 안내합니다.

---

## 1. 사전 준비

1. [Docker Desktop](https://www.docker.com/products/docker-desktop/)을 설치하세요.
2. Docker Desktop이 실행 중인지 확인하세요.

---

## 2. Docker 이미지 받기

**Docker 이미지**란, 운영체제(Linux)와 설치된 프로그램(컴파일러 등)을 하나로 묶어놓은 "환경 압축 파일" 같은 것입니다. 이미지를 다운받아 실행하면 내 컴퓨터 설정에 영향을 주지 않고 독립된 개발 환경을 쓸 수 있습니다.

터미널에서 아래 명령어를 실행하여 수업용 이미지를 다운로드합니다:

```bash
docker pull chwoong/cpp-dev:13
```

> [!NOTE]
> 이 이미지에는 **gcc-13, g++-13, gdb, cmake, git** 등 수업에 필요한 모든 도구가 포함되어 있습니다.

---

## 3. run.sh 실행하여 플젝 진행

### 3.1. run.sh 설명

```bash
docker run -it --rm \
  --platform linux/amd64 \
  -v $(pwd):/workspace \
  --name cpp-dev-container \
  chwoong/cpp-dev:13
```

#### 옵션별 설명

| 옵션 | 설명 |
|---|---|
| `-it` | 인터랙티브 모드로 실행 (터미널 입출력 가능) |
| `--rm` | 컨테이너 종료 시 자동 삭제 (불필요한 컨테이너가 쌓이지 않음) |
| `--platform linux/amd64` | **필수!** Linux x86_64 환경으로 실행 (아래 주의사항 참고) |
| `-v $(pwd):/workspace` | 현재 디렉토리를 컨테이너의 `/workspace`에 마운트 (파일 공유) |
| `--name cpp-dev-container` | 컨테이너 이름을 `cpp-dev-container`로 지정 |
| `cpp-dev:13` | 사용할 이미지 이름 및 태그 |

> [!CAUTION]
> **반드시 `--platform linux/amd64` 옵션을 사용해야 합니다!**
>
> - macOS (Apple Silicon, M1/M2/M3 등) 사용자는 이 옵션이 없으면 아키텍처 불일치로 **예기치 않은 오류**가 발생할 수 있습니다.
> - 이 수업의 모든 과제와 채점은 **Linux x86_64 환경**에서 진행되므로, 항상 이 옵션을 포함하여 실행하세요.
> - `run.sh`에 이미 포함되어 있으므로, **반드시 `run.sh`를 통해 실행**하는 것을 권장합니다.

### 3.2. 실행 방법

```bash
# 1. 프로젝트 디렉토리로 이동
cd 프로젝트_디렉토리

# 2. run.sh 실행 권한 부여 (최초 1회)
chmod +x docker/run.sh

# 3. 컨테이너 실행
./docker/run.sh
```

실행하면 컨테이너 내부의 bash shell로 진입합니다. 현재 디렉토리의 파일들이 컨테이너 내 `/workspace`에 보이게 됩니다.

---

## (4. run.sh 없이 직접 실행하기)

`run.sh` 없이도 아래 명령어로 직접 실행할 수 있습니다:

```bash
docker run -it --rm \
  --platform linux/amd64 \
  -v $(pwd):/workspace \
  --name cpp-dev-container \
  chwoong/cpp-dev:13
```

> [!TIP]
> `docker pull`로 이미지를 미리 받아두면 `docker run` 시 다운로드 시간을 절약할 수 있습니다.

---

## FAQ

### Q. 컨테이너에서 나가려면?

`exit` 명령어를 입력하거나 `Ctrl+D`를 누르세요. `run.sh` 스크립트를 통해 실행할 때 `--rm` 옵션이 적용되어 있으므로, 종료 시 컨테이너가 자동으로 삭제됩니다.

### Q. "name is already in use" 오류가 발생하면?

이전에 실행한 컨테이너가 남아있는 경우입니다. 아래 명령어로 제거 후 다시 실행하세요:

```bash
docker rm cpp-dev-container
```

### Q. 파일이 컨테이너 안에서 보이지 않으면?

`run.sh`를 실행할 때 **프로젝트 디렉토리에서 실행**했는지 확인하세요. `-v $(pwd):/workspace` 옵션이 현재 디렉토리를 마운트합니다.
