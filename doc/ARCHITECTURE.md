# 🏠 Architecture Overview

## 1. Architectural Principles

### Hexagonal Architecture (Ports & Adapters)

![Hexagonal Architecture Diagram](architecture.png)

**Core ideas:**
- **Domain-centric** design — engine logic independent of UI or infrastructure
- **Dependency inversion** — interfaces in core, implementations in adapters
- **Dependency injection** for testable boundaries
- **Event-driven** communication between components
- **Clean layering:** `core/`, `infra/`, `ui/`, `app/`, `tests/`

### Benefits
- Isolated, testable modules
- Reusable domain core across multiple front-ends
- Easy extension with new adapters (e.g. REST, QML, CLI)

---

## 2. Domain Model

### Core Subsystems
| Module | Responsibility |
|---------|----------------|
| `engine/` | Game orchestration, FSM control |
| `hand/` | Lifecycle and statistics |
| `game/` | Board, pot distribution, betting logic |
| `player/` | Player entities, strategies, and statistics |
| `interfaces/` | Abstract service contracts (logging, RNG, persistence) |

---

## 3. Strategy Pattern — AI Player Behavior

Example of a pluggable strategy:

```cpp
class BotStrategy {
public:
    virtual PlayerAction decidePreflop(const CurrentHandContext& ctx) = 0;
    virtual PlayerAction decideFlop(const CurrentHandContext& ctx) = 0;
    virtual PlayerAction decideTurn(const CurrentHandContext& ctx) = 0;
    virtual PlayerAction decideRiver(const CurrentHandContext& ctx) = 0;
};

Player player(events, "BotName");
player.setStrategy(std::make_unique<TightAggressiveStrategy>());
```

**Advantages**
- Runtime strategy switching  
- Unit-testable decision logic  
- Extensible plugin framework  

---

## 4. Engine Design

### Finite State Machine
`Preflop → Flop → Turn → River → PostRiver`

Each state:
- Encapsulates its own decision logic
- Emits state transitions via `GameEvents`
- Keeps domain purity — no Qt dependencies

---

## 5. Current Architectural Rules

- Core must remain **UI-free** (Qt, etc)
- Maintain **hexagonal boundaries** strictly:
  - Core logic → no infrastructure knowledge  
  - UI layers → communicate only via bridges and events

---

## 6. Testing Strategy

- **Unit Tests:** GoogleTest, main core components are covered  
- **Integration / E2E:** validates FSM, betting rounds, statistics persistence  

---

## 7. Reference Layout

```
HoldemCore/
├── src/
│   ├── core/          # Domain logic
│   ├── infra/         # Infrastructure adapters (DB, eval engine, logging)
│   ├── ui/            # UI front-ends (Qt Widgets, QML...)
│   └── app/           # Application entrypoints
└── tests/             # Unit and E2E tests
```


