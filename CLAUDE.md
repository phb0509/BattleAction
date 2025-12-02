# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

BattleAction is an Unreal Engine 5.6 C++ project implementing a third-person action combat game with a sword-and-shield main player fighting against AI-controlled monsters. The project uses Enhanced Input, Motion Warping, CommonUI, and Niagara systems.

## Build Commands

### Building the Project
```bash
# Generate Visual Studio project files (if needed)
# Right-click BattleAction.uproject → "Generate Visual Studio project files"

# Build in Visual Studio
# Open BattleAction.sln and build the solution (Ctrl+Shift+B)
# Or use MSBuild from command line:
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" BattleAction.sln /p:Configuration=Development /p:Platform=Win64
```

### Opening in Unreal Editor
```bash
# Launch the project
# Double-click BattleAction.uproject
# Or use UE command line:
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" "D:\UnrealProject\BattleAction\BattleAction.uproject"
```

## Architecture Overview

### Module Structure

The project consists of two modules:

1. **BattleAction** (Runtime): Main gameplay module
2. **EditorModule** (Editor): Editor-only utilities for custom detail panels

### Core Class Hierarchy

```
CharacterBase (abstract)
├── PlayableCharacter (abstract)
│   └── MainPlayer (concrete)
└── Monster (abstract, poolable)
    ├── SuperMinion
    └── LichKing
```

**CharacterBase** (`Source/BattleAction/public/CharacterBase/CharacterBase.h`)
- Base class for all combat characters implementing the `IDamageable` interface
- Contains core combat components: `UStatComponent`, `UCrowdControlComponent`, `UMotionWarpingComponent`
- Manages hit detection tracking via `m_HitActorsByMe` map to prevent multi-hits
- Provides invincibility, super armor, crowd control state management
- Uses delegates: `FOnTakeDamageDelegate`, `FOnDeathDelegate`, `FOnCrowdControl_End_Delegate`

**PlayableCharacter** (`Source/BattleAction/public/PlayableCharacter/PlayableCharacter.h`)
- Abstract base for player-controllable characters
- Integrates Enhanced Input system with dynamic Input Mapping Context switching
- Contains `USkillComponent` for skill execution and management
- Handles camera (SpringArm + CameraComponent), lock-on targeting system
- Provides directional input processing with 8-directional movement using `DirectionIndex` lookup table

**MainPlayer** (`Source/BattleAction/public/MainPlayer/MainPlayer.h`)
- Concrete implementation of playable character with sword and shield
- Multiple collision components: sword collider, shield attack collider, shield guard collider
- Parrying system with special effects
- Inherits from PlayableCharacter, uses `UMainPlayerSkillComponent`

**Monster** (`Source/BattleAction/public/Monster/Monster.h`)
- Abstract base for AI enemies implementing `IPoolableActor` for object pooling
- FSM-based AI using Behavior Trees with `m_CurFSMState` tracking
- Template method `SetFSMState<T>()` with C++20 concepts for type-safe state management
- Groggy system when stamina reaches zero
- Uses Timeline for death fade-out effects

### Component System

**UStatComponent** (`Source/BattleAction/public/Component/StatComponent.h`)
- Manages HP, Stamina, Defense, MoveSpeed, AttackSpeed
- Auto-recovery systems for HP and Stamina with timer handles
- Delegates: `FOnStatIsZeroDelegate`, `FOnChangedStatDelegate`
- Percentage-based stat modifications (e.g., `AddMoveSpeed()`, `AddAttackSpeed()`)

**USkillComponent** (`Source/BattleAction/public/Component/SkillComponent.h`)
- Manages skill execution for playable characters
- Skills organized by Input Mapping Context (e.g., "OnGround", "InAir")
- Stores skill instances in `TMap<FName, FSkillList>` structure
- Handles gravity scaling for aerial skills
- Tracks currently executing skill via `m_CurSkill`

**UCrowdControlComponent** (`Source/BattleAction/public/Component/CrowdControlComponent.h`)
- Handles crowd control effects: Knockback, Airborne, Down, Groggy, Execution
- Integrates with character movement for physics-based CC effects

### Skill System

**USkill** (`Source/BattleAction/public/PlayableCharacter/Skill.h`)
- Abstract base class for all player skills
- Skills are UObjects created per-character instance
- Cooldown management, stamina cost, super armor properties
- UI integration with thumbnail textures for skill slots
- Delegate `FOnExecute` for UI notifications

