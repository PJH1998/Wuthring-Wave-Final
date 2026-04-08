## Model Double Buffering 성능 측정 계획서

### 1. 목표

- **측정 대상**: `Engine`의 `CModel`에서 GPU→CPU 본 행렬 리드백을 위한 **더블 버퍼링(Readback_BoneMatrices)**.
- **비교 조건**:
  - **ON**: 더블 버퍼링(스테이징 버퍼 2개 ping-pong) 사용.
  - **OFF**: 단일 스테이징 버퍼(또는 동일 버퍼를 즉시 Map) 사용.
- **결과 지표**:
  - 리드백 구간 시간(ms) 차이와 **퍼센트 개선율**.
  - 프레임 전체 시간(ms) / FPS 차이와 **퍼센트 개선율**.

> 주의: 이 문서는 **작업 계획만 정리**하며, 실제 코드 수정은 별도 단계에서 수행한다.

---

### 1.1. 플레이 중 조작 방식 (요구사항)

아래는 **게임 플레이 도중**에만으로도 측정·비교를 끝낼 수 있게 하기 위한 동작 정의이다. (구체 키 코드는 구현 시 `CGameInstance`/`Input_Device`와 충돌 없는 키로 확정)

1. **녹화(프레임 샘플링) 구간**
   - **시작 키**를 한 번 누르면: 그 시점부터 프레임별로 리드백 시간·프레임 시간·당시 더블 버퍼링 모드 등을 **버퍼에 기록**하기 시작한다.
   - **종료 키**를 한 번 누르면: 기록을 **즉시 중단**하고, **누적된 데이터를 `.csv` 파일로 저장**한다. (파일명은 타임스탬프 포함 권장, 예: `BoneReadback_YYYYMMDD_HHMMSS.csv`)
   - 시작/종료에 **서로 다른 키**를 쓸지, **같은 키로 토글**할지는 구현 시 선택 가능하나, 문서상 기본 요구는 **「시작 → 플레이 → 종료 → CSV 산출」** 한 흐름이다.

2. **더블 버퍼링 ON/OFF**
   - **플레이 중**에도 토글 가능해야 한다. (예: 전용 핫키 한 개, 또는 ImGui와 병행)
   - 토글 시점이 **프레임 경계**에 적용되도록 하는 것이 안전하다(중간 프레임에서 버퍼 의미가 꼬이지 않도록). 구현 시 `CModel`의 `Readback_BoneMatrices` 진입 전 또는 `BeginFrame` 직후에만 반영하는 식으로 설계한다.

3. **ENGINE_DLL과의 역할 분리**
   - **입력 판정(키 다운/업)**은 `Client`의 `MainApp` 등에서 `CGameInstance`의 입력 API로 처리하고,
   - **버퍼에 쌓기·CSV 직렬화·파일 쓰기**는 `Engine` DLL에 노출된 API로 위임하는 형태를 권장한다. (MainApp은 “키를 눌렀다”는 이벤트만 DLL에 전달)

---

### 2. 노출 설계 (ENGINE_DLL / MainApp 연동)

1. **Engine DLL 측정 기능 설계**
   - `Engine` 프로젝트 안에 **프로파일러 유틸리티 모듈** 설계:
     - 역할: 프레임 단위로
       - `Readback_BoneMatrices` 호출에서 측정한 리드백 시간(ms) 누적.
       - 프레임 전체 시간(ms) 측정.
       - 더블 버퍼링 ON/OFF 상태별로 통계 집계.
       - ImGui / CSV로 숫자 출력.
   - 이 모듈의 API는 `ENGINE_DLL`로 내보낼 수 있는 최소 인터페이스로 정의:
     - 예시:
       - `ENGINE_DLL void BeginFrame();`
       - `ENGINE_DLL void EndFrame();`
       - `ENGINE_DLL void AddReadbackMs(double ms);`
       - `ENGINE_DLL void DrawImGui();` (선택: 디버그/옵션 표시용)
       - `ENGINE_DLL void BeginRecordingSession();` — 녹화 시작(버퍼 초기화 후 프레임 기록)
       - `ENGINE_DLL void EndRecordingSessionAndExportCsv(const char* pathOrNull);` — 녹화 종료 + **CSV 파일 생성**. `pathOrNull == nullptr`이면 기본 경로(실행 파일/작업 폴더 규칙) 사용.
       - `ENGINE_DLL void SetUseDoubleBuffering(bool);`
       - `ENGINE_DLL bool GetUseDoubleBuffering();`
   - 실제 구현 파일/클래스 이름은 이후 단계에서 결정 (예: `CBoneReadbackProfiler` 또는 네임스페이스 기반 정적 함수).

