# HoldemCore

**🌐 Language / Langue:** [English](README.md) | [Français](README_fr.md)

**Projet Portfolio : Architecture C++ Moderne**

Moteur de simulation Texas Hold'em mettant en application des pratiques d’ingénierie logicielle modernes : architecture hexagonale, TDD et CI/CD, principes SOLID, refactoring continu et conception multiplateforme.
Le projet initial, créé en 2011, a fait l'objet en 2025 d'une réécriture très substantielle.

> **Note Portfolio** : Ce dépôt est maintenu comme une démonstration d'ingénierie C++. Les tickets et PRs sont les bienvenus, mais la feuille de route suit les priorités de l'auteur.

---

## 🎯 Vitrine du Projet

Moteur de simulation Texas Hold'em démontrant une architecture logicielle avancée :

- **Architecture Hexagonale** avec séparation stricte des responsabilités
- **Suite de tests complète** garantissant la fiabilité
- **Compatibilité multiplateforme** (Windows, Linux, macOS)
- **Multiples frameworks UI** supportés via des abstractions propres

### Fonctionnalités Clés
- **Modélisation de domaine complexe** : règles de poker, tours d'enchères, évaluation des ranges
- **Framework de stratégies IA** : comportements de bots modulaires avec analyse statistique
- **Machine à états** : gestion robuste et simple du flux de jeu
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

## 🏠 Vue d'Ensemble de l'Architecture

Voir les caractéristiques d'architecture dans [ARCHITECTURE_fr.md](doc/ARCHITECTURE_fr.md)


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

**Support Multiplateforme :**
- **Qt 6** (Widgets et QML)
- **SQLite** base de données embarquée
- **C++ standard** multiplateforme


---
## 🚧 Feuille de Route de Développement
- refactoring des portions legacy respectant encore imparfaitement SOLID 
- Développer une interface graphique Qt Quick (QML), pour version mobile
- Explorer la possibilité d'intégrer une IA de bot basée sur le machine learning

---

##  Dépendances & Licences

**Framework UI (Optionnel) :**
- [Qt 6](https://www.qt.io/) - Framework d'application multiplateforme (Qt Widgets et Qt QML)
  
  *Note : Le moteur central est découplé de l’interface utilisateur. Qt n'est requis que pour l'interface Widgets fournie. Les développeurs peuvent intégrer n'importe quel framework UI grâce à l'architecture hexagonale.*

**Composants Tiers (facilement remplaçables si nécessaire) :**
- [psim](https://github.com/christophschmalhofer/poker/tree/master/XPokerEval/XPokerEval.PokerSim) - Moteur d'évaluation de mains
- [SQLite3](https://www.sqlite.org/) - Base de données embarquée  
- [GoogleTest](https://github.com/google/googletest) - Framework de tests

**Licence :** Licence MIT - Gratuit pour usage commercial et personnel

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
