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
- Follow SOLID
- Maintain clear naming and function responsibility
- **Comments**: Only add comments when code doesn't speak for itself. Never add obvious comments explaining what you just did or trivial implementation details.

---

## 5. Expected AI Assistant Behavior

When proposing or modifying code:
1. **Explain reasoning first**, then propose changes.
2. **Proceed by small steps, keep the commits easily reviewable
3. **Preserve unit test integrity**.
4. **Use only C++ and Qt conventions consistent with this repo**.
5. **If unsure, state uncertainty** (“I don’t know”) rather than hallucinate.

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
4. **Test execution is NON-NEGOTIABLE** 

### 🚨 Critical Testing Protocol
- **ALWAYS run both unit tests AND e2e tests** before marking any task complete
- **Use release build**: `cmake --build build/vscode/clang/release-widgets --config Release`
- **Execute**: `tests/unit_tests.exe` and `tests/e2e_tests.exe` (release versions)
- **All tests must pass** (except explicitly disabled ones) 


---

*Last updated: October 2025 — all tests passing, refactoring ongoing.*
