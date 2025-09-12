# PokerTraining

**Texas Hold'em Poker Training Simulator**  
Cross-platform, event-driven C++ engine with bot opponents and optional human player, customizable bot strategies, and frontend-agnostic architecture.

---

## 🎯 Project Goal

PokerTraining is an offline, open-source poker simulation tool designed to help players practice against a variety of opponents.  
It is also designed for **bot developers** to easily implement and test custom poker strategies, focusing purely on poker logic without dealing with the underlying mechanics of game management.

- Train against tight, loose, aggressive, or random bots — or implement your own  
- Customize table parameters: number of players, stack size, small blind, opponent profiles  
- Run entirely offline (desktop, mobile, or CLI)

---
## 🚀 Quick Start

### Prerequisites
- CMake 3.20+
- C++20 compatible compiler
- Qt 6.x (optional, only for GUI)

### Build & Test
```bash
git clone https://github.com/Marcennaji/PokerTraining.git
cd PokerTraining
cmake --preset debug-widgets
cmake --build build/debug-widgets

# Run tests to verify the build
.\build\debug-widgets\tests\unit_tests.exe
.\build\debug-widgets\tests\e2e_tests.exe
```

---

## 👨‍💻 About the Author

Maintained by **Marc Ennaji**, freelance **senior software engineer**  
[LinkedIn profile](https://www.linkedin.com/in/marcennaji/)

- Expertise in **cross-platform development** (C++, Python)  
- Specialized in **refactoring legacy code** into **modern, testable architectures**  
- Focused on **clean, modular design** and maintainability  

📫 Available for freelance collaborations — feel free to reach out!

---

## 🧱 Architecture Highlights

### 1. **Decoupled C++ Engine** (`src/core/`)
- **Fully UI-independent**, thanks to a hexagonal architecture (ports & adapters)
- Manages game state, hand resolution, betting rounds, player actions
- Event-driven via plain `GameEvents` callback struct

### 2. **Multiple Frontends**
- ✅ Qt Widgets UI (in progress)
- 🟡 Flutter/Web UI (planned)

### 3. **Bot Strategies**
- Clear separation of `Player` and `BotStrategy`
- Minimal `BotStrategy` interface (4 template methods: `decidePreflop`, `decideFlop`, `decideTurn`, `decideRiver`)
- Built-in bots:
  - Tight Aggressive
  - Loose Aggressive
  - Maniac
  - Ultra Tight
- If needed, built-in bots' shared logic can be reused by bot developers, by deriving a new strategy from `BotStrategyBase` (instead of directly from `BotStrategy`).  
- Strategies can switch mid-game (e.g., stack size changes)
- Testable in isolation using `CurrentHandContext`

**Example custom bot:**

```cpp
class MyCustomBotStrategy : public BotStrategy {
public:
    MyCustomBotStrategy() : BotStrategy() {}

    PlayerAction decidePreflop(const CurrentHandContext& ctx) override {
        // Custom logic here
    }
    // ... same for decideFlop, decideTurn, decideRiver
};
```

Then:

```cpp
Player player(events, playerName /*, other params if any */); 
player.setStrategy(std::make_unique<MyCustomBotStrategy>());

```

### 4. **Range Management & Equity Evaluation**
- Bots estimate opponent ranges using only public information
- Range pruning based on table actions & statistics (SQLite persistence)
- Flexible preflop/streets decision logic

### 5. **Finite State Machine for Streets**
- FSM model hand progress:
  - Preflop → Flop → Turn → River → PostRiver
- Ensures predictable and maintainable game flow

### 6. **Testing Infrastructure**
- GoogleTest-based unit tests in `tests/`

---

## 🔧 Technologies Used
- **C++20**
- **Qt Widgets** (current UI)
- **GoogleTest**
- **uWebSockets** (planned WebSocket server)
- **Flutter** (planned UI)
- **SQLite3**

---
## 🕰 Project History and Current Status — *Ongoing refactoring with FSM-based core redesign*

- **2011** — Initial version published on SourceForge (5⭐ project, >11,000 downloads).  
  The original version reused portions of the open-source **PokerTH** (GPLv2) engine, with a focus on creating **poker bot strategies and AI logic** (the existing PokerTH builtin bot AI was not reused at all).  

- **2025 (today)** — Migrated to GitHub and undergoing a **major architectural rewrite**.  
  The legacy PokerTH code (layered architecture, using Boost C++ library) has been **progressively refactored and replaced** with a new **FSM-driven, modular architecture** (hexagonal architecture, pure standard C++), designed for **testability** and to support multiple UIs (Qt Widgets, QML, and web frontends).  

The ongoing refactor has replaced almost all legacy code. 

🚧 Since it is still under refactoring for the core engine, and since UI development is still on progress, no public release or announcement has been made yet.  

✅ Core engine is now headless  
✅ `GameEvents` decouples UI from logic  
✅ Minimalistic `BotStrategy` API with overridable defaults  
✅ FSM-based street handling has been achieved  


---

## 📋 Roadmap 

**Short-term**
- Enhance testing on the new FSM-based hand street transitions, and remove the remaining legacy code
- Redesign Qt Widgets table UI with a modern layout

**Medium-term**
- Implement WebSocket server & JSON protocol
- Build Flutter UIs for Android and Web

---

## 📚 Third-Party Components

- **[psim](https://github.com/christophschmalhofer/poker/tree/master/XPokerEval/XPokerEval.PokerSim)** — Poker hand simulator and evaluator  
- **[poker-eval](https://github.com/atinm/poker-eval)** — C library, used by psim for hand evaluation  
- **[SQLite3](https://www.sqlite.org/index.html)** — Embedded SQL database  

The core engine is decoupled from these components thanks to dependency injection, so they can be swapped or extended with minimal changes.

> See `third_party/` for licenses.

---

## 🤝 Contributing
See [CONTRIBUTING.md](CONTRIBUTING.md)

---

## 📝 License

This project, PokerTraining, is released under the MIT License —  
you are free to use, learn from, and contribute to it.

⚖️ Note on legacy code:
The initial version of PokerTraining (2011, SourceForge) reused portions of the open-source [PokerTH](https://github.com/pokerth/pokerth) engine, which at that time was distributed under **GPLv2**. That early codebase focused mainly on adding poker bot strategies and AI logic.  

Since 2025, the engine has been undergoing a **progressive rewrite**: nearly all PokerTH-derived code has been replaced with a **fully original, modular codebase under MIT**.  

🙏 Acknowledgment to the PokerTH project for inspiring the early stages of PokerTraining.
