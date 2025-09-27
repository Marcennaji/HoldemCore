# HoldemCore

**🌐 Language / Langue:** [English](README.md) | [Français](README_fr.md)

**Vitrine d'Architecture C++ de Niveau Production**  
Moteur de simulation de poker Texas Hold'em démontrant les pratiques modernes d'ingénierie logicielle : architecture hexagonale, injection de dépendances, tests exhaustifs et conception multiplateforme.

> **Note Portfolio** : Ce dépôt est maintenu comme une démonstration d'ingénierie C++ de niveau production. Les issues et PRs sont les bienvenues mais la feuille de route suit les priorités de l'auteur.

---

## 🎯 Vitrine d'Ingénierie

Ce projet démontre **l'ingénierie logicielle de niveau entreprise** à travers un moteur complet de simulation de poker :

- **Architecture Hexagonale** avec séparation stricte des préoccupations
- **Injection de Dépendances** 
- **~500 tests automatisés sur le moteur central**, garantissant fiabilité et maintenabilité
- **Compatibilité multiplateforme** (Windows, Linux, macOS)
- **Multiples frameworks UI** supportés via des abstractions propres
- **Refactoring continu** - modernisation et améliorations architecturales en cours

### Complexité Technique
- **Modélisation de domaine complexe** : règles de poker, tours d'enchères, réévaluation des ranges des joueurs à chaque action
- **Framework de stratégies IA** : comportements de bots modulaires avec estimation de ranges
- **Conception de machine à états** : gestion robuste du flux de jeu
- **Analyse statistique** : suivi et persistance du comportement des joueurs

---
## 🚀 Démarrage Rapide

### Prérequis
- CMake 3.20+
- Compilateur compatible C++20
- Qt 6.x (optionnel, uniquement pour l'interface graphique)

### Construire & Tester le moteur central (tests unitaires + tests end-to-end)
```bash
git clone https://github.com/Marcennaji/HoldemCore.git
cd HoldemCore
cmake --preset debug-widgets
cmake --build build/debug-widgets
.\build\debug-widgets\tests\unit_tests.exe
.\build\debug-widgets\tests\e2e_tests.exe
```

---

## 👨‍💻 À Propos de l'Ingénieur

**Marc Ennaji** 

Ingénieur Logiciel Senior | Spécialiste C++ | Développeur Python Expérimenté | Architecture Logicielle & Refactoring  
[LinkedIn](https://www.linkedin.com/in/marcennaji/) 

### Expertise Centrale
- C++ Moderne 
- Architecture Logicielle 
- Modernisation du Legacy 
- Développement Multiplateforme 
- Développement Dirigé par les Tests 

**Actuellement disponible pour collaborations freelance et postes à temps plein**

---

## 🏠 Vue d'Ensemble de l'Architecture

### Principes d'Ingénierie Fondamentaux

**1. Architecture Hexagonale (Ports & Adaptateurs)**

![Diagramme d'Architecture Hexagonale](doc/architecture.png)

*Architecture hexagonale avec injection de dépendances - représentation visuelle des interactions de composants et du flux de services*

**2. Conteneur d'Injection de Dépendances**
- Frontières de services testables  
- Implémentations de services configurables
- Architecture compatible avec les mocks

**3. Architecture Riche en Domaine**
- Modèles de domaine riches (Player, Hand, Board, Strategy)
- Encapsulation de logique métier complexe
- Communication événementielle via callbacks de fonctions
- Séparation claire entre domaine et infrastructure

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

**Bénéfices d'Ingénierie :**
- **Changement de stratégie à l'exécution** basé sur les conditions de jeu
- **Tests unitaires isolés** de stratégies individuelles  
- **Architecture de plugins** pour un développement facile de stratégies
- **Pattern de méthode template** avec logique de base partagée

### 5. **Logique de Jeu Avancée**
- **Algorithmes d'estimation de ranges** pour la modélisation d'adversaires
- **Simulations Monte Carlo** pour les calculs d'equity  
- **Analyse statistique** avec profils de joueurs persistants
- **Gestion d'état complexe** à travers les tours d'enchères
- **Persistance des statistiques et élagage de ranges**
- **FSM pour les tours d'enchères** : Preflop → Flop → Turn → River → PostRiver

### 6. **Stratégie de Tests Exhaustive**
- **~500 tests automatisés sur le moteur central** (unitaires + intégration + end-to-end)
- **Approche TDD** avec le framework GoogleTest
- **Tests de conformité architecturale** validant les principes hexagonaux
- **Services mock** pour les tests de composants isolés
- **Tests événementiels** avec validation MockUI

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
- **C++20** (fonctionnalités modernes, concepts, ranges, coroutines)
- **CMake** (système de build multiplateforme avec presets)
- **GoogleTest** (framework de tests exhaustif)

**Patterns d'Architecture :**
- **Architecture Hexagonale** (ports & adaptateurs)
- **Injection de Dépendances** 
- **Pattern Strategy** (comportements IA modulaires)
- **Machine à États** (gestion du flux de jeu)
- **Architecture Événementielle** (communication basée sur les callbacks)

**Support Multiplateforme :**
- **Qt 6** (Qt Widgets et Qt QML)
- **SQLite** (base de données embarquée)
- **C++ Standard** (logique métier portable)

**Pratiques de Développement :**
- Principes **Clean Code**
- Principes de conception **SOLID**  
- **Développement Dirigé par les Tests**
- Prêt pour **l'Intégration Continue**

---
## 🚧 Feuille de Route de Développement

- Développer une interface graphique Qt Quick (QML), pour version mobile
- Explorer la possibilité d'intégrer une IA de bot basée sur le machine learning

---

## 📈 Métriques du Projet

- **19 000+ lignes** de code C++ moderne
- **~500 tests** avec couverture élevée
- **50+ classes** avec responsabilités claires

---

## 📚 Dépendances & Licences

**Framework UI (Optionnel) :**
- [Qt 6](https://www.qt.io/) - Framework d'application multiplateforme (Qt Widgets et Qt QML)
  
  *Note : Le moteur central est agnostique de l'UI. Qt n'est requis que pour l'interface de bureau fournie. Les développeurs peuvent intégrer n'importe quel framework UI grâce à l'architecture hexagonale propre.*

**Composants Tiers (facilement remplaçables si nécessaire) :**
- [psim](https://github.com/christophschmalhofer/poker/tree/master/XPokerEval/XPokerEval.PokerSim) - Moteur d'évaluation de mains
- [SQLite3](https://www.sqlite.org/) - Base de données embarquée  
- [GoogleTest](https://github.com/google/googletest) - Framework de tests

**Licence :** Licence MIT - Gratuit pour usage commercial et personnel