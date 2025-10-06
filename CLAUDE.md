# HoldemCore - AI Assistant Context File

This file contains comprehensive context about the HoldemCore Texas Hold'em poker simulation project for AI assistant sessions.

## Project Overview

**HoldemCore** is a C++20 Texas Hold'em poker simulation engine with the following key characteristics:
- **Language**: C++20 with CMake build system
- **Compiler**: Clang (LLVM) on Windows
- **Architecture**: SOLID principles with Interface Segregation Principle (ISP) compliance
- **Testing**: Google Test framework with unit and e2e tests
- **Purpose**: Poker game simulation, bot strategies, and player statistics tracking

## Active Development Focus: Complete ISP Enforcement

### **CURRENT PRIORITY: Eliminate All Legacy Code**
The project is actively working to achieve **complete ISP compliance** by eliminating all remaining service container structures and legacy code:

**Primary Objectives:**
- 🎯 **ELIMINATE**: All `ServiceContainer` usage and related infrastructure
- 🎯 **REMOVE**: All legacy backward-compatibility methods (e.g., `ensureServicesInitialized`)
- 🎯 **ENFORCE**: Pure dependency injection with direct interface passing only
- 🎯 **ACHIEVE**: Simple, clean hexagonal architecture without service locator patterns

### Target Architecture (Final State)
- **No Service Containers**: Complete elimination of service container lookup patterns
- **Pure ISP Compliance**: Every component receives exactly the interfaces it needs via constructor
- **No Legacy Code**: Remove all transitional/compatibility code and helper methods
- **Hexagonal Simplicity**: Clean core domain with infrastructure adapters on the edges

### Completed ISP Architectural Refactoring
The project recently underwent a major architectural improvement to achieve Interface Segregation Principle compliance:

**Before**: Over-engineered wrapper interfaces (`HasLogger`, `HasRandomizer`, `HasHandEvaluationEngine`, `HasPlayersStatisticsStore`)
**After**: Direct core interfaces (`Logger`, `Randomizer`, `HandEvaluationEngine`, `PlayersStatisticsStore`)

### Successfully Converted Components
- ✅ **Player System**: `Player` class, `PlayerStatisticsUpdater`, `MixedPlayerFactory`
- ✅ **Engine States**: `PreflopState`, `FlopState`, `TurnState`, `RiverState`, `PostRiverState`
- ✅ **Engine Factory**: `EngineFactory` with hybrid ISP/ServiceContainer support (TO BE CLEANED)
- ✅ **Service Layer**: `ServiceAdapter` helper for ISP compliance
- ✅ **Test Framework**: `SessionUnitTest` and related test infrastructure

### Components Needing Legacy Code Removal
- 🔄 **Board**: Contains `ensureServicesInitialized()` method - should be removed
- 🔄 **EngineFactory**: Has ServiceContainer compatibility - should be pure ISP
- 🔄 **Other Components**: Any remaining service container dependencies

### Current Architecture Patterns
```cpp
// ISP-Compliant Constructor Pattern
Player(const GameEvents& events,
       std::shared_ptr<Logger> logger,
       std::shared_ptr<HandEvaluationEngine> handEvaluator, 
       std::shared_ptr<PlayersStatisticsStore> statisticsStore,
       std::shared_ptr<Randomizer> randomizer,
       int id, const std::string& name, int cash);

// ServiceAdapter Pattern for Migration
class ServiceAdapter {
    std::shared_ptr<Logger> createLoggerService() const;
    std::shared_ptr<Randomizer> createRandomizerService() const;
    // Uses non-deleting custom deleters for shared ownership
};
```

## Project Structure

