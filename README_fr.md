# HoldemCore

**🌐 Language / Langue:** [English](README.md) | [Français](README_fr.md)

**Projet Portfolio : Architecture C++ Moderne**

Moteur de simulation Texas Hold'em mettant en application des pratiques d’ingénierie logicielle modernes : architecture hexagonale, TDD et CI/CD, principes SOLID et conception multiplateforme.

> **Note Portfolio** : Ce dépôt est maintenu comme une démonstration d'ingénierie C++. Les tickets et PRs sont les bienvenus, mais la feuille de route suit les priorités de l'auteur.

---

## 🎯 Vitrine du Projet

Moteur de simulation Texas Hold'em démontrant une architecture logicielle avancée :

- **Architecture Hexagonale** avec séparation stricte des responsabilités
- **Injection de Dépendances** 
- **Suite de tests complète** garantissant la fiabilité
- **Compatibilité multiplateforme** (Windows, Linux, macOS)
- **Multiples frameworks UI** supportés via des abstractions propres

### Fonctionnalités Clés
- **Modélisation de domaine complexe** : règles de poker, tours d'enchères, évaluation des ranges
- **Framework de stratégies IA** : comportements de bots modulaires avec analyse statistique
- **Machine à états** : gestion robuste du flux de jeu
- **Suivi du comportement** des joueurs avec persistance en base

---
## 🚀 Démarrage Rapide

### Prérequis
- CMake 3.20+
- Compilateur compatible C++20
- Qt 6.x (optionnel, uniquement pour l'interface graphique)

### Construction & Tests
```bash
git clone https://github.com/Marcennaji/HoldemCore.git
cd HoldemCore
cmake --preset debug-widgets
cmake --build build/debug-widgets
.\build\debug-widgets\tests\unit_tests.exe
.\build\debug-widgets\tests\e2e_tests.exe
```

---

## 👨‍💻 À Propos

**Marc Ennaji** - Ingénieur Logiciel Senior  
[LinkedIn](https://www.linkedin.com/in/marcennaji/) 

### Expertise
- Développement C++ Moderne
- Architecture & Conception Logicielle
- Modernisation de Systèmes Legacy
- Développement Dirigé par les Tests

*Disponible pour collaborations et opportunités*

---

## 🏠 Vue d'Ensemble de l'Architecture

### Principes d'Architecture

**Architecture Hexagonale (Ports & Adaptateurs)**

![Diagramme d'Architecture Hexagonale](doc/architecture.png)

**Éléments de Conception Clés :**
- Injection de dépendances pour des frontières testables
- Modèles de domaine riches avec séparation claire
- Communication événementielle
- Pattern Strategy pour des comportements IA modulaires

### 4. **Implémentation du Pattern Strategy**

Démontre une **conception OOP avancée** avec des comportements IA modulaires :

```cpp
class BotStrategy {
public:
    virtual PlayerAction decidePreflop(const CurrentHandContext& ctx) = 0;
    virtual PlayerAction decideFlop(const CurrentHandContext& ctx) = 0;
    virtual PlayerAction decideTurn(const CurrentHandContext& ctx) = 0;
    virtual PlayerAction decideRiver(const CurrentHandContext& ctx) = 0;
};

// Composition propre plutôt qu'héritage
Player player(events, "BotName");
player.setStrategy(std::make_unique<TightAggressiveStrategy>());
```

**Avantages :**
- Changement de stratégie à l'exécution
- Tests unitaires isolés
- Architecture de plugins pour l'extensibilité

### 5. **Fonctionnalités du Moteur**
- **Estimation de ranges** pour la modélisation d'adversaires
- **Simulations Monte Carlo** pour les calculs d'equity  
- **Analyse statistique** avec profils de joueurs persistants
- **Machine à états** gérant les tours d'enchères (Preflop → Flop → Turn → River)

### 6. **Stratégie de Tests**
- **Tests automatisés complets** (unitaires + intégration + end-to-end)
- **Approche TDD** avec le framework GoogleTest
- **Services mock** pour les tests de composants isolés

---
## 🎮 Interface Utilisateur

<table>
  <tr>
    <td width="50%">
      <img src="doc/start%20game.png" alt="Démarrer une nouvelle partie" width="100%">
      <p align="center"><em>Fenêtre de Configuration de Partie</em></p>
    </td>
    <td width="50%">
      <img src="doc/poker%20table.png" alt="Interface de Table de Poker" width="100%">
      <p align="center"><em>Interface de Table de Poker</em></p>
    </td>
  </tr>
</table>

*Version Qt Widgets (bientôt disponible : version Qt Quick/QML)*


## 🔧 Stack Technique

**Technologies Centrales :**
- **C++20** avec fonctionnalités modernes
- **CMake** pour builds multiplateformes
- **GoogleTest** framework de tests

**Patterns d'Architecture :**
- Architecture Hexagonale, Injection de Dépendances
- Pattern Strategy, Machine à États
- Architecture Événementielle

**Support Multiplateforme :**
- **Qt 6** (Widgets et QML)
- **SQLite** base de données embarquée
- **C++ standard** multiplateforme


---
## 🚧 Feuille de Route de Développement

- Développer une interface graphique Qt Quick (QML), pour version mobile
- Explorer la possibilité d'intégrer une IA de bot basée sur le machine learning

---

## 📈 Échelle du Projet

**19 000+ lignes** de C++ moderne • **Couverture de tests complète** • **50+ classes**

---

## 📚 Dépendances & Licences

**Framework UI (Optionnel) :**
- [Qt 6](https://www.qt.io/) - Framework d'application multiplateforme (Qt Widgets et Qt QML)
  
  *Note : Le moteur central est découplé de l’interface utilisateur. Qt n'est requis que pour l'interface Widgets fournie. Les développeurs peuvent intégrer n'importe quel framework UI grâce à l'architecture hexagonale.*

**Composants Tiers (facilement remplaçables si nécessaire) :**
- [psim](https://github.com/christophschmalhofer/poker/tree/master/XPokerEval/XPokerEval.PokerSim) - Moteur d'évaluation de mains
- [SQLite3](https://www.sqlite.org/) - Base de données embarquée  
- [GoogleTest](https://github.com/google/googletest) - Framework de tests

**Licence :** Licence MIT - Gratuit pour usage commercial et personnel