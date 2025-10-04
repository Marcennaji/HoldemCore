## 🏠 Architecture Overview

### Architecture Principles

**Hexagonal Architecture (Ports & Adapters)**

![Hexagonal Architecture Diagram](architecture.png)

**Key Design Elements:**
- Dependency injection for testable boundaries
- Rich domain models with clear separation
- Event-driven communication
- Strategy pattern for pluggable AI behaviors

**Strategy Pattern Implementation**

Pluggable AI behaviors:

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

**Benefits:**
- Runtime strategy switching
- Isolated unit testing
- Plugin architecture for extensibility



---