Skills are organized in subdirectories by character:
- `Source/BattleAction/public/MainPlayer/Skill/`: MainPlayer skills (NormalAttack, DashAttack, Guard, Dodge, Execution, etc.)

Each skill defines behavior in `Execute()` and validation in `CanExecuteSkill()`.

### Subsystems (GameInstanceSubsystem)

**UDataManager** (`Source/BattleAction/public/SubSystems/DataManager.h`)
- Centralized storage for attack data loaded from Data Tables
- Maps `TSubclassOf<AActor>` to attack information by attack name
- Methods: `LoadAttackInformation()`, `GetAttackInformation()`, `InitHitActors()`
- Attack data includes damage ratio, crowd control type, knockback, airborne power, stamina damage

**UBattleManager** (`Source/BattleAction/public/SubSystems/BattleManager.h`)
- Centralized combat processing system
- `Attack()` method handles all damage calculations and application
- Creates collision shapes and queries for hit detection
- Works with UDataManager to retrieve attack information

**UUIManager** (`Source/BattleAction/public/SubSystems/UIManager.h`)
- Manages all UI creation and updates: HP bars, skill slots, lock-on indicators, damage numbers
- Widget lifecycle management with pooling for damage numbers
- Binds to component delegates for automatic UI updates

**UActorPoolManager** (`Source/BattleAction/public/SubSystems/ActorPoolManager.h`)
- Object pooling system for frequently spawned actors (VFX, projectiles, monsters)
- References a single `AActorPool` instance

**UDebugManager** (`Source/BattleAction/public/SubSystems/DebugManager.h`)
- Development utilities for debugging gameplay systems

### AI System

Monster AI uses Unreal's Behavior Tree system with custom nodes:

**Behavior Tree Nodes**:
- `UMontageTaskNodeBase`: Base class for animation montage-based tasks with instance memory
- FSM-specific tasks like `BTT_LichKing_NormalAttack`, `BTT_SuperMinion_NormalAttack`
- Service nodes: `BTS_FindPatrolPos`, `BTS_SetMaxWalkSpeed`, `BTS_SetFSMState`

Monsters use FSM enums (e.g., `ELichKingFSMStates`, `ESuperMinionFSMStates`) synchronized with Blackboard via `SetFSMState()` template method.

### Data Structures

**FAttackInformation** (`Source/BattleAction/public/Utility/CustomStructs.h`)
- Runtime attack data: damage ratio, crowd control type/time, knockback distance, airborne power, stamina damage
- Loaded from `FAttackInformationData` Data Tables

**FHitInformation** (`Source/BattleAction/public/Utility/CustomStructs.h`)
- Complete hit result: attacker, target, final damage, critical flag, hit direction, CC info
- Passed through damage delegates for UI/VFX responses

**FInputConfig** (`Source/BattleAction/public/PlayableCharacter/PlayableCharacter.h`)
- Maps Input Mapping Context name to UInputMappingContext and action bindings
- Enables dynamic context switching (e.g., ground vs. air controls)

### Animation System

**UAnimInstanceBase** (`Source/BattleAction/public/CharacterBase/AnimInstanceBase.h`)
- Base animation instance for all characters
- Likely handles blend spaces, montage management, and state machines

Animation Notifies:
- `Notify_FindTarget`: Target acquisition during attacks
- `NotifyState_Check_Collision`: Hit detection windows in attack montages
- `NotifyState_Check_KeyInput`: Combo input buffering
- `NotifyState_MotionWarping_Move`: Integration with Motion Warping plugin
- `RotateToTarget`: Monster rotation towards player
- `SoulSiphonAttack`: Boss-specific attack notify

### Level and Game Flow

**LevelBase** (`Source/BattleAction/public/LevelBase.h`)
- Base class for gameplay levels

**FirstLevel** (`Source/BattleAction/public/FirstLevel.h`)
- Main gameplay level implementation

**LoadingLevel** (`Source/BattleAction/public/LoadingLevel.h`)
- Handles level transitions and loading screens

**MainPersistantLevel** (`Source/BattleAction/public/MainPersistantLevel.h`)
- Persistent level for cross-scene data

**MonsterSpawner/MonsterSpawnerManager** (`Source/BattleAction/public/GameSystem/`)
- Enemy spawning and wave management

## Key Patterns and Conventions

### Naming Conventions
- Member variables: `m_VariableName` prefix
- Booleans: `m_bIsCondition` or `bCondition` for parameters
- Unreal types: `TObjectPtr<>`, `TWeakObjectPtr<>`, `TSubclassOf<>`
- Public inline getters use `FORCEINLINE` macro

