# 🏠 Architecture Overview

## 1. Architectural Principles

### Hexagonal Architecture (Ports & Adapters)

![Hexagonal Architecture Diagram](architecture.png)

**Core ideas:**
- **Domain-centric** design — engine logic independent of UI or infrastructure
- **Dependency inversion** — ports (interfaces) in core, adapters (implementations) in infrastructure
- **Dependency injection** for testable boundaries
- **Event-driven** communication between components
- **Clean layering:** `core/`, `adapters/`, `ui/`, `app/`, `tests/`

### Layer Dependencies (Dependency Direction)

```
                    ┌─────────────────┐
                    │       App       │ ← Composition Root
                    │   (AppFactory)  │   (orchestrates all)
                    └─────────────────┘
                            │
                    ┌───────┼───────┐
                    ↓       ↓       ↓
            ┌───────────┐   │   ┌───────────┐
            │    UI     │   │   │ Adapters  │
            │ (Widgets) │   │   │  (Infra)  │
            └───────────┘   │   └───────────┘
                    │       │       │
                    └───────┼───────┘
                            ↓
                    ┌───────────────┐
                    │     Core      │ ← Stable Center
                    │   (Domain)    │   • Business Logic
                    │               │   • Ports
                    │  ┌─────────┐  │   • No Dependencies
                    │  │  Ports  │  │
                    │  │(Interf.)│  │
                    │  └─────────┘  │
                    └───────────────┘
```

**Dependency Flow:**
- `App` → depends on → `UI + Core + Adapters` (orchestrates everything)
- `UI` → depends on → `Core` (uses ports)
- `Adapters` → depends on → `Core` (implements ports)  
- `Core` → depends on → **NOTHING** (pure domain)

**Key Rule:** All arrows point **toward Core** - it's the stable foundation.

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
| `ports/` | Abstract service contracts (logging, RNG, persistence) |

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

## 5. Composition Root Pattern — AppFactory

### Dependency Inversion Implementation

The `AppFactory` serves as the **composition root** for the entire application, implementing proper dependency inversion:

```cpp
// src/app/AppFactory.h
class AppFactory {
public:
    // Configuration-based creation
    static std::unique_ptr<GuiAppController> createApplication(
        LoggerType loggerType = LoggerType::Console,
        HandEvaluatorType evaluatorType = HandEvaluatorType::Psim,
        DatabaseType dbType = DatabaseType::Sqlite
    );
    
    // Environment-specific shortcuts
    static std::unique_ptr<GuiAppController> createTestApp();
    ...
};
```

### Clean Main Application

```cpp
// src/app/main_qt_widgets.cpp - No concrete dependencies!
int main(int argc, char** argv) {
    QApplication app(argc, argv);
    
    // Single point of configuration
    auto controller = pkt::app::AppFactory::createApplication(
        pkt::app::LoggerType::Console,
        pkt::app::HandEvaluatorType::Psim,
        pkt::app::DatabaseType::Sqlite
    );
    
    auto* mainWindow = controller->createMainWindow();
    mainWindow->show();
    return app.exec();
}
```

### Benefits of AppFactory Pattern

- ✅ **Dependency Inversion:** High-level modules don't depend on low-level modules
- ✅ **Single Responsibility:** `main()` focuses on application lifecycle, not object construction
- ✅ **Open/Closed:** Easy to add new implementations without changing existing code
- ✅ **Configuration Flexibility:** Switch implementations via simple enum changes
- ✅ **Testability:** Easy to inject mock implementations for testing

### Adding New Implementations

To add a new database (e.g., PostgreSQL):

1. **Create the implementation:** `PostgreSqlPlayersStatisticsStore`
2. **Add enum value:** `DatabaseType::PostgreSql`
3. **Add factory case:** One switch statement in `AppFactory::createStatisticsStore()`
4. **Update main:** Change `DatabaseType::Sqlite` to `DatabaseType::PostgreSql`

**Zero changes needed** in core business logic, UI, or tests!

---

## 6. Current Architectural Rules

- Core must remain **UI-free** (Qt, etc)
- Maintain **hexagonal boundaries** strictly:
  - Core logic → no infrastructure knowledge  
  - UI layers → communicate only via bridges and events
- **Composition Root:** All concrete dependencies instantiated only in `AppFactory`
- **Dependency Inversion:** High-level modules depend only on abstractions

---

## 7. Testing Strategy

- **Unit Tests:** GoogleTest, main core components are covered  
- **Integration / E2E:** validates FSM, betting rounds, statistics persistence  

---

## 8. Reference Layout

```
HoldemCore/
├── src/
│   ├── core/                  # Domain logic (business rules, ports)
│   │   ├── engine/           # Game engine and FSM
│   │   ├── player/           # Player entities and strategies
│   │   ├── session/          # Session management
│   │   └── ports/            # Abstract interfaces (Logger, Randomizer, etc.)
│   ├── adapters/             # Infrastructure adapters
│   │   └── infrastructure/   # Concrete implementations
│   │       ├── logger/       # ConsoleLogger, NullLogger
│   │       ├── hand_evaluation/  # PsimHandEvaluationEngine
│   │       ├── statistics/   # Statistics storage
│   │       │   └── sqlite/   # SQLite implementation
│   │       └── randomizer/   # DefaultRandomizer
│   ├── ui/                   # UI front-ends (Qt Widgets, QML...)
│   └── app/                  # Application layer (AppFactory composition root)
│       ├── AppFactory.h      # Dependency injection factory
│       ├── AppFactory.cpp    # Concrete instantiation logic
│       └── main_*.cpp        # Clean application entry points
└── tests/                    # Unit and E2E tests
```




