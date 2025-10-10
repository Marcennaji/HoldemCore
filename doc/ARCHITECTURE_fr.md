# 🏠 Vue d’ensemble de l’architecture

## 1. Principes architecturaux

### Architecture hexagonale (Ports & Adaptateurs)
![Diagramme d'architecture hexagonale](architecture.png)

**Idées clés :**
- Conception centrée sur le domaine — la logique du moteur est indépendante de l’interface utilisateur et de l’infrastructure
- Inversion des dépendances — les interfaces résident dans le cœur, les implémentations dans les adaptateurs
- Injection de dépendances — permet des frontières testables et un couplage faible
- Communication événementielle entre les composants
- Couches propres et claires : core/, infra/, ui/, app/, tests/

### Bénéfices
- Modules isolés et facilement testables
- Noyau de domaine réutilisable pour plusieurs interfaces
- Extension aisée avec de nouveaux adaptateurs (ex. REST, QML, CLI)

## 2. Modèle de domaine
### Sous-systèmes principaux
**Module	Responsabilité**
engine/	    Orchestration du jeu, contrôle de la machine à états (FSM)
hand/	    Cycle de vie et statistiques d’une main
game/	    Plateau, distribution du pot, logique de mise
player/	    Entités joueur, stratégies et statistiques
interfaces/	Contrats de service abstraits (journalisation, générateur aléatoire, persistance)

### 3. Design pattern Strategy — Comportement des joueurs IA

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

## 5. Règles architecturales actuelles
- Le cœur doit rester indépendant de toute UI (Qt ou autre)
- Respecter le principe de ségrégation des interfaces (ISP)
- Maintenir des frontières hexagonales strictes :
   - Logique du cœur → aucune connaissance de l’infrastructure
   - Couches UI → communication uniquement via ponts et événements

## 6. Stratégie de test
- Tests unitaires : GoogleTest, couverture des principaux composants du domaine
- Tests d’intégration / de bout en bout (E2E) : validation de la FSM, des tours de mise, de la persistance des statistiques

## 7. Structure de référence
HoldemCore/
├── src/
│   ├── core/          # Logique métier principale
│   ├── infra/         # Adaptateurs d’infrastructure (BDD, moteur d’évaluation, logs)
│   ├── ui/            # Interfaces utilisateur (Qt Widgets, QML...)
│   └── app/           # Points d’entrée applicatifs
└── tests/             # Tests unitaires et E2E
