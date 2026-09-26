# Arena Survivor

A top-down wave survival shooter for **Unreal Engine 5** (5.4+), written entirely in C++.

Hold out against escalating waves of enemies in a walled arena. Move, aim, shoot and
dash; grab drops for health or rapid fire; beat your high score.

The project needs **no imported art, maps, Blueprints or input assets**. The arena, lights,
input bindings and HUD are all created in code from the engine's basic shapes, so the
source tree alone is the whole game.

## Controls

| Action  | Keyboard / mouse      | Gamepad          |
|---------|-----------------------|------------------|
| Move    | WASD / arrow keys     | Left stick       |
| Aim     | Mouse cursor          | Right stick      |
| Fire    | Left mouse (hold)     | Right trigger    |
| Dash    | Space / Left Shift    | A / Cross        |
| Restart | R (after death)       | Start / Menu     |

## Gameplay

- **Waves**: each wave spawns more enemies (`5 + 3w + w²/6`) with health scaling 8% per
  wave. A short intermission separates waves.
- **Enemies**
  - *Grunt*: the baseline chaser.
  - *Runner* (from wave 2): fragile and faster than the player, so you need to dash.
  - *Brute* (from wave 4): slow, tough, hits hard and is worth 5× the points.
- **Pickups**: enemies sometimes drop health (green) or 2× rapid fire (yellow). Drops despawn
  after 12 seconds.
- **Dash**: a quick burst with 0.3 s of invulnerability, on a 1.2 s cooldown.
- **Score**: each kill scores its enemy's value × the current wave. The high score and best wave
  are kept in a save slot.

## Building

1. Install Unreal Engine 5.4 or newer and a matching C++ toolchain (Visual Studio 2022 on
   Windows, Xcode on macOS, clang on Linux).
2. Right-click `ArenaSurvivor.uproject` → *Generate Visual Studio project files*
   (or run `GenerateProjectFiles` for your platform).
3. Build the `ArenaSurvivorEditor` target in `Development Editor`, then open the project.
4. Press **Play**. The default map is the empty engine `Entry` map, and the game mode builds the
   arena on top of it.

If you open the project against a newer engine version, the editor will offer to convert the
`EngineAssociation`; accept it.

### Tests

The wave and enemy tuning is covered by automation tests. Run them from
*Tools → Session Frontend → Automation* and filter by `ArenaSurvivor`, or headless:

```
UnrealEditor-Cmd ArenaSurvivor.uproject -ExecCmds="Automation RunTests ArenaSurvivor; Quit" -unattended -nullrhi
```

## Code layout

```
Source/ArenaSurvivor/
  Public/ & Private/
    ASVisuals            Basic-shape mesh/material lookup and tinting
    Components/
      ASHealthComponent  HP, death and heal events, invulnerability; driven by ApplyDamage
      ASWeaponComponent  Automatic projectile weapon with fire-rate boosts
    Weapons/ASProjectile Sweeping projectile that applies damage on hit
    Player/
      ASPlayerCharacter  Top-down pawn, camera boom and dash
      ASPlayerController Enhanced Input built in code; mouse/stick aiming
    Enemies/
      ASEnemyTypes       Enemy type enum and per-type tuning table
      ASEnemyCharacter   Shared enemy pawn: contact attack, hit flash, death
      ASEnemyAIController  Direct steering at the player (no nav mesh needed)
    Pickups/ASPickup     Health / rapid-fire collectibles
    Game/
      ASGameMode         Arena construction, wave flow, scoring, pickup drops
      ASGameState        Match state read by the HUD
      ASSaveGame         High score persistence
    UI/ASHUD             Canvas HUD (no UMG assets)
    Tests/               Automation tests for wave and enemy tuning
```

### Design notes

- **Dependencies point inward.** The player and enemy classes know nothing about the game mode.
  The game mode subscribes to their health components' `OnDeath` events, which keeps the actors
  reusable in other modes.
- **Damage goes through `UGameplayStatics::ApplyDamage`**, so anything with a
  `UASHealthComponent` can be damaged by anything, including future Blueprint content.
- **Tuning is data plus pure functions.** `FASEnemyTuning::ForType` and the static wave functions
  on `AASGameMode` can be tested without a world, and every gameplay number is also exposed as an
  `EditDefaultsOnly` property for Blueprint subclasses.

## Extending

- Swap the shapes for real art by subclassing the characters in Blueprint and assigning meshes;
  the C++ keeps working unchanged.
- Build a real level with a `PlayerStart` and set it as `GameDefaultMap`. The procedural arena
  is still spawned around the origin, so either remove `BuildArena()` or design around it.
- New enemy types: add an `EASEnemyType` value and a case to `FASEnemyTuning::ForType`.

## Provenance

This codebase was written with AI assistance (Claude Code) in September 2026, and the Git history
reflects that development as it happened. It was written against the UE 5.4 C++ API but has not yet
been compiled or play-tested in the editor. Expect to fix small compile issues on the first build.
