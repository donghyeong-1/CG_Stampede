# 진행상황 (PROGRESS)

> Claude / Codex / Gemini 공용 인수인계 문서. 규칙은 `AGENTS.md`의 "에이전트 협업 규칙" 참고.

## 현재 상태

- 단계: 1단계 진행 중. 프로젝트 뼈대 완성(창·카메라·피킹 강조·HUD·상태 전환 동작), 점프·능력·충돌은 TODO
- 마지막 갱신: 2026-11-12 (Claude)

## 진행 중

| 에이전트 | 작업 | 건드리는 파일 | 시작일 |
|---|---|---|---|
| - | - | - | - |

## 다음 할 일

- [x] 제안서 내용 기반으로 기술 스택 및 구조 확정 (plan.md)
- [x] 프로젝트 뼈대 생성
- [x] 실행 방법을 `AGENTS.md`에 기록
- [ ] 팀 합의: `src/core/GameState.h` 필드, `src/core/Config.h` 수치 (plan.md 11장)
- [ ] (김동형) `Game::TryJump` 점프·탑승 판정, `Game::TryUseAbility` 능력·충돌·피해 1회, 체크포인트 복귀 — `src/logic/`에 모듈로 분리 권장
- [ ] (장수현) 글자 출력, 실제 바닥 텍스처, 모델·모션 다듬기, 장애물(균열) 표현
- [ ] 다른 PC 실행용 VC++ 재배포 패키지 대응 (glfw3.lib가 /MD)

## 결정 사항

| 날짜 | 결정 | 이유 |
|---|---|---|
| 2026-11-11 | `AGENTS.md`를 공통 규칙 원본으로, `docs/PROGRESS.md`를 공유 진행 문서로 사용 | 세 에이전트가 동일한 컨텍스트로 작업하기 위함 |
| 2026-11-12 | 빌드는 CMake가 아니라 수업 실습 틀의 Visual Studio 솔루션(`Stampede.sln`) + GLEW/GLFW/GLM 동봉(`third_party/`) 사용 | 사용자 요청: 실습 14~24 틀 활용. 두 팀원이 수업과 같은 환경에서 바로 빌드 |
| 2026-11-12 | 런타임은 `/MD`(정적 `/MT` 불가) | 동봉 glfw3.lib가 /MD로 빌드됨. 다른 PC에는 VC++ 재배포 패키지 필요할 수 있음 |
| 2026-11-12 | 화면 오른쪽 = 월드 -X (D 조향은 -X) | 진행 방향 +Z를 보는 카메라에서는 오른쪽이 -X. `cfg::STEER_RIGHT_SIGN` |
| 2026-11-12 | 피킹은 프레임버퍼 픽셀 기준(창 좌표를 비율로 변환) | 창 크기·DPI가 달라도 조준이 맞도록 |

## 작업 로그 (최신순)

### 2026-11-12 · Claude
- 한 일: 실습 14~24(특히 공통틀·19·21·23) 틀을 활용해 프로젝트 뼈대 제작. Debug/Release x64 MSBuild 경고 없이 빌드, 실행하여 셰이더 컴파일 오류 없음·Enter로 Title→Playing 전환·장면(바닥 텍스처, 동물 6마리, 원숭이, 균열·바위, HUD) 렌더 확인(스크린샷).
- 변경 파일: `Stampede.sln`, `Stampede.vcxproj`, `StampedeSettings.props`, `.gitignore`, `src/**`, `shaders/*`, `third_party/**`(실습3-9 동봉본 복사, 약 7MB), `plan.md`, `AGENTS.md`, `docs/PROGRESS.md`
- 파일 소유(권장): `render/ model/ ui/`=장수현, `logic/`=김동형, `core/`=공동(변경 시 여기에 기록)
- 다음 에이전트 참고: 점프·능력·충돌은 `Game.cpp`의 TODO 두 곳이 진입점이다. 미검증: 마우스 클릭 버튼·창 크기 변경·피킹 정확도(수동 확인 필요, 자동 검증은 Enter 전환과 렌더 스크린샷만 수행). 커밋·푸시는 하지 않았다.

### 2026-11-11 · Claude
- 한 일: 제안서 docx를 기준으로 구현 계획 `plan.md` 작성 (기술 스택·구조·기능별 설계·일정·테스트·위험)
- 변경 파일: `plan.md`, `docs/PROGRESS.md`
- 다음 에이전트 참고: `plan.md`의 **[제안]** 표시 항목(라이브러리, 디렉터리 구조, GameState 필드 등)은 제안서에 없는 제안이므로 팀 확정 전까지 임의로 굳히지 말 것. 확정 항목은 `plan.md` 11장 참고.

### 2026-11-11 · Claude
- 한 일: 멀티 에이전트 협업 환경 세팅 (`AGENTS.md`, `CLAUDE.md`, `GEMINI.md`, `docs/PROGRESS.md`)
- 다음 에이전트 참고: 아직 코드가 없으므로 제안서(`컴퓨터그래픽스_최종프로젝트_제안서.docx`)를 먼저 읽을 것
