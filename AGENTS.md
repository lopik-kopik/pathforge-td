# Pathforge TD - Android Tower Defense Game

This document provides essential information for AI coding agents working on this Android tower defense game project.

## Project Overview

This is a native Android tower defense game built with C++ and OpenGL ES 3.0. The game uses a hybrid architecture with a minimal Kotlin/Java layer and the majority of game logic implemented in native C++ code.

### Technology Stack

- **Language**: Kotlin (entry point) + C++17 (game engine)
- **Build System**: Gradle with CMake
- **Graphics**: OpenGL ES 3.0, EGL
- **Android SDK**: API 31+ (Android 12.0), target SDK 34, compile SDK 36
- **Game Activity**: `androidx.games:games-activity:4.0.0`
- **NDK/CMake**: CMake 3.22.1 for native builds

### Project Structure

```text
pathforge-td/
|- app/src/main/
|  |- cpp/                          # Native C++ game code
|  |  |- CMakeLists.txt             # CMake build configuration
|  |  |- main.cpp                   # Native entry point (android_main)
|  |  |- Game.cpp/h                 # Main game logic
|  |  |- Renderer.cpp/h             # OpenGL rendering
|  |  |- Map.cpp/h                  # Grid-based game map (10x16)
|  |  |- Tower.cpp/h                # Tower placement and logic
|  |  |- Enemy.cpp/h                # Enemy movement and stats
|  |  |- Projectile.cpp/h           # Projectile physics
|  |  |- Wave.cpp/h                 # Wave management system
|  |  |- HUD.cpp/h                  # Heads-up display rendering
|  |  |- Shader.cpp/h               # OpenGL shader management
|  |  |- Model.cpp/h                # 3D model/vertex data
|  |  |- TextureAsset.cpp/h         # Texture loading
|  |  |- SpriteSheet.cpp/h          # Animation frames
|  |  |- AndroidOut.cpp/h           # Android logcat output
|  |  '- Utility.cpp/h              # OpenGL utilities
|  |- java/com/example/myapplication/
|  |  '- MainActivity.kt            # Android entry point
|  |- assets/                       # Game assets (textures)
|  '- res/                          # Android resources
|- build.gradle.kts                 # Root build script
|- app/build.gradle.kts             # App module build script
|- settings.gradle.kts              # Project settings
'- gradle/libs.versions.toml        # Dependency versions
```

## Agent Working Agreement

- Keep the Kotlin layer minimal unless Android integration requires changes.
- Put gameplay, rendering, and simulation logic in C++.
- Prefer small, isolated edits over broad refactors.
- Do not add new libraries unless clearly required by the task.
- Preserve existing public behavior unless the task explicitly requests gameplay changes.

## Game Architecture

### Native Layer (C++)

The game is primarily implemented in C++ using Android Native App Glue:

1. **Entry Point** (`main.cpp`)
   - `android_main()` is the native entry point.
   - Uses `android_native_app_glue` for event loop.
   - Creates `Renderer` instance on `APP_CMD_INIT_WINDOW`.

2. **Game Loop** (`main.cpp`)
   - Polls Android events (input, lifecycle).
   - Calls `Renderer::handleInput()` for touch processing.
   - Calls `Renderer::render()` each frame.

3. **Core Classes**
   - `Game`: Main game logic and state machine (`PLAYING`, `WAVE_COMPLETE`, `GAME_OVER`, `VICTORY`).
   - `Renderer`: OpenGL ES 3.0 rendering and asset loading.
   - `Map`: 10x16 grid with `PATH`, `GRASS`, `BLOCKED`, `TOWER` cells.
   - `Tower`: Archer towers with range, damage, fire rate.
   - `Enemy`: Slimes and Goblins with HP, speed, rewards.
   - `Projectile`: Arrow projectiles targeting enemies.
   - `WaveManager`: Multi-wave enemy spawning system.
   - `HUD`: UI rendering (gold, HP, wave count).

4. **Graphics System**
   - OpenGL ES 3.0 with vertex/fragment shaders.
   - Orthographic projection (world: 10x16 units).
   - Texture-based sprites with fallback to colored quads.
   - Alpha blending enabled.

### Kotlin Layer

- `MainActivity.kt`: Extends `GameActivity`, loads native library.
- Hides system UI for immersive fullscreen experience.

## Build Configuration

### Gradle (`app/build.gradle.kts`)

```kotlin
android {
    namespace = "com.example.myapplication"
    compileSdk = 36
    defaultConfig {
        minSdk = 31
        targetSdk = 34
    }
    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }
}
```

### CMake (`app/src/main/cpp/CMakeLists.txt`)

- Builds `libmyapplication.so` shared library.
- Links with `game-activity::game-activity_static`.
- OpenGL ES 3.0 libraries: `EGL`, `GLESv3`.
- Additional: `jnigraphics`, `android`, `log`.

## Build and Run Commands

Use `gradlew` on Unix-like shells and `gradlew.bat` in Windows PowerShell/CMD.
This project is configured to use Android Studio JBR from `gradle.properties`:
`org.gradle.java.home=C:\\Program Files\\Android\\Android Studio\\jbr`.
If CLI build fails with "Gradle requires JVM 17 or later", verify this value first.

### Build Debug APK

```bash
./gradlew assembleDebug
# Windows
.\gradlew.bat assembleDebug
# Preferred module-scoped command
.\gradlew.bat :app:assembleDebug
```

### Build Release APK

