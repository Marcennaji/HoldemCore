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

## 8. Summary of Priorities for AI Agents

| Priority | Rule |
|-----------|------|
| 1 | Maintain ISP enforcement |
| 2 | Remove all ServiceContainer remnants |
| 3 | Keep core Qt-free |
| 4 | Maintain test coverage |
| 5 | Follow architectural layering |
| 6 | Explain changes and output clean diffs |

---

*Last updated: October 2025 — all tests passing, refactoring ongoing.*
