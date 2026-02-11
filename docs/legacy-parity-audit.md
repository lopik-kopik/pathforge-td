# Legacy Parity Audit (C++ `:app` -> Kotlin/libGDX)

This document tracks migration of legacy gameplay/UI behavior from:
`legacy_snapshot/app/src/main/cpp`
to active modules:
- `core/`
- `game/`
- `android/`

## Current Status

- Stabilization fixes implemented:
  - map reset on mode restart (`GameMap.reset`)
  - deterministic upgrade menu visibility (`isUpgradeMenuOpen`)
  - enemy HP bars in battle HUD
  - menu frame style and chest reward interaction
- Remaining parity scope is significant and should be implemented incrementally.

## Priority Order

1. Gameplay correctness parity
2. HUD/menu/upgrade screen parity
3. Combat readability/effects parity
4. Meta-progression screens parity
5. Optional platform extras (cloud/achievements)

## Feature Backlog (Decision-Complete)

### P0 - Correctness

- [x] Reset map state between runs/sandbox transitions.
- [x] Restore upgrade menu open/close behavior.
- [ ] Ensure tower placement constraints match legacy exactly.
- [ ] Validate wave progression/endless scaling against legacy balance.

### P1 - HUD/Menu parity

- [x] Main menu framed buttons (legacy style approximation).
- [x] Corner chest interaction (+coins).
- [ ] Difficulty selection screen parity.
- [ ] Character menu parity (archer/sheriff/ally levels).
- [ ] Upgrade panel visual parity (spacing, labels, costs, max-level state).
- [ ] Pause menu visual parity.

### P2 - Combat readability parity

- [x] Enemy HP bars.
- [ ] Damage numbers.
- [ ] Hit/element particles.
- [ ] Ally HP bar and rendering parity.

### P3 - Gameplay features parity

- [ ] Ally unit logic from sheriff tower.
- [ ] Mage/sheriff special behavior parity.
- [ ] Sandbox controls parity (spawn wave, clear towers, quick economy controls).

### P4 - Progression/platform parity

- [ ] Achievements model and save integration.
- [ ] Cloud sync integration hooks (disabled by default).
- [ ] Login flow parity (if still required for product scope).

## Legacy File Inventory and Target Mapping

### Core gameplay

- `Game.cpp/.h` -> `core/.../simulation/GameWorld.kt` (state machine, actions, wave lifecycle)
- `Map.cpp/.h` -> `core/.../domain/GameMap.kt`
- `Wave.cpp/.h` -> `core/.../simulation/WaveManager.kt`
- `Enemy.cpp/.h` -> `core/.../domain/Entities.kt` + `GameWorld` update loops
- `Tower.cpp/.h` -> `core/...` tower data and placement/upgrade rules
- `Projectile.cpp/.h` -> projectile simulation in `GameWorld`
- `MageTower.cpp/.h` -> mage-specific combat behavior (pending)
- `SheriffTower.cpp/.h` -> sheriff-specific + ally spawn (pending)
- `Ally.cpp/.h` -> ally model/update loop (pending)

### UI/HUD and rendering behavior

- `HUD.cpp/.h` -> `game/.../screen/MenuScreen.kt`, `BattleScreen.kt`
- `Renderer.cpp/.h` -> render pass ordering and input routing in `game` screens
- `DamageNumber.cpp/.h` -> pending `game` overlay manager
- `ParticleSystem.cpp/.h` -> pending `game` FX manager
- `SpriteSheet.cpp/.h` -> texture-region animation helper logic in `game`

### Assets/render infrastructure

- `TextureAsset.cpp/.h`, `Shader.cpp/.h`, `Model.h`, `Utility.cpp/.h`
  - Recreate only where libGDX primitives are insufficient.

### Platform/storage

- `GameStorage.cpp/.h` -> `android/.../SharedPrefsProgressRepository.kt` + optional cloud adapter
- `Achievements.cpp/.h` -> `core` progression service (pending)
- `main.cpp`, `AndroidOut.cpp/.h`, `CMakeLists.txt` -> archival reference only

## Acceptance Criteria Per Increment

For each feature batch:

1. `.\gradlew.bat :core:test` passes.
2. `.\gradlew.bat :android:assembleDebug` passes.
3. Manual smoke via ADB screenshot confirms visual/behavior parity target.
4. No regressions in placement, wave flow, and pause/menu navigation.

