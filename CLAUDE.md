# HoldemCore — AI Assistant Context File

This file defines the operational rules, architectural constraints, and behavioral expectations for AI assistants working on this repository.

---

## 1. Project Summary

HoldemCore is a **C++20 Texas Hold’em simulation engine** applying SOLID and Hexagonal Architecture principles.  
The project demonstrates clean modular design, modern dependency management, and continuous refactoring discipline.

- **Core:** pure C++ engine, Qt-free
- **UI:** Qt Widgets and QML bridges
- **Tests:** GoogleTest framework (all passing)
- **Build:** CMake + Clang (cross-platform)

---

## 2. Active Development Focus

### 🎯 Primary Goal
**Achieve complete Interface Segregation Principle (ISP) enforcement**  
and remove all legacy **ServiceContainer** or compatibility code.

### ✅ Completed ISP Migrations
- **Hand class**: Successfully converted to ISP with focused service injection (`Logger`, `PlayersStatisticsStore`)
- **EngineFactory**: Updated `createHand()` to use ISP-compliant constructor
- **Test validation**: SaveAndLoadStatistics test passes with ISP implementation

### 📋 ISP Migration TODO List

#### 🔄 In Progress
- **Board class**: Convert from ServiceContainer to focused service injection
  - Identify required services: likely `Logger`, `HandEvaluationEngine`, `Randomizer`
  - Add ISP constructor: `Board(dealerPosition, events, logger, handEvaluator, randomizer)`
  - Update `EngineFactory::createBoard()` to use focused services

#### 📌 Next Priority Classes
- **HandStateManager**: Eliminate ServiceContainer dependencies
  - Convert to receive only required interfaces
  - Update Hand constructors to pass focused services
  
- **Session class**: Apply ISP to session management
  - Remove ServiceContainer usage in session lifecycle
  - Inject focused services for session operations

#### 🧪 Test Infrastructure Updates
- **Legacy test constructors**: Update remaining test files using old EngineFactory constructor
  - Fix SessionUnitTest.cpp, SessionE2ETest.cpp compilation errors
  - Convert single-parameter EngineFactory calls to ISP constructors
  
#### 🧹 Final Cleanup Phase
- **Remove ServiceContainer remnants**: Eliminate all ServiceContainer usage
- **Remove legacy constructors**: Clean up backward-compatibility constructors
- **Remove ensureServicesInitialized()**: Eliminate all service initialization helpers

### Objectives
- Eliminate any usage of `ServiceContainer` or `ensureServicesInitialized()`
- Ensure each class receives only the interfaces it needs (constructor injection)
- Maintain clean hexagonal boundaries: **core domain ←→ adapters ←→ UI**

### Secondary Focus
- Maintain test coverage parity after refactors
- Preserve build integrity (CMake)
- Keep the core strictly Qt-free

---

## 3. Architectural Ground Rules

- Respect the design principles from [`doc/ARCHITECTURE.md`](doc/ARCHITECTURE.md)
- Core logic (under `/src/core`) must never include Qt headers
- UI communication happens only through **GameEvents** and bridges
- Infrastructure dependencies (DB, logger, RNG) are abstracted by interfaces
- Adapters in `/infra` must depend **outward** on core interfaces only
- Avoid global state, singletons, or service locators
- **NEVER modify files in `/src/third_party`** — third-party source code must remain untouched for licensing, updates, and maintainability

---

## 4. Coding Standards

- Modern C++20 (smart pointers, ranges, structured bindings)
- Member variables prefixed with `m_`
- Follow SOLID, especially SRP and ISP
- Maintain clear naming and function responsibility

---

## 5. Expected AI Assistant Behavior

When proposing or modifying code:
1. **Explain reasoning first**, then propose changes.
2. **Proceed by small steps, keep the commits easily reviewable
3. **Preserve unit test integrity**.
4. **Never reintroduce ServiceContainer or legacy helpers**.
5. **Use only C++ and Qt conventions consistent with this repo**.
6. **If unsure, state uncertainty** (“I don’t know”) rather than hallucinate.

---

## 6. Interaction Protocol with Structured Prompts

When receiving a structured prompt (XML-style):

```xml
<context>Files in play...</context>
<goal>Task description...</goal>
<constraints>Explicit rules...</constraints>
<instructions>Output format...</instructions>
```

- The **explicit prompt always overrides** any instruction here if conflicts arise.  
- `CLAUDE.md` acts as the persistent background context.  
- If a prompt temporarily violates a global rule, it must specify the exception explicitly.

---

## 7. File Awareness

AI tools may automatically read:
- `README.md` (project intro)
- `doc/ARCHITECTURE.md` (design principles)
- `CLAUDE.md` (operational rules)

Keep these documents consistent — avoid redundancy.  
Use `ARCHITECTURE.md` for design theory, and this file for **actionable guidance**.

---

## 8. Task Completion Criteria

**A task is considered COMPLETE only when:**
1. **All code compiles successfully** (including tests)
2. **ALL tests pass in RELEASE mode** (unit tests + e2e tests) - **MANDATORY before proceeding**
3. **Use RELEASE build for testing** (much faster than debug) - always use release test executables
4. **Test execution is NON-NEGOTIABLE** - every ISP migration step must be validated with full test suite
5. **Basic tests related to modified code are verified**
6. **No compilation errors or warnings introduced**

### 🚨 Critical Testing Protocol
- **ALWAYS run both unit tests AND e2e tests** before marking any task complete
- **Use release build**: `cmake --build build/vscode/clang/release-widgets --config Release`
- **Execute**: `tests/unit_tests.exe` and `tests/e2e_tests.exe` (release versions)
- **All tests must pass** (except explicitly disabled ones) before proceeding to next ISP migration step
- **NO EXCEPTIONS** - broken tests indicate incomplete or incorrect ISP implementation

### ✅ CURRENT TEST STATUS (2024-12-28)
**ALL TESTS PASSING** - ISP migration can proceed safely:
- **Unit Tests**: 389/389 PASSED (12 disabled as expected)
- **E2E Tests**: 46/46 PASSED 
- **Total**: 435/435 tests passing
- **Critical Fix Applied**: Resolved Deck::shuffle() SEH exceptions by replacing static AppServiceContainer with std::shuffle approach

### Summary of Priorities for AI Agents

| Priority | Rule |
|-----------|------|
| 1 | Ensure full compilation + test success before marking tasks complete |
| 2 | Maintain ISP enforcement |
| 3 | Remove all ServiceContainer remnants |
| 4 | Keep core Qt-free |
| 5 | Maintain test coverage |
| 6 | Follow architectural layering |
| 7 | Explain changes and output clean diffs |

---

*Last updated: October 2025 — all tests passing, refactoring ongoing.*