```
HoldemCore/
├── src/
│   ├── core/
│   │   ├── engine/           # Game engine components
│   │   │   ├── actions/      # Player actions (fold, call, raise, etc.)
│   │   │   ├── cards/        # Card utilities and hand evaluation
│   │   │   ├── game/         # Board, pot management
│   │   │   ├── hand/         # Hand lifecycle and state management
│   │   │   ├── model/        # Game data structures
│   │   │   ├── state/        # FSM states (Preflop, Flop, Turn, River, PostRiver)
│   │   │   └── utils/        # Helper utilities
│   │   ├── interfaces/       # Core interfaces (ISP-compliant)
│   │   │   ├── persistence/  # Statistics storage interfaces
│   │   │   ├── Logger.h      # Logging interface
│   │   │   ├── Randomizer.h  # Random number generation
│   │   │   ├── HandEvaluationEngine.h
│   │   │   └── ServiceAdapter.h  # ISP migration helper
│   │   ├── player/           # Player management and strategies
│   │   │   ├── strategy/     # Bot strategies (TightAggressive, LooseAggressive, etc.)
│   │   │   ├── Player.h/.cpp # Main player class (ISP-compliant)
│   │   │   └── PlayerStatisticsUpdater.h/.cpp
│   │   ├── services/         # Service container and dependency injection
│   │   └── session/          # Game session management
│   ├── infra/                # Infrastructure implementations
│   │   ├── persistence/      # SQLite statistics storage
│   │   ├── eval/             # Hand evaluation engines
│   │   └── ConsoleLogger.cpp # Logging implementation
│   └── ui/                   # User interface (Qt widgets)
└── tests/
    ├── unit/                 # Unit tests (389 tests passing)
    ├── e2e/                  # End-to-end integration tests
    └── common/               # Test utilities and fixtures
```

## Key Technical Details

### Build System
- **CMake** with presets in `CMakePresets.json`
- **Build Directory**: `build/vscode/clang/release-widgets/`
- **Executables**: 
  - `src/app/HoldemCore-widgets.exe` (main application)
  - `tests/unit_tests.exe` (389 passing tests)
  - `tests/e2e_tests.exe` (integration tests)

### Dependencies
- **Google Test/Mock**: Unit testing framework
- **SQLite3**: Player statistics persistence
- **Qt Widgets**: GUI components
- **Poker-eval library**: Hand evaluation routines
- **Custom PSIM engine**: hand evaluator, based on Poker-eval

### Core Classes and Responsibilities

#### Player System
- **Player**: Main player entity with ISP-compliant constructor
- **PlayerStatisticsUpdater**: Tracks and persists player statistics
- **Strategy Classes**: Bot behavior implementations (TightAggressive, LooseAggressive, Maniac, etc.)

#### Engine System  
- **EngineFactory**: Creates game components with hybrid ISP/ServiceContainer support
- **Hand**: Manages single hand lifecycle and statistics persistence
- **Board**: Manages community cards and pot
- **State Classes**: FSM implementation for game phases

#### Service System
- **ServiceContainer/AppServiceContainer**: Dependency injection container
- **ServiceAdapter**: Helper for ISP migration with non-deleting custom deleters
- **PokerServices**: Legacy service wrapper

## Testing Status
- **Unit Tests**: 389/389 passing ✅
- **E2E Tests**: All passing ✅  
- **Build Status**: Clean compilation ✅
- **Recent Fix**: `SaveAndLoadStatistics` test resolved via EngineFactory ServiceContainer integration

## Development Environment
- **OS**: Windows 
- **Shell**: bash.exe (Git Bash)
- **Compiler**: Clang-cl (LLVM)
- **IDE**: VS Code with CMake Tools extension
- **Repository**: Git (main branch)

## Common Development Tasks

### Building
```bash
cd "c:\Users\Utilisateur\dev\perso\cpp\poker\HoldemCore"
cmake --build build/vscode/clang/release-widgets
```

### Running Tests  
```bash
# All unit tests
build/vscode/clang/release-widgets/tests/unit_tests.exe

# Specific test filter
build/vscode/clang/release-widgets/tests/unit_tests.exe --gtest_filter="*TestName*"

# E2E tests
build/vscode/clang/release-widgets/tests/e2e_tests.exe
```

