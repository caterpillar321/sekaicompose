# WorldLink

**SekaiOS 의 Wayland 합성기.** [Hyprland](https://github.com/hyprwm/Hyprland) v0.50.1 에서 갈라진 포크다.

*The Wayland compositor of [SekaiOS](https://github.com/caterpillar321/sekaios) — a fork of
[Hyprland](https://github.com/hyprwm/Hyprland) v0.50.1 by vaxerski and contributors.
WorldLink (formerly SekaiCompose) is not affiliated with or endorsed by the Hyprland project.
Please report bugs here, not upstream.*

## 왜 포크인가

SekaiOS 는 윈도우를 쓰던 사람이 설명서 없이 쓰는 데스크톱을 목표로 한다. 그래서 창을 다루는 핵심 동작 —
최대화·최소화·스냅·쌓임 순서·끌기·테두리 크기 조절·다중 모니터 — 을 윈도우처럼 바꿨다. 바꾼 곳이 많아져서
"Hyprland 에 패치를 얹은 것"이라 부르기 어려워 이름을 따로 붙였다. 앱들의 창(각자의 세계)을 한 화면으로
이어 준다는 뜻에서 WorldLink 다 (처음 이름은 SekaiCompose).

- `sekai` 브랜치 = 원본 태그 `v0.50.1` 위에 SekaiOS 커밋을 쌓은 것 (`git log v0.50.1..sekai`)
- 데비안 13(trixie)의 라이브러리(wayland-protocols 1.44 등)와 맞는 마지막 판이 v0.50.1 이라 거기서 갈라졌다
- 창 제목줄 플러그인 hyprbars 는 [hyprland-plugins](https://github.com/hyprwm/hyprland-plugins) v0.50.0 에서
  `plugins/hyprbars/` 로 가져와 함께 고친다

## 바꾼 것

| 커밋 | 내용 |
|---|---|
| `build:` | GCC 14 로 빌드 (#embed·insert_range 대체, C++26 shim `sekai/cxx26-compat.hpp`) |
| `clientmove` | 창이 스스로 그린 제목줄(크롬 탭 줄 등)을 끌어 옮기기 |
| `keepoutputs` | 끝날 때 모니터를 끄지 않음 — 로그인 화면 → 바탕화면에서 화면이 꺼지지 않게 |
| `bordergrab` | 테두리 크기 조절을 윈도우처럼 (커서, 안쪽 띠, 화면 끝) |
| `layerfocus` · `misclick` | 바탕화면 레이어가 창의 키보드 초점을 가로채지 않게, 잘못 누름 처리 |
| `dndhotspot` | 끌기 아이콘 기준점 |
| `popupreserved` · `fitnew` | 메뉴와 새 창을 작업 표시줄 뺀 작업 영역 안으로 |
| `initialmax` · `multimax` · `minimize` | 나타나기 전 최대화, 한 데스크톱에 최대화 창 여럿, 앱의 최소화 요청 |
| `raise` · `dragrestore` · `floatoffset` | 고른 창을 맨 앞으로, 최대화 창 끌어 내리기, 밀려 그려지던 창 |
| `hyprbars/*` | 벡터 창 단추, 마우스 올림, 끌어서 스냅 알림, 대화상자 단추, 판정 칸 |
| `hyprbars/release` · `drag` · `drag-anchor` | 창 단추는 같은 단추 위에서 뗄 때, 놓은 창의 제목줄을 화면 안으로, 빠른 끌기가 커서를 정확히 따라오게 |
| `dialog-follow` · `dialog-follow2` | 대화상자가 부모와 함께 최소화·복원, `hyprctl clients` 에 `sekaiParent` |
| `fixed-size` | 크기를 바꿀 수 없는 창은 최대화하지 않음, `sekaiFixed` |
| `geom-subsurface` | 스스로 그림자를 두는 앱(Firefox 탭 제목줄 등)의 하위 면·입력을 창 영역 기준으로 |

각 커밋 메시지에 무엇이 왜 문제였는지 적었다.

## 이름과 호환

- 실행 파일은 `worldlink` 다 (데비안 패키지도 `worldlink`). 옛 이름 `sekaicomp` 와 `Hyprland`·`hyprland` 는 호환용 링크로 남겼다.
  옛 패키지 `sekaicomp`·`hyprland` 는 `worldlink` 로 넘어가는 빈 전환 패키지다.
- IPC(`hyprctl`·소켓 경로)와 설정 형식은 원본과 같다.
- 플러그인은 합성기와 같은 커밋으로 빌드해야 로드된다 (커밋 해시 검사) — 커밋을 올리면 플러그인도 다시 빌드한다.
- 빌드·패키징은 SekaiOS 저장소의 `scripts/build-hypr.sh` 가 한다.

## 라이선스

원본과 같은 [BSD 3-Clause](LICENSE) 다. 원본 저작권 표시(Copyright (c) 2022-2024, vaxerski)는 그대로 두었다.
hyprbars 는 `plugins/hyprbars/LICENSE`(hyprland-plugins, BSD 3-Clause). 원본 README 는 [README.hyprland.md](README.hyprland.md).
