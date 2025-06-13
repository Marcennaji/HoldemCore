# PokerTraining

**Texas Hold'em Poker Training Simulator**
Cross-platform event-driven C++ engine with bot opponents, customizable strategies, and frontend-agnostic architecture.

---

## 🎯 Project Goal

PokerTraining is an offline, open-source poker simulation tool designed to help players practice against various types of opponents.

* Train against tight, loose, aggressive, or random bots
* Analyze hand strength and opponent behavior
* Run entirely offline (desktop or mobile)

---

## 🧱 Architecture Highlights

### 1. **Decoupled C++ Engine** (`src/core/`)

* Fully independent of UI (no Qt dependency)
* Handles game state, hand resolution, betting rounds
* Event-driven via a plain `GameEvents` callback struct

### 2. **Multiple Frontends (Planned)**

* ✅ Qt Widgets UI (existing)
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

* GoogleTest-based unit tests under `tests/core/...`
* Focus on verifying strategy behavior, ranges calculation & decision logic

---

## 🔧 Technologies Used

* C++17
* Qt Widgets (for current UI)
* GoogleTest
* uWebSockets (planned WebSocket server)
* Flutter (planned UI)

---

## 🚧 Current Status

This project is under **active refactoring**, with major milestones completed:

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

## 🙋 About Me

I'm a senior freelance C++ / Python developer, and this project is both a portfolio piece and a real-world training tool.

It demonstrates:

* Domain-driven design in games
* Code decoupling and modularization, applying the SOLID and clean architecture principles
* Clean testing practices
* Cross-platform architecture thinking

📫 Feel free to reach out or contribute if you're interested in poker strategy simulation, bots, or frontends.

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

---

## 📝 License

MIT — free for use, learning, and contribution.