### Code Quality
- Follows SOLID principles with recent ISP compliance improvements
- Uses modern C++20 features
- Comprehensive test coverage with behavior-driven scenarios
- Clean architecture with proper separation of concerns

## Recent Problem Resolutions

### ISP Compliance Migration
**Problem**: Over-engineered wrapper interfaces creating unnecessary complexity
**Solution**: Systematic conversion to direct core interfaces throughout codebase
**Impact**: Cleaner architecture, better testability, maintained functionality

## Architecture Principles

### Interface Segregation Principle (ISP)
- Components depend only on interfaces they actually use
- Direct interface injection instead of fat service containers
- `ServiceAdapter` provides migration path from legacy patterns

### Dependency Injection
- Constructor injection with shared_ptr for interface dependencies
- ServiceContainer for complex object graphs
- Custom deleters for shared ownership without lifecycle management

### State Machine Design
- Game phases implemented as FSM states (Preflop → Flop → Turn → River → PostRiver)
- State transitions handled by `HandStateManager`
- Clean separation between state logic and game data

## File Locations for Quick Access

### Key Headers
- `src/core/player/Player.h` - Main player class (ISP-compliant)
- `src/core/engine/EngineFactory.h` - Component factory 
- `src/core/interfaces/ServiceAdapter.h` - ISP migration helper
- `src/core/services/ServiceContainer.h` - Dependency injection

### Key Implementation Files  
- `src/core/player/Player.cpp` - Player implementation
- `src/core/engine/EngineFactory.cpp` - Factory implementation
- `src/core/engine/hand/Hand.cpp` - Hand lifecycle management

### Test Files
- `tests/unit/session/SessionUnitTest.cpp` - Session testing (recently fixed)
- `tests/unit/db/SqlitePlayersStatisticsStoreTest.cpp` - Statistics persistence tests

## Development Guidelines & Priorities

### **PRIMARY FOCUS: ISP Enforcement & Legacy Code Elimination**
When working on this codebase, the highest priority is to:

1. **Identify and Remove Legacy Code**:
   - Look for `ensureServicesInitialized()` methods and similar patterns
   - Remove ServiceContainer dependencies where possible
   - Convert remaining service locator patterns to pure dependency injection

2. **Enforce Pure ISP Compliance**:
   - Every constructor should receive only the specific interfaces it needs
   - No service containers or service locator patterns
   - Direct interface injection only

3. **Maintain Hexagonal Architecture**:
   - Core domain logic should not depend on infrastructure details
   - Use adapter patterns for external dependencies
   - Keep business logic pure and testable

### Code Patterns to Avoid
- `ensureServicesInitialized()` methods
- Service container lookups within business logic
- Backward compatibility layers for non-ISP code
- Any form of service locator pattern

### Code Patterns to Prefer
- Constructor injection with specific interfaces
- Pure business logic without infrastructure dependencies
- Clear separation between core and adapters

## Notes for AI Assistant

### When Starting New Sessions
1. **REMEMBER**: Current focus is eliminating ALL legacy code and achieving pure ISP compliance
2. Check current build status with cmake build command
2. Run unit tests to verify system health  
3. Review any compilation errors or test failures
4. Consider ISP compliance when suggesting architectural changes
5. Use ServiceAdapter pattern for any new ISP migrations

### Code Style Preferences
- Modern C++20 features preferred
- SOLID principles adherence required
- Comprehensive unit test coverage expected
- Clear separation between interfaces and implementations

### Common Issues to Watch
- Include path corrections when adding new interfaces
- ServiceContainer integration when using ISP constructors  
- Test fixture updates when changing constructor signatures
- CMake dependency management for new components

## Other documentations
doc/architecture.puml
README.md

---
*This context file was last updated: October 6, 2025*
*All tests passing, ISP compliance completed, statistics persistence resolved*