# Pathforge TD - Agent Guide (Current Architecture)

This document is the source of truth for AI agents working in this repo.

## Project Status

Pathforge TD is now migrating to a Kotlin/libGDX architecture.
The active game client is `:android` + `:game` + `:core`.

The old native C++ OpenGL implementation in `:app` is **legacy** and should not be the default target for new gameplay/features.

## Current Technology Stack

- Language: Kotlin
- Engine/runtime: libGDX
- Build system: Gradle (Kotlin DSL)
- Android target: minSdk 31, targetSdk 34, compileSdk 36
- Java runtime for Gradle: Android Studio JBR (configured via `gradle.properties`)

## Module Layout

```text
pathforge-td/
|- core/       # Pure game domain + simulation logic + unit tests
|- game/       # libGDX runtime (screens, rendering, input)
|- android/    # Android launcher, packaging, local persistence
|- desktop/    # Stub module (not priority in current milestone)
'- app/        # Legacy C++/OpenGL implementation (removed; kept in git history)\r?\n```

## Architecture (Active)

### `:core`

- Contains game state machine, map model, waves, enemies, towers, projectiles.
- No Android SDK dependencies.
- Exposes action-driven API through `GameAction` and `GameWorld.dispatch(...)`.
- Includes unit tests for gameplay scenarios.

### `:game`

- Contains libGDX `Screen` implementations and rendering/input glue.
- Uses fixed-step simulation (`1/60`) and delegates gameplay decisions to `:core`.
- Uses assets from Android assets folder.

### `:android`

- Entry point: `com.pathforge.android.AndroidLauncher`.
- Uses `SharedPrefsProgressRepository` for local save.
- Packages libGDX native `.so` through generated `jniLibs` task pipeline.

## Legacy / Deprecated (Important)

The following are **not** the primary implementation path now:

- Native loop in `app/src/main/cpp/main.cpp`
- Native renderer in `app/src/main/cpp/Renderer.cpp`
- JNI storage bridge in old C++ path
- Build/test commands scoped only to `:app`

You may still read legacy code for parity/reference, but new gameplay work should go to `:core` and `:game`.

## Build & Run (Current)

Use `gradlew.bat` in Windows PowerShell/CMD.

### Build Android debug APK (primary)

```powershell
.\gradlew.bat :android:assembleDebug
```

APK output:

```text
android/build/outputs/apk/debug/android-debug.apk
```

### Run core unit tests (primary)

```powershell
.\gradlew.bat :core:test
```

### Build everything relevant

```powershell
.\gradlew.bat :core:test :android:assembleDebug
```

### Install APK via ADB

```powershell
adb install -r android\build\outputs\apk\debug\android-debug.apk
adb shell am start -n com.pathforge.android/.AndroidLauncher
```

If `adb` is not in PATH, use:

```powershell
$adb = Join-Path $env:LOCALAPPDATA 'Android\Sdk\platform-tools\adb.exe'
& $adb devices
```

## ADB Wi-Fi Debugging (Quick Recovery)

If wireless device connection is unstable:

```powershell
$adb = Join-Path $env:LOCALAPPDATA 'Android\Sdk\platform-tools\adb.exe'
& $adb kill-server
& $adb start-server
& $adb connect <phone_ip>:<wireless_debug_port>
& $adb devices
```

Notes:
- Port is usually dynamic for Wireless Debugging (not always `5555`).
- If connection is rejected: re-pair in Developer Options -> Wireless debugging.

## Assets

Primary assets currently used in libGDX flow:

- `tile_grass.png`
- `tile_path.png`
- `tile_tree.png`
- `tower_archer.png`
- `enemy_slime.png`
- `enemy_goblin.png`
- `projectile_arrow.png`

All are under `app/src/main/assets/` and consumed by the active libGDX runtime.

## Coding Rules for Agents

- Default target for gameplay/rendering changes: `:core` and `:game`.
- Keep `:android` thin (launcher/platform integration only).
- Avoid new dependencies unless required.
- Prefer small, isolated edits.
- Preserve behavior unless change is requested.
- Add or update `:core` tests for gameplay logic changes.

## Definition of Done (Current)

Before finishing agent work:

1. `:core:test` passes.
2. `:android:assembleDebug` passes.
3. A manual smoke flow is possible: open menu -> start game -> place tower -> wave progresses.
4. If architecture/build assumptions changed, update this file.

## Known Limitations (Current Milestone)

1. Desktop module is a stub.
2. Cloud sync is temporarily disabled in active libGDX path (local save only).
3. UI is functional but still minimal; polish and parity with legacy client are in progress.
4. Some Android immersive mode APIs are deprecated warnings (non-blocking).

