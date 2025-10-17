# 플레이어 카메라 튜닝 위젯 사용법

## 개요
- `UEditorPlayerCameraTuningWidget`는 BoxPlayer와 TPSPlayer의 카메라 파라미터를 에디터에서 즉시 조정하기 위한 Editor Utility Widget입니다.
- `/Game/Editor/Utility/EUW_PlayerCameraTuning` 에셋이 생성되며, 탭에서 상태별 카메라 길이·오프셋·보간 속도를 편집할 수 있습니다.

## 준비
- C++ 변경 사항을 `HeavenlyCastle` 모듈로 빌드한 뒤 에디터를 실행합니다.
- 에디터 메뉴 `Edit > Plugins`에서 `Editor Scripting Utilities`(기본 활성화)만 있으면 추가 플러그인은 필요하지 않습니다.

## 위젯 생성 및 실행
1. 에디터 메뉴 `Tools > Execute Python Script...` 선택.
2. `HeavenlyCastle/Scripts/create_camera_tuning_widget.py` 실행.
3. 이미 에셋이 존재하면 재사용하고, 없으면 `/Game/Editor/Utility` 경로에 새 블루프린트를 생성한 뒤 탭으로 띄웁니다.
4. 이후에는 `Window > Editor Utilities > EUW_PlayerCameraTuning`에서 언제든지 탭을 다시 열 수 있습니다.

## 사용 절차
1. 레벨에서 `ABoxPlayer` 또는 `ATPSPlayer` 인스턴스를 선택합니다. 다중 선택도 가능합니다.
2. 위젯 탭에서 `선택 불러오기` 버튼을 눌러 현재 선택된 액터의 데이터를 로드합니다.
3. 각 상태(Default, Aiming, Sprint 등)의 `TargetArmLength`와 `SocketOffset(X/Y/Z)`를 조정합니다.
4. `Camera Interp Speed`로 전체 보간 속도를 조절합니다.
5. `선택된 클래스 기본값도 갱신` 체크 시, 런타임 인스턴스뿐 아니라 해당 클래식 기본값(CDO)까지 갱신되어 이후 배치되는 액터에도 동일 값이 적용됩니다.
6. `선택 항목 적용`을 클릭하면 모든 선택된 액터에 즉시 반영되고 카메라가 재설정됩니다.

## 저장 및 검증
- 기본값을 갱신한 경우, 관련 블루프린트/데이터 에셋을 `File > Save All` 또는 콘텐츠 브라우저에서 저장하세요.
- 런타임에서 튜닝 값이 정상 동작하는지 PIE에서 확인하고, 필요 시 카메라 상태 전환(조준, 스프린트 등)을 직접 테스트합니다.

## 문제 해결
- 위젯 탭이 보이지 않으면 `create_camera_tuning_widget.py`를 다시 실행해 새 탭을 띄웁니다.
- 선택한 액터가 인터페이스를 구현하지 않았다면 목록이 비어 보일 수 있으므로 플레이어 클래스만 선택했는지 확인하세요.
- 에디터 셧다운 후 재실행 시에도 에셋이 남아 있으므로 스크립트를 반복 실행할 필요는 없습니다.

