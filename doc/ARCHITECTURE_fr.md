## 🏠 Vue d'Ensemble de l'Architecture

### Principes d'Architecture

**Architecture Hexagonale (Ports & Adaptateurs)**

![Diagramme d'Architecture Hexagonale](architecture.png)

**Éléments de Conception Clés :**
- Injection de dépendances pour des frontières testables
- Modèles de domaine riches avec séparation claire
- Communication événementielle
- Pattern Strategy pour des comportements IA modulaires

**Implémentation du Pattern Strategy**

Comportements IA modulaires :

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

**Avantages :**
- Changement de stratégie à l'exécution
- Tests unitaires isolés
- Architecture de plugins pour l'extensibilité



---