2. **엔진 내부와의 연결**
   - `CModel::Readback_BoneMatrices()` 내부에 **측정 지점**을 둘 계획:
     - Map~Unmap + memcpy + 본 행렬 적용 전체를 하나의 구간으로 측정.
     - 측정한 시간을 위의 `AddReadbackMs()`로 전달.
   - 더블 버퍼링 ON/OFF 스위치는:
     - `CModel`의 `static` 플래그 또는 엔진 전역 설정에 두고,
     - 공개 Getter/Setter를 `ENGINE_DLL`로 노출.

3. **MainApp에서의 사용**
   - `Client`의 `MainApp` / `EditorApp`에서:
     - 한 프레임 시작 시 `BeginFrame()` 호출.
     - 렌더 이후 프레임 종료 시 `EndFrame()` 호출.
     - **핫키 처리** (예: `Update` 초반 또는 입력 처리 후):
       - 녹화 시작 키 → `BeginRecordingSession()`.
       - 녹화 종료 키 → `EndRecordingSessionAndExportCsv(...)` (또는 토글 1키면 `IsRecording()`에 따라 분기).
       - 더블 버퍼링 토글 키 → `SetUseDoubleBuffering(!GetUseDoubleBuffering())` (또는 명시 ON/OFF).
     - ImGui가 필요하면 업데이트 시점에 `DrawImGui()` 호출로 디버그 윈도우 출력 (핫키와 동일한 API를 호출해도 됨).
   - MainApp 쪽에서는 오직 **ENGINE_DLL로 노출된 함수만 호출**하고,
     엔진 내부 구현(모델/버퍼 구조)에는 직접 접근하지 않는 형태로 유지.

---

### 3. 측정 지표 및 계산 방식

#### 3.1. 리드백 구간 시간

- **정의**: 한 프레임 내에서 `Readback_BoneMatrices()`가 사용하는 총 시간.
- 방법:
  - 함수 진입 시점 / 종료 시점 기준으로 **고해상도 타이머** 사용(Win32 QPC 또는 `std::chrono`).
  - 한 프레임 동안 이 함수가 여러 번 호출될 수 있으므로:
    - 프레임별 누적 시간 = `Σ(각 호출 시간)`.
  - `EndFrame()`에서 한 프레임의 누적 값을 통계 구조에 기록.

- **지표**:
  - 더블 버퍼링 ON / OFF 각각에 대해:
    - 평균 리드백 시간 (ms).
    - 최소/최대, 표본 개수.

#### 3.2. 프레임 전체 시간 / FPS

- 프레임 루프 레벨에서:
  - `BeginFrame()`에서 프레임 시작 시각 저장.
  - `EndFrame()`에서 프레임 종료 시각과의 차이 → 프레임 전체 시간(ms).
- **측정 값**:
  - 더블 버퍼링 ON/OFF 각각에 대해:
    - 평균 Frame Time (ms).
    - 평균 FPS (1 / 평균 Frame Time).

#### 3.3. 퍼센트 개선율

- 리드백 시간 기준:
  - \[
      \text{개선율}_{\text{readback}} =
      \frac{T_{\text{OFF}} - T_{\text{ON}}}{T_{\text{OFF}}} \times 100 (\%)
    \]
- 프레임 전체 시간 기준:
  - \[
      \text{개선율}_{\text{frame}} =
      \frac{F_{\text{OFF}} - F_{\text{ON}}}{F_{\text{OFF}}} \times 100 (\%)
    \]
- ON/OFF 둘 다 충분한 표본 수(수천 프레임 이상)를 모은 뒤 계산.

---

### 4. 실험 시나리오 설계

1. **환경 고정**
   - 동일 PC, 동일 GPU, 동일 해상도(예: 1920×1080).
   - 동일 그래픽 옵션, 동일 레벨/씬.
   - Release 빌드 + 최적화 옵션 고정.

2. **씬 유형**
   - **일반 씬**:
     - 실제 게임 플레이에서 자주 나오는 상황 (플레이어 + 소수의 몬스터).
   - **스트레스 씬**:
     - `CModel` 인스턴스와 GPU 애니메이션이 많이 도는 상황.
     - 예: 캐릭터, 몬스터, 이펙트가 대량으로 등장하는 전투 장면.

3. **수집 절차**
   - **핫키 기반(권장)**: 한 CSV 세션 안에서 플레이하며 필요 시 더블 버퍼링만 토글한다. 종료 키로 CSV를 저장하면, 파일 안에 ON/OFF 구간이 **시간 순서대로** 섞여 기록되므로 후처리에서 구간 필터링·평균 비교가 가능하다.
   - **블록 비교(대안)**: 각 씬마다
     1. 더블 버퍼링 **ON**으로 두고 녹화 시작→종료로 전용 CSV 저장.
     2. 더블 버퍼링 **OFF**로 두고 동일 씬에서 다시 녹화 시작→종료.
     3. 가능하면 ON/OFF 순서를 바꿔 2~3회 반복(조건에 따른 변동성 줄이기).

