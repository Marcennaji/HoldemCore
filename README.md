# PokerTraining

**Texas Hold'em Poker Training Simulator**
Cross-platform event-driven C++ engine with bot opponents, customizable strategies, and frontend-agnostic architecture.

---

## 🎯 Project Goal

PokerTraining is an offline, open-source poker simulation tool designed to help players practice against various types of opponents.

* Train against tight, loose, aggressive, or random bots
* Customize your poker table : heads-up to 10 players, stack size, small blind value, opponents profiles...
* Run entirely offline (desktop or mobile)

---

## 🧱 Architecture Highlights

### 1. **Decoupled C++ Engine** (`src/core/`)

* Fully independent of UI (no Qt dependency)
* Handles game state, hand resolution, betting rounds
* Event-driven via a plain `GameEvents` callback struct

### 2. **Multiple Frontends (Planned)**

* ✅ Qt Widgets UI (in progress)
* 🟡 Flutter/Web frontend (in progress)

### 3. **Bot Strategies with Dependency Injection**

* Clean separation of `BotPlayer` and `IBotStrategy`
* Includes: TightAggressive, LooseAggressive, Maniac, UltraTight
* Testable via `CurrentHandContext`
* Bots might switch dynamically their strategies in a given game, depending on the context (stack size, etc)
* Easy to add your own poker bot, that would implement the simple IBotStrategy interface

### 4. **Range Management & Equity Evaluation**

* Bot players estimates opponents ranges, based on the current hand actions and the historical stats (stored in DB)
* Opponents range pruning is based only on public signals and historical stats (fair game guaranteed !)
* Sophisticated preflop call/raise range calculation, based on numerous parameters

### 5. **Testing Infrastructure**

* GoogleTest-based unit tests under `tests` directory

---

## 🔧 Technologies Used

* C++17
* Qt Widgets (for current UI)
* GoogleTest
* uWebSockets (planned WebSocket server)
* Flutter (planned UI)

---

## 🚧 Current Status

I published the first version of this project 15 years ago, on SourceForge.
This project is today under **active refactoring**, with major milestones completed:

✅ Engine now headless  
✅ `GameEvents` emit UI updates without UI dependency  
✅ Strategies refactored and unit-tested  
✅ Player creation uses a `PlayerFactory` and `StrategyAssigner`  
✅ Ready to be wrapped in WebSocket server for cross-platform frontends  


🔜 Next steps:

* Build JSON-based WebSocket protocol
* Develop Flutter UI for Android/Web
* Reach a 'A' Cppdepend overall rating, as well as a high scoring SonarQube

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
└── third_party/      # hand simulation, sqlite3, etc.
```

### 📚 Third-Party Components

This project uses or integrates the following third-party libraries:

- **[psim](https://github.com/christophschmalhofer/poker/tree/master/XPokerEval/XPokerEval.PokerSim)** — A fast poker hand simulator and evaluator, used for hand ranking and simulation.  
- **[poker-eval](https://github.com/atinm/poker-eval)** — A C library for poker hand evaluation, originally developed by Andrew Prock.  
- **[SQLite3](https://www.sqlite.org/index.html)** — A self-contained, high-reliability embedded SQL database engine, used for persistent data storage.

> All third-party libraries are included in accordance with their respective open-source licenses.  
> Please refer to the `third_party/` directory for license information where applicable.

## 🙋 Contributing

See `[CONTRIBUTING.md](CONTRIBUTING.md)`

---

## 📝 License

MIT — free for use, learning, and contribution.
