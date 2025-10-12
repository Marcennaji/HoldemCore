# 🏠 Vue d’ensemble de l’architecture

## 1. Principes architecturaux

### Architecture hexagonale (Ports & Adapteurs)
![Diagramme d'architecture hexagonale](architecture.png)

**Idées clés :**
- Conception centrée sur le domaine — la logique du moteur est indépendante de l’interface utilisateur et de l’infrastructure
- Inversion des dépendances — les ports (interfaces) résident dans le cœur, les adaptateurs (implémentations) dans l'infrastructure
- Injection de dépendances — permet des frontières testables et un couplage faible
- Communication événementielle entre les composants
- Couches propres et claires : core/, adapters/, ui/, app/, tests/

### Bénéfices
- Modules isolés et facilement testables
- Noyau de domaine réutilisable pour plusieurs interfaces
- Extension aisée avec de nouveaux adaptateurs (ex. REST, QML, CLI)

### Dépendances entre couches (Direction des dépendances)

```
                    ┌─────────────────┐
                    │       App       │ ← Racine de composition
                    │   (AppFactory)  │   (orchestre tout)
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
                    │     Core      │ ← Centre stable
                    │   (Domaine)   │   • Logique métier
                    │               │   • Ports
                    │  ┌─────────┐  │   • Aucune dépendance
                    │  │  Ports  │  │
                    │  │(Interf.)│  │
                    │  └─────────┘  │
                    └───────────────┘
```

**Flux de dépendances :**
- `App` → dépend de → `UI + Core + Adapters` (orchestre tout)
- `UI` → dépend de → `Core` (utilise les ports)
- `Adapters` → dépend de → `Core` (implémente les ports)
- `Core` → dépend de → **RIEN** (domaine pur)

**Règle clé :** Toutes les flèches pointent **vers Core** - c'est la fondation stable.

## 2. Modèle de domaine

### Sous-systèmes principaux
| Module | Responsabilité |
|---------|----------------|
| `engine/` | Orchestration du jeu, contrôle de la machine à états (FSM) |
| `hand/` | Cycle de vie et statistiques d'une main |
| `game/` | Plateau, distribution du pot, logique de mise |
| `player/` | Entités joueur, stratégies et statistiques |
| `ports/` | Contrats de service abstraits (journalisation, générateur aléatoire, persistance) |

---

## 3. Design pattern Strategy — Comportement des joueurs IA

Exemple d’une stratégie interchangeable :
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

**Avantages**
- Changement de stratégie à l’exécution
- Logique décisionnelle testable unitairement
- Cadre extensible pour de nouvelles stratégies

## 4. Conception du moteur
### Machine à états finis
`Preflop → Flop → Turn → River → PostRiver`

Chaque état :
- Encapsule sa propre logique décisionnelle
- Émet des transitions via GameEvents
- Préserve la pureté du domaine — aucune dépendance à Qt

## 5. Pattern Composition Root — AppFactory

### Implémentation de l'inversion des dépendances

L'`AppFactory` sert de **racine de composition** pour toute l'application, implémentant une inversion de dépendances :

```cpp
// src/app/AppFactory.h
class AppFactory {
public:
    // Création basée sur la configuration
    static std::unique_ptr<GuiAppController> createApplication(
        LoggerType loggerType = LoggerType::Console,
        HandEvaluatorType evaluatorType = HandEvaluatorType::Psim,
        DatabaseType dbType = DatabaseType::Sqlite
    );
    
    // Raccourcis spécifiques à l'environnement
    static std::unique_ptr<GuiAppController> createTestApp();
    ...
};
```

### Application principale propre

```cpp
// src/app/main_qt_widgets.cpp - Aucune dépendance concrète 
int main(int argc, char** argv) {
    QApplication app(argc, argv);
    
    // Point unique de configuration
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

### Avantages du pattern AppFactory

- ✅ **Inversion des dépendances :** Les modules de haut niveau ne dépendent pas des modules de bas niveau
- ✅ **Responsabilité unique :** `main()` se concentre sur le cycle de vie de l'application, pas sur la construction d'objets
- ✅ **Ouvert/fermé :** Facile d'ajouter de nouvelles implémentations sans changer le code existant
- ✅ **Flexibilité de configuration :** Changer d'implémentations via de simples modifications d'énumérations
- ✅ **Testabilité :** Facile d'injecter des implémentations mockées pour les tests

### Ajout de nouvelles implémentations

Pour ajouter une nouvelle base de données (ex. PostgreSQL) :

1. **Créer l'implémentation :** `PostgreSqlPlayersStatisticsStore`
2. **Ajouter la valeur enum :** `DatabaseType::PostgreSql`
3. **Ajouter le cas factory :** Une instruction switch dans `AppFactory::createStatisticsStore()`
4. **Mettre à jour main :** Changer `DatabaseType::Sqlite` en `DatabaseType::PostgreSql`

**Aucun changement nécessaire** dans la logique métier core, l'UI, ou les tests.

---

## 6. Règles architecturales actuelles
- Le cœur doit rester indépendant de toute UI (Qt ou autre)
- Maintenir des frontières hexagonales strictes :
   - Logique du cœur → aucune connaissance de l'infrastructure
   - Couches UI → communication uniquement via ponts et événements
- **Racine de composition :** Toutes les dépendances concrètes instanciées uniquement dans `AppFactory`
- **Inversion des dépendances :** Les modules de haut niveau ne dépendent que d'abstractions

## 7. Stratégie de test
- Tests unitaires : GoogleTest, couverture des principaux composants du domaine
- Tests d’intégration / de bout en bout (E2E) : validation de la FSM, des tours de mise, de la persistance des statistiques

## 8. Structure de référence

```
HoldemCore/
├── src/
│   ├── core/                  # Logique métier (règles business, ports)
│   │   ├── engine/           # Moteur de jeu et FSM
│   │   ├── player/           # Entités joueur et stratégies
│   │   ├── session/          # Gestion de session
│   │   └── ports/            # Interfaces abstraites (Logger, Randomizer, etc.)
│   ├── adapters/             # Adaptateurs d'infrastructure
│   │   └── infrastructure/   # Implémentations concrètes
│   │       ├── logger/       # ConsoleLogger, NullLogger
│   │       ├── hand_evaluation/  # PsimHandEvaluationEngine
│   │       ├── statistics/   # Stockage des statistiques
│   │       │   └── sqlite/   # Implémentation SQLite
│   │       └── randomizer/   # DefaultRandomizer
│   ├── ui/                   # Interfaces utilisateur (Qt Widgets, QML...)
│   └── app/                  # Couche applicative (racine de composition AppFactory)
│       ├── AppFactory.h      # Factory d'injection de dépendances
│       ├── AppFactory.cpp    # Logique d'instanciation concrète
│       └── main_*.cpp        # Points d'entrée applicatifs propres
└── tests/                    # Tests unitaires et E2E
```


