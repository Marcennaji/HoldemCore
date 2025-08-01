# PokerTraining

**Texas Hold'em Poker Training Simulator**  
Cross-platform, event-driven C++ engine with bot opponents and 1 human player (optional), customizable bot strategies, and frontend-agnostic architecture.

---

## 🎯 Project Goal

PokerTraining is an offline, open-source poker simulation tool designed to help players practice against a variety of opponents.

- Train against tight, loose, aggressive, or random bots  
- Customize your poker table: number of players, stack size, small blind value, opponent profiles  
- Run entirely offline (desktop or mobile)

---

## 🧱 Architecture Highlights

### 1. **Decoupled C++ Engine** (`src/core/`)

- Fully independent of UI (no Qt dependency)
- Handles game state, hand resolution, betting rounds
- Event-driven via a plain `GameEvents` callback struct

### 2. **Multiple Frontends (Planned)**

- ✅ Qt Widgets UI (in progress)
- 🟡 Flutter/Web frontend (planned)

### 3. **Bot Strategies with Dependency Injection**

- Clean separation of `BotPlayer` and `IBotStrategy`
- Includes: `TightAggressive`, `LooseAggressive`, `Maniac`, `UltraTight`
- Strategies are testable via `CurrentHandContext`
- Bots may dynamically switch strategies mid-game based on context (e.g., stack size)
- Easy to create custom bots implementing the simple `IBotStrategy` interface

### 4. **Range Management & Equity Evaluation**

- Bot players estimate opponent ranges based on current hand actions and historical statistics (stored in DB)
- Range pruning is based only on public information (no cheating!)
- Sophisticated preflop call/raise logic using multiple parameters

### 5. **Testing Infrastructure**

- GoogleTest-based unit tests located in the `tests/` directory

---

## 🔧 Technologies Used

- C++17
- Qt Widgets (for current UI)
- GoogleTest
- uWebSockets (planned WebSocket server)
- Flutter (planned UI)

---

## 🚧 Current Status : under active refactoring

This project was originally created and published on SourceForge by Marc Ennaji in 2011. It has been migrated on Github during 2025.
The current version is a complete architectural rewrite, for a better decoupling, modularity, maintainability, testability, and cross-platform UI support. It is currently under **active refactoring**, with major milestones already completed:

✅ Engine is now headless  
✅ `GameEvents` emit UI updates without any UI dependency  
✅ Strategies refactored and unit-tested  
✅ Engine is ready to be wrapped in a WebSocket server for multi-platform frontends  

🔜 Next steps:
- Ongoing : completely redesign the transition between a hand's streets (Preflop->Flop->Turn->River->PostRiver) using a Finished State Machine (FSM)
- Completely rewrite the Qt Widgets poker table UI for a clean and modern layout
- Build JSON-based WebSocket protocol  
- Develop Flutter UI for Android/Web  
- Reach an 'A' CppDepend rating and high SonarQube score  

---

## 📁 Project Structure (Simplified)

```
src/
├── core/             # Engine logic
│   ├── engine/       # Hand, board, betting, evaluator
│   ├── events/       # GameEvents definition
│   ├── player/       # Player, BotPlayer, strategies
│   ├── session/      # Session & Game management
├── ui/
│   └── qtwidgets/    # Qt Widgets UI (views, controllers)
├── server/           # WebSocket server (planned)
├── tests/            # GoogleTest tests
└── third_party/      # hand simulation, SQLite3, etc.
```

---

## 📚 Third-Party Components

This project uses or integrates the following third-party libraries:

- **[psim](https://github.com/christophschmalhofer/poker/tree/master/XPokerEval/XPokerEval.PokerSim)** — A fast poker hand simulator and evaluator for hand ranking and simulations  
- **[poker-eval](https://github.com/atinm/poker-eval)** — A C library for poker hand evaluation originally developed by Andrew Prock  
- **[SQLite3](https://www.sqlite.org/index.html)** — A self-contained, high-reliability embedded SQL database engine for persistent data storage  

> All third-party libraries are used in accordance with their respective open-source licenses.  
> See the `third_party/` directory for license information.

---

## 🙋 Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md)

---

## 📝 License

MIT — free to use, learn from, and contribute to.
