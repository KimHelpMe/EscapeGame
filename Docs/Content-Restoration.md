# Content 복원 안내

## 공개 저장소의 포함 범위

- 프로젝트의 Blueprint·입력·힌트·UI·레벨 데이터와 참조된 Epic 템플릿 예제를 포함합니다.
- 포함 파일: **1,825개**, 원본 바이트 합계 **395.20 MiB**. `.uasset`과 `.umap`은 Git LFS로 관리합니다.
- 외부 팩 원본과 출처 확인이 필요한 복사본: **117개**, **669.25 MiB**. 공개 저장소에서 제외하고 로컬 복원 목록으로 관리합니다.
- 사용 후보는 설정된 시작 맵·캐릭터·GameInstance·MPC와 맵의 외부 액터에서 시작해 패키지 이름 테이블의 `/Game` 참조를 따라 정리했습니다. 실행 중 동적 로딩까지 보장하는 목록은 아닙니다.

`AssetManifest.json`에 경로·크기·SHA-256·분류·참조 경로를 기록했습니다. 원본 프로젝트에 없는 참조 12개도 별도로 표시합니다. 이 경로들이 실제 실행 오류를 일으키는지는 확인하지 않았습니다.

## 처음 받는 경우

```powershell
git clone https://github.com/KimHelpMe/EscapeGame.git
cd EscapeGame
git lfs install
git lfs pull
python Tools/content.py check
```

공개 파일을 내려받은 상태에서는 외부 에셋이 빠져 있습니다. 따라서 원래 맵과 Blueprint 전체를 그대로 실행할 수 없습니다.

## 보유한 원본에서 필요한 파일만 복원

```powershell
python Tools/content.py restore --source "D:/MyOriginalProject"
python Tools/content.py check --all
```

`--source`에는 `Content` 폴더를 포함하는 원본 프로젝트 루트를 지정합니다. 스크립트는 목록의 `restore` 파일만 복사하고 이미 존재하는 서로 다른 파일을 덮어쓰지 않습니다. Git LFS 포인터만 받은 파일도 검사에서 구분합니다. 외부 팩 파일은 `.gitignore`로 제외합니다.

## 원본이 없는 경우

필요한 팩을 본인 계정의 라이선스로 확보하고 아래 경로를 유지해 복원해야 합니다. 같은 팩이라도 버전에 따라 파일명이나 해시가 다를 수 있습니다. 이동되거나 이름을 바꾼 복사본은 프로젝트 소유자가 별도로 제공해야 할 수 있습니다.

| Content 폴더 | 복원 대상 수 | 크기(MiB) |
| --- | ---: | ---: |
| `Bodycam_VHS_Effect` | 23 | 7.50 |
| `DirtyStoneTile_material` | 1 | 5.47 |
| `FreePBRLampsPack` | 15 | 6.03 |
| `HighQualityTools` | 6 | 46.98 |
| `IndustryPropsPack6` | 26 | 31.92 |
| `MSPresets` | 1 | 0.76 |
| `NecrosUtilityMatPack` | 10 | 87.21 |
| `RamsterZ_FreeAnims_Volume1` | 2 | 0.17 |
| `SM_Tiles_02` | 11 | 383.36 |
| `ThirdPerson` | 8 | 1.16 |
| `Whisper` | 14 | 98.69 |

확인한 제품 페이지: [Industry Props Pack 6 / SilverTm](https://www.fab.com/listings/b5603e44-e1b0-4346-9c3d-04887aa9f87d). 나머지 팩은 폴더명을 기록했으며, 개별 제품 URL·취득 시점의 라이선스까지 검증한 것으로 표시하지 않았습니다.

`Content/__ExternalActors__`, `Content/__ExternalObjects__`와 `_GENERATED`의 원래 경로를 유지합니다. 이 저장소는 원본 프로젝트의 파일을 삭제하거나 변경하지 않았습니다.