### Delegate Usage
Multicast delegates are heavily used for loose coupling:
- `FOnTakeDamageDelegate` → UI damage numbers, VFX
- `FOnDeathDelegate` → Death cleanup, rewards
- `FOnStatIsZeroDelegate` → Groggy state, death triggers
- `FOnExecute` (skills) → UI cooldown display

### Collision and Hit Detection
- Characters maintain `m_HitActorsByMe` map keyed by attack name
- Before applying damage, check `HasContainHitActor()` to prevent multi-hits
- After hit, call `AddHitActorsByMe()`
- Clear on new attack with `EmptyHitActorsByMe()`

### Input Mapping Context Switching
PlayableCharacter supports dynamic input contexts:
- Use `AddInputMappingContext()` / `RemoveInputMappingContext()` to switch controls
- Skills check `GetHighestPriorityInputMappingContext()` to determine valid actions
- Common contexts: "OnGround", "InAir", "Guard"

### FSM State Management for Monsters
Use the templated `SetFSMState<T>()` method with enum types:
```cpp
SetFSMState(ELichKingFSMStates::NormalAttack);
```
This updates both the internal `m_CurFSMState` and the Behavior Tree Blackboard.

### Object Pooling
Monsters implement `IPoolableActor`:
- Use `UActorPoolManager::GetActorPool()` to retrieve the pool
- Pool actors instead of spawning/destroying for performance

### Motion Warping
Characters have `UMotionWarpingComponent`:
- Use for root motion adjustments (e.g., dashing to target, aerial attacks)
- Configure warp targets in Animation Notifies

## Common Development Workflows

### Adding a New Skill
1. Create new skill class inheriting from `USkill` in `Source/BattleAction/public/MainPlayer/Skill/`
2. Implement `Execute()` and `CanExecuteSkill()` methods
3. Set skill properties in constructor: `m_Name`, `m_StaminaCost`, `m_CoolDownTime`, `m_bIsSuperArmor`
4. Add to skill component's skill class list in Blueprint/C++ configuration
5. Create corresponding animation montage with collision check notifies
6. Add attack information to Data Table (if dealing damage)

### Adding a New Monster
1. Create class inheriting from `AMonster` in `Source/BattleAction/public/Monster/`
2. Define FSM enum in `EnumTypes.h` (e.g., `ENewMonsterFSMStates`)
3. Implement AI Controller inheriting from `AAIControllerBase`
4. Create Behavior Tree with custom task nodes (inherit from `UMontageTaskNodeBase`)
5. Create Animation Blueprint inheriting from `UAnimInstanceBase`
6. Configure attack data in Data Tables
7. Implement pooling via `IPoolableActor` interface

### Adding Attack Data
1. Create/edit Data Table based on `FAttackInformationData` row struct
2. Load in character's `BeginPlay()` via `UDataManager::LoadAttackInformation()`
3. Reference attack by name in animation notifies and skill execution
4. Attack information retrieved automatically by `UBattleManager::Attack()`

### Debugging Combat
- Use `UDebugManager` subsystem for gameplay debug info
- Check `m_HitActorsByMe` if attacks aren't hitting
- Verify collision components are enabled during attack windows (NotifyState_Check_Collision)
- Ensure attack data is loaded in DataManager

## Editor Module

**DetailCustomization_SetFSMServiceNode** (`Source/EditorModule/Public/DetailCustomization_SetFSMServiceNode.h`)
- Custom detail panel for FSM service nodes in Behavior Trees
- Provides improved UI for configuring monster AI state management
- Only active in Editor builds

## Content Organization

- `Content/MainPlayerAsset/`: Main character assets (meshes, animations, blueprints)
- `Content/MonsterAsset/`: Enemy assets organized by monster type
- `Content/Maps/`: Level maps
- `Content/UI/`: Widget blueprints
- `Content/DataAsset/`: Data Tables for attack information and configuration
- `Content/Assets/`: Shared assets (materials, textures, VFX)

## Dependencies and Plugins

**Engine Modules**:
- Enhanced Input: Modern input system with mapping contexts
- Motion Warping: Root motion warping for dynamic animations
- CommonUI: Scalable UI framework
- Niagara: VFX system
- GameplayCameras: Advanced camera system
- AIModule: Behavior Trees, Blackboards, Perception
- UMG: UI widget system

**Project Configuration**:
- Engine Version: 5.6
- Target Platform: Win64
- Build Configuration: Development Editor (typical)