4. **데이터 저장**
   - 프레임마다:
     - 프레임 인덱스.
     - 모드(`DB_ON` / `DB_OFF`).
     - 리드백 누적 시간(ms).
     - 프레임 전체 시간(ms).
   - CSV 포맷 예:
     - `frame,mode,readback_ms,frame_ms`

---

### 5. 핫키 / ImGui / CSV 설계

#### 5.1. 핫키(플레이 중) — **주요 조작 경로**

- **녹화 시작**: 한 번 누르면 `BeginRecordingSession()`에 해당.
- **녹화 종료 + CSV 저장**: 한 번 누르면 `EndRecordingSessionAndExportCsv(...)`에 해당.  
  - 종료 시점에 디스크에 **한 개의 CSV**가 생성된다.
  - 같은 세션 안에서 ON/OFF를 바꿔 가며 플레이해도, **각 행에 `mode` 컬럼**으로 남기면 나중에 구간별로 필터링 가능.
- **더블 버퍼링 토글**: 전용 키로 `SetUseDoubleBuffering` 호출.  
  - (선택) 화면에 HUD 텍스트로 현재 ON/OFF만 잠깐 표시.

#### 5.2. ImGui (선택)

- 이름 예시: `"Bone Readback (Model Double Buffering)"`.
- 표시 내용:
  - 현재 모드: 더블 버퍼링 ON/OFF (체크박스 — 핫키와 동일 API).
  - **녹화 중 여부** 표시 (빨간 점 등).
  - 최근 프레임:
    - `Last frame: readback X.XXX ms | frame Y.YYY ms`.
  - 누적 통계(세션 전체 또는 세션 밖 누적 통계 — 구현 시 구분):
    - `Samples ON: N_on | OFF: N_off`.
    - `Avg readback: ON a_on ms | OFF a_off ms`.
    - `Avg frame: ON b_on ms | OFF b_off ms`.
    - 위 두 값에서 계산한 **퍼센트 개선율**.
  - 제어:
    - 통계 초기화 버튼.
    - (선택) 수동 Export 경로 입력 — **핫키 종료 시 자동 저장이 기본**이면, ImGui는 보조용.

#### 5.3. CSV 내보내기 (동작)

- **핫키로 녹화 종료**할 때 자동으로 CSV를 쓴다.
- 기록은 **Engine DLL**에서 버퍼에 누적하고, 종료 시 한 번에 flush.
- CSV 포맷은 §4와 동일하게 유지하되, 필요 시 `session_id` 또는 파일명 타임스탬프로 구분.

---

### 6. 구현 단계 요약 (향후 코드 작업용)

> 아래는 **향후 작업 순서**를 위한 메모이며, 이 문서 단계에서는 코드 변경을 하지 않는다.

1. **Engine 쪽**
   - [ ] 프로파일러용 헤더/CPP 추가, `ENGINE_DLL` API 정의.
   - [ ] `CModel::Readback_BoneMatrices()`에 시간 측정 및 `AddReadbackMs()` 호출 추가.
   - [ ] 더블 버퍼링 ON/OFF 스위치를 전역적으로 제어할 수 있는 Getter/Setter 구현.

2. **Client / Editor 쪽**
   - [ ] `MainApp` / `EditorApp`에서:
     - 프레임 루프에 `BeginFrame()` / `EndFrame()` 삽입.
     - **핫키**: 녹화 시작 / 녹화 종료+CSV / 더블 버퍼링 토글을 입력 처리에서 DLL API로 연결.
     - (선택) ImGui 업데이트 타이밍에 `DrawImGui()` 호출.

3. **실험 & 분석**
   - [ ] ON/OFF 각각 충분한 프레임 수 수집.
   - [ ] Export된 CSV를 외부 도구(Excel, Python 등)로 분석.
   - [ ] 문서(예: 이 파일에 결과 섹션 추가)로 최종 수치 기록.

---

### 7. 주의사항

- Release 빌드 기준으로 측정을 우선하고, Debug는 **디버깅용 참고치**로만 사용.
- 가능하면 **측정 코드 자체의 오버헤드**를 최소화:
  - 리드백 구간 안에서는 간단한 시간 차 계산과 누적만 수행.
  - 통계/퍼센트 계산은 프레임 끝이나 ImGui 렌더 시에만 수행.
- 더블 버퍼링 OFF 모드에서는 GPU 스톨이 증가할 수 있으므로,
  테스트 중 프레임 드랍이 심할 수 있음을 인지하고 진행.

