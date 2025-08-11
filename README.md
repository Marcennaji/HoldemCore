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

## 🧱 Architecture Highlights

### 1. **Decoupled C++ Engine** (`src/core/`)
- **Fully UI-independent** 
- Manages game state, hand resolution, betting rounds, player actions
- Event-driven via plain `GameEvents` callback struct

### 2. **Multiple Frontends**
- ✅ Qt Widgets UI (in progress)
- 🟡 Flutter/Web UI (planned)
- ⚪ CLI mode for headless or manual human input (currently working)

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
- Ongoing refactor to model hand progress:
  - Preflop → Flop → Turn → River → PostRiver
- Ensures predictable and maintainable game flow

### 6. **Testing Infrastructure**
- GoogleTest-based unit tests in `tests/`

---

## 🔧 Technologies Used
- **C++17**
- **Qt Widgets** (current UI)
- **GoogleTest**
- **uWebSockets** (planned WebSocket server)
- **Flutter** (planned UI)
- **SQLite3**

---

## 🚀 Current Status — *Unstable, redesign of core engine using an FSM is on active development*
Originally created by Marc Ennaji in 2011 (SourceForge), migrated to GitHub in 2025.  
Currently in **major architectural rewrite** to improve decoupling, modularity, maintainability, and testability.

✅ Core engine is now headless  
✅ `GameEvents` decouples UI from logic  
✅ Minimalistic `BotStrategy` API with overridable defaults  
✅ CLI-based manual play for human player  
✅ FSM-based street handling in progress  

---

## 📋 Roadmap

**Short-term**
- Complete FSM-based hand street transitions
- Rewrite Qt Widgets table UI with modern layout
- Refactor some classes to improve design

**Medium-term**
- Implement WebSocket server & JSON protocol
- Build Flutter UIs for Android and Web

---

## 📁 Project Structure (Simplified)
```
src/
├── core/             # Engine logic
│   ├── engine/       # Hand, board, betting, evaluator
│   ├── events/       # GameEvents definition
│   ├── player/       # Player, BotStrategy
│   ├── session/      # Session & game management
├── ui/
│   └── qtwidgets/    # Qt Widgets UI
├── server/           # WebSocket server (planned)
├── tests/            # Unit tests
└── third_party/      # External dependencies
```

---

## 💻 Building & Running

### Requirements
- CMake 3.15+
- C++17 compiler
- Qt6 (only if building the qt UI)
- (Optional) GoogleTest for running tests

### Build Instructions
```bash
git clone https://github.com/Marcennaji/PokerTraining.git
cd PokerTraining
mkdir build && cd build
cmake ..
make
```

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
MIT — free to use, learn from, and contribute to.
