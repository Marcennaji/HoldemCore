# HoldemCore

**🌐 Langue / Language :** [Français](README_fr.md) | [English](README.md)

**Projet Portfolio : Architecture logicielle moderne en C++**

HoldemCore est un moteur de simulation **Texas Hold'em** développé en **C++20 moderne**, démontrant des pratiques d’ingénierie logicielle éprouvées:
architecture hexagonale, principes SOLID, inversion de dépendances et refactorisation continue.

Le projet d’origine (2011) a été **entièrement modernisé en 2025** pour illustrer la rigueur et la qualité d’un développement C++ professionnel.

> **Note de portfolio :**  
> Ce dépôt est maintenu comme démonstration d’architecture et d’ingénierie C++.  
> Les issues et PR sont les bienvenues, mais la feuille de route suit les priorités de l’auteur.

---

## 🎯 Points forts du projet

- **Architecture hexagonale** assurant une séparation stricte entre domaine, infrastructure et interface utilisateur  
- **Suite de tests complète**, avec couverture unitaire et de bout en bout  
- **Conception multiplateforme**, basée sur CMake et C++ moderne  
- **Interfaces graphiques multiples :** Qt Widgets et Qt Quick (QML), extensible à d'autres frameworks 
- **Facilité d'implémentation de nouvelles stratégies de poker**, via des interfaces abstraites simples
- **Moteur événementiel** avec machine à états

---

## 🚀 Démarrage rapide

### Prérequis
- CMake ≥ 3.20  
- Compilateur compatible C++20  
- Qt 6.x (uniquement pour l’interface graphique)

### Compilation & Tests
```bash
git clone https://github.com/Marcennaji/HoldemCore.git
cd HoldemCore
cmake --preset debug-widgets
cmake --build build/debug-widgets
.\build\debug-widgets\tests\unit_tests.exe
.\build\debug-widgets\tests\e2e_tests.exe
```

---

## 🏗️ Vue d’ensemble de l’architecture

Explications et diagrammes détaillés :  
👉 [ARCHITECTURE.md](doc/ARCHITECTURE.md)

<p align="center">
  <img src="doc/architecture.png" alt="Diagramme d'architecture hexagonale" width="80%">
</p>

---

## 🧰 Pile technique

**Langages et frameworks**
- **C++20 moderne**
- Système de build **CMake**
- **Qt 6** (Widgets et QML, si vous souhaitez utiliser les UI fournies et optionnelles)
- **GoogleTest** pour les tests
- **SQLite3** pour la persistance

**Composants tiers**
- [psim](https://github.com/christophschmalhofer/poker/tree/master/XPokerEval/XPokerEval.PokerSim)  
- [SQLite3](https://www.sqlite.org/)  
- [GoogleTest](https://github.com/google/googletest)

**Licence :** MIT — utilisation libre, personnelle ou commerciale.

---

## 📈 Feuille de route

- 🧩 Éliminer les violations SOLID restantes dans le code legacy de 2011  
- 🎨 Améliorer l'UI Qt Widgets existante (application desktop) et développer une interface Qt Quick (QML) pour mobile  
- 🧠 Explorer l’intégration d’un bot basé sur machine learning

---

## 👨‍💻 Auteur

**Marc Ennaji** — Ingénieur logiciel senior  
[LinkedIn](https://www.linkedin.com/in/marcennaji/)

### Domaines d’expertise
- Développement C++ moderne
- Architecture logicielle et conception orientée objet  
- Modernisation de systèmes existants  
- Développement piloté par les tests (TDD)