```bash
./gradlew assembleRelease
# Windows
.\gradlew.bat assembleRelease
```

### Install and Run on Device

```bash
./gradlew installDebug
# Windows
.\gradlew.bat installDebug
```

### Clean Build

```bash
./gradlew clean
# Windows
.\gradlew.bat clean
```

### Run Tests

```bash
# Unit tests
./gradlew test
# Windows
.\gradlew.bat test
# Preferred module-scoped unit tests
.\gradlew.bat :app:testDebugUnitTest

# Instrumented tests (requires connected device)
./gradlew connectedAndroidTest
# Windows
.\gradlew.bat connectedAndroidTest
# Preferred module-scoped instrumented tests
.\gradlew.bat :app:connectedDebugAndroidTest
```

### Android Studio Build/Test (Recommended)

1. Use the embedded JDK (`jbr-21`) in Android Studio Gradle settings.
2. Build: `Build > Make Project` or run the `app` configuration.
3. Unit tests: right-click `app/src/test` and run tests.
4. Instrumented tests: run on a connected device/emulator from `app/src/androidTest`.

## Code Style Guidelines

### C++ Style

- **Naming**: Classes use `PascalCase`, methods use `camelCase`, private members use trailing underscore (`name_`).
- **Headers**: Use `#ifndef/#define` include guards (for example, `TOWERDEFENSE_GAME_H`).
- **Namespaces**: No explicit namespace currently used.
- **Comments**: Use `//` for short intent comments and avoid obvious comments.
- **Includes**: Group system headers first, then third-party, then local.

Example:

```cpp
#ifndef TOWERDEFENSE_GAME_H
#define TOWERDEFENSE_GAME_H

#include <vector>
#include "Map.h"

class Game {
public:
    Game();
    void update(float dt);

private:
    int gold_;
    std::vector<Tower> towers_;
};

#endif // TOWERDEFENSE_GAME_H
```

### Kotlin Style

- **Naming**: Classes use `PascalCase`, methods/properties use `camelCase`.
- **Indentation**: 4 spaces.
- Use companion objects for static native library loading.

## Native Runtime Rules (Important)

- Handle EGL/OpenGL lifecycle events correctly; recreate resources after context loss.
- Check shader compile and program link status, and log full errors.
- Guard all pointer casts and object lifetimes around app lifecycle callbacks.
- Avoid per-frame heap allocations in hot render/update loops.
- Keep world-space conversion logic centralized to avoid input/render mismatch.

## Input Handling

Touch input is handled in `Renderer::handleInput()`:

- Converts screen coordinates to world coordinates (10x16 grid).
- Tap to place towers on valid grass tiles.
- Costs 25 gold per tower.

## Asset Management

Assets are loaded from `app/src/main/assets/`:

- `tile_grass.png` - Grass terrain texture
- `tile_path.png` - Path terrain texture
- `tile_tree.png` - Blocked terrain texture
- `tower_archer.png` - Tower sprite
- `enemy_slime.png` - Slime enemy sprite
- `enemy_goblin.png` - Goblin enemy sprite
- `projectile_arrow.png` - Projectile sprite

If assets fail to load, the game falls back to solid colored quads.

## Game Mechanics

### Map

- Grid size: 10 columns x 16 rows.
- Predefined path waypoints for enemy movement.
- Players place towers on grass cells only.

### Towers

- Cost: 25 gold.
- Automatically target enemies in range.
- Fire projectiles at nearest enemy.

### Enemies

- **Slime**: Basic enemy, moderate HP and speed.
- **Goblin**: Stronger enemy, higher HP.
- Follow path waypoints from spawn to base.
- Deal 1 damage to base on reaching end.

### Waves

- Multiple waves with increasing difficulty.
- 5-second delay between waves.
- Victory condition: complete all waves with base HP > 0.

### Resources

- Starting gold: 100.
- Earn gold by defeating enemies.
- Base HP: 20.

## Testing

### Unit Tests

- Location: `app/src/test/java/`
- Framework: JUnit 4
- Current coverage: minimal (example tests only)

### Instrumented Tests

- Location: `app/src/androidTest/java/`
- Framework: AndroidJUnit4, Espresso
- Requires connected Android device/emulator

### Native Code Testing

No dedicated native test framework is currently configured. Consider adding Google Test for C++ unit tests.

## Debugging

### Native Debugging

- Use Android Studio native debugger.
- Check logcat for `aout` messages (tag: `myapplication`).
- Common issues: EGL context loss, shader compilation errors.

### Useful Logcat Commands

```bash
adb logcat -s myapplication:D
```

## Security and Stability Notes

- Native code uses `reinterpret_cast`; verify pointer ownership and validity.
- No network communication in current implementation.
- Assets are loaded from APK assets only (no external storage).
- No sensitive data storage.

## Definition of Done for Agent Changes

Before finishing a task, agents should complete this checklist when applicable:

1. Build passes for touched modules (`assembleDebug` at minimum).
2. No new native warnings or obvious lifecycle regressions.
3. Gameplay changes validated in one manual run path (tower placement, wave progress, damage handling).
4. New assets (if any) are referenced correctly and have fallback behavior.
5. Notes are added to this file when architecture/process assumptions change.

## Known Limitations

1. No save/load game state persistence.
2. Limited automated test coverage.
3. No sound/audio system.
4. Simple colored-quad HUD (no text rendering).
5. Projectile target invalidation on enemy death needs refinement.
