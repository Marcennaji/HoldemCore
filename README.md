# HoldemCore

**🌐 Language / Langue:** [English](README.md) | [Français](README_fr.md)

**Portfolio Project: Modern C++ Architecture**

Texas Hold'em simulation engine applying modern software engineering practices: hexagonal architecture, TDD and CI/CD, SOLID principles, continuous refactoring, and cross-platform design.
The original project, created in 2011, has been substantially rewritten in 2025.

> **Portfolio Notice**: This repository is maintained as a demonstration of C++ engineering. Issues and PRs are welcome but the roadmap follows the author's priorities.

---

## 🎯 Project Showcase

Texas Hold'em simulation engine demonstrating advanced software architecture through:

- **Hexagonal Architecture** with strict separation of concerns
- **Dependency Injection** container pattern
- **Comprehensive test suite** ensuring reliability
- **Cross-platform compatibility** (Windows, Linux, macOS)
- **Multiple UI frameworks** supported through clean abstractions

### Key Features
- **Complex domain modeling**: poker rules, betting rounds, player range evaluation
- **AI strategy framework**: pluggable bot behaviors with statistical analysis
- **State machine design**: robust game flow management
- **Player behavior tracking** with database persistence

---
## 🚀 Quick Start

### Prerequisites
- CMake 3.20+
- C++20 compatible compiler
- Qt 6.x (optional, only for GUI)

### Build & Test
```bash
git clone https://github.com/Marcennaji/HoldemCore.git
cd HoldemCore
cmake --preset debug-widgets
cmake --build build/debug-widgets
.\build\debug-widgets\tests\unit_tests.exe
.\build\debug-widgets\tests\e2e_tests.exe
```

---

## 👨‍💻 About

**Marc Ennaji** - Senior Software Engineer  
[LinkedIn](https://www.linkedin.com/in/marcennaji/) 

### Expertise
- Modern C++ Development
- Software Architecture & Design
- Legacy System Modernization
- Test-Driven Development

*Available for collaborations and opportunities*

---

## 🏠 Architecture Overview

### Architecture Principles

**Hexagonal Architecture (Ports & Adapters)**

![Hexagonal Architecture Diagram](doc/architecture.png)

**Key Design Elements:**
- Dependency injection for testable boundaries
- Rich domain models with clear separation
- Event-driven communication
- Strategy pattern for pluggable AI behaviors

### 4. **Strategy Pattern Implementation**

Demonstrates **advanced OOP design** with pluggable AI behaviors:

```cpp
class BotStrategy {
public:
    virtual PlayerAction decidePreflop(const CurrentHandContext& ctx) = 0;
    virtual PlayerAction decideFlop(const CurrentHandContext& ctx) = 0;
    virtual PlayerAction decideTurn(const CurrentHandContext& ctx) = 0;
    virtual PlayerAction decideRiver(const CurrentHandContext& ctx) = 0;
};

// Clean composition over inheritance
Player player(events, "BotName");
player.setStrategy(std::make_unique<TightAggressiveStrategy>());
```

**Benefits:**
- Runtime strategy switching
- Isolated unit testing
- Plugin architecture for extensibility

### 5. **Game Engine Features**
- **Range estimation** for opponent modeling
- **Monte Carlo simulations** for equity calculations  
- **Statistical analysis** with persistent player profiles
- **State machine** managing betting rounds (Preflop → Flop → Turn → River)

### 6. **Testing Strategy**
- **Comprehensive automated testing** (unit + integration + end-to-end)
- **TDD approach** with GoogleTest framework
- **Mock services** for isolated component testing

---
## 🎮 User Interface

<table>
  <tr>
    <td width="50%">
      <img src="doc/start%20game.png" alt="Starting a new game" width="100%">
      <p align="center"><em>Game Configuration Window</em></p>
    </td>
    <td width="50%">
      <img src="doc/poker%20table.png" alt="Poker Table Interface" width="100%">
      <p align="center"><em>Poker Table Interface</em></p>
    </td>
  </tr>
</table>

*Qt Widgets version (coming soon: Qt Quick/QML version)*


## 🔧 Technical Stack

**Core Technologies:**
- **C++20** with modern language features
- **CMake** for cross-platform builds
- **GoogleTest** testing framework

**Architecture Patterns:**
- Hexagonal Architecture, Dependency Injection
- Strategy Pattern, State Machine
- Event-driven Architecture

**Platform Support:**
- **Qt 6** (Widgets and QML)
- **SQLite** embedded database
- **Cross-platform** standard C++


---
## � Development Roadmap
- eliminate completely remaining SOLID violations in legacy code, by continuous refactoring
- Develop a Qt Quick (QML) GUI, for mobile version
- explore the possibility of plugging a machine learning-based bot AI

---

## 📈 Project Scale

**19,000+ lines** of modern C++ • **Comprehensive test coverage** • **50+ classes**

---

## 📚 Dependencies & Licensing

**UI Framework (Optional):**
- [Qt 6](https://www.qt.io/) - Cross-platform application framework (Qt Widgets and Qt QML)
  
  *Note: The core engine is UI-agnostic. Qt is only required for the provided desktop interface. Developers can integrate any UI framework through the clean hexagonal architecture.*

**Third-Party Components (easily switched if needed):**
- [psim](https://github.com/christophschmalhofer/poker/tree/master/XPokerEval/XPokerEval.PokerSim) - Hand evaluation engine
- [SQLite3](https://www.sqlite.org/) - Embedded database  
- [GoogleTest](https://github.com/google/googletest) - Testing framework

**Licensing:** MIT License - Free for commercial and personal use
