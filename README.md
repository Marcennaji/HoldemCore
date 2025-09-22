# HoldemCore

**Production-Grade C++ Architecture Showcase**  
Texas Hold'em poker simulation engine demonstrating modern software engineering practices: hexagonal architecture, dependency injection, comprehensive testing, and cross-platform design.

> **Portfolio Notice**: This repository is maintained as a demonstration of production-grade C++ engineering. Issues and PRs are welcome but the roadmap follows the author's priorities.

---

## 🎯 Engineering Showcase

This project demonstrates **enterprise-level software engineering** through a complete poker simulation engine:

- **Hexagonal Architecture** with strict separation of concerns
- **Dependency Injection** eliminating all singleton anti-patterns  
- **376+ automated tests** ensuring reliability and maintainability
- **Cross-platform compatibility** (Windows, Linux, macOS)
- **Multiple UI frameworks** supported through clean abstractions
- **Continuous refactoring** - ongoing modernization and architectural improvements

### Technical Complexity
- **✨ Complex domain modeling**: poker rules, betting rounds, hand evaluation
- **🧠 AI strategy framework**: pluggable bot behaviors with range estimation
- **🔄 State machine design**: robust game flow management
- **📊 Statistical analysis**: player behavior tracking and persistence

---
## 🚀 Quick Start

### Prerequisites
- CMake 3.20+
- C++20 compatible compiler
- Qt 6.x (optional, only for GUI)

### Build & Test (unit tests + end-to-end tests)
```bash
git clone https://github.com/Marcennaji/HoldemCore.git
cd HoldemCore
cmake --preset debug-widgets
cmake --build build/debug-widgets
.\build\debug-widgets\tests\unit_tests.exe
.\build\debug-widgets\tests\e2e_tests.exe
```

---

## 👨‍💻 About the Engineer

**Marc Ennaji** 
Freelance Software Engineer | C++ Expert | Experienced Python Developer | Software Architecture & Refactoring  
[LinkedIn](https://www.linkedin.com/in/marcennaji/) 

### Core Expertise
- Modern C++ 
- Software Architecture 
- Legacy Modernization 
- Cross-Platform Development 
- Test-Driven Development 

� **Currently available for freelance collaborations and full-time positions**

---

## 🏠 Architecture Overview

### Core Engineering Principles

**1. Hexagonal Architecture (Ports & Adapters)**

![Hexagonal Architecture Diagram](doc/architecture.png)

*Hexagonal architecture with dependency injection - visual representation of component interactions and service flow*


**2. Dependency Injection Container**
- Zero singleton patterns
- Testable service boundaries  
- Configurable service implementations
- Mock-friendly architecture

**3. Domain-Rich Architecture**
- Rich domain models (Player, Hand, Board, Strategy)
- Complex business logic encapsulation
- Event-driven communication via function callbacks
- Clear separation between domain and infrastructure

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

**Engineering Benefits:**
- ⚙️ **Runtime strategy switching** based on game conditions
- 🧪 **Isolated unit testing** of individual strategies  
- 🔌 **Plugin architecture** for easy strategy development
- 📈 **Template method pattern** with shared base logic

### 5. **Advanced Game Logic**
- **Range estimation algorithms** for opponent modeling
- **Monte Carlo simulations** for equity calculations  
- **Statistical analysis** with persistent player profiles
- **Complex state management** across betting rounds
- Statistics persistence and range pruning
- **FSM for betting rounds**: Preflop → Flop → Turn → River → PostRiver

### 6. **Comprehensive Testing Strategy**
- **376+ automated tests** (unit + integration + end-to-end)
- **TDD approach** with GoogleTest framework
- **Architecture compliance tests** validating hexagonal principles
- **Mock services** for isolated component testing
- **Event-driven testing** with MockUI validation
- **Performance benchmarks** for critical algorithms

---

## 🔧 Technical Stack

**Core Technologies:**
- **C++20** (modern features, concepts, ranges, coroutines)
- **CMake** (cross-platform build system with presets)
- **GoogleTest** (comprehensive testing framework)

**Architecture Patterns:**
- **Hexagonal Architecture** (ports & adapters)
- **Dependency Injection** (service container pattern)
- **Strategy Pattern** (pluggable AI behaviors)
- **State Machine** (game flow management)
- **Event-driven Architecture** (callback-based communication)

**Cross-Platform Support:**
- **Qt 6** (desktop UI framework)
- **SQLite** (embedded database)
- **Standard C++** (portable business logic)

**Development Practices:**
- **Clean Code** principles
- **SOLID** design principles  
- **Test-Driven Development**
- **Continuous Integration** ready

---
## � Development Roadmap

**Architecture (Ongoing)**
- Still some important refactoring ongoing

**UI & Integration (Q4 2025)**
- Complete Qt Widgets table interface
- WebSocket API for remote clients
- REST API for game state queries

**Advanced Features (2026)**
- Flutter mobile/web clients
- Machine learning integration for advanced bot strategies

---

## 📈 Project Metrics

- **📝 19,000+ lines** of modern C++ code
- **⚙️ 376+ tests** with high coverage
- **📚 50+ classes** with clear responsibilities
- **� Zero singleton** anti-patterns
- **✅ 100% dependency injection** compliance
- **🎯 Multi-platform** compatibility

---

## 📚 Dependencies & Licensing

**Third-Party Components:**
- [psim](https://github.com/christophschmalhofer/poker/tree/master/XPokerEval/XPokerEval.PokerSim) - Hand evaluation engine
- [SQLite3](https://www.sqlite.org/) - Embedded database  
- [GoogleTest](https://github.com/google/googletest) - Testing framework

**Licensing:** MIT License - Free for commercial and personal use
