# Contributing to PokerTraining

First of all — thanks for your interest in contributing! 🃏
PokerTraining is an open-source poker simulation tool written in C++ with a strong focus on clean architecture, testability, and modular design.

Whether you're improving strategies, fixing bugs, refactoring code, or suggesting new features — you're welcome here.

---

## 🧠 Design Principles

To keep the project maintainable and high-quality, please respect the following core principles:

### ✅ Decoupling is sacred

* Follow the [SOLID design principles](https://en.wikipedia.org/wiki/SOLID)
* The **core engine** must remain **UI-agnostic** (no Qt, no Flutter, no I/O inside `src/core/`).
* All UI updates are driven by `GameEvents`.

### ✅ No shortcuts

* No large monoliths or god classes (we’re actively breaking them down — not rebuilding them 😉).
* Don’t add new singletons or global state unless we discuss it first.

### ✅ Extend via interfaces

* Use **interfaces** (e.g. `BotStrategy`, etc.) to allow new implementations.
* Favor testable, modular patterns like Strategy, Factory, and Observer.

### ✅ Prefer clean, small commits

* Keep your pull requests focused.
* Always include test cases for new code. Ideally, follow a TDD approach.

### ✅ Maintain naming and style consistency

* Use `camelCase` for variables, `PascalCase` for types.
* Stick to the existing naming conventions unless you're improving them across the board.

---

## 📂 Help needed for...

* developing new UIs, or improving the existing ones (Flutter, Qt Widgets, Qt QML...)
* adding new bot strategies
* ...or any new feature that would be nice to have !

---

## 🛠️ How to Contribute

1. **Fork the repo**
2. **Create a new branch** for your fix/feature
3. **Write clean, self-contained commits**
4. **Push and create a pull request**

---

## 🙏 Final Note

This project aims to be a showcase of modern, maintainable C++ code in a domain (poker) that’s fun and challenging.

We’re glad you’re here — but we care a lot about code quality.
**Please avoid hacks, shortcuts, or quick wins that add technical debt.**

If you're unsure about a design, open a discussion first — we're friendly, but picky 😄

Happy hacking, and may the odds be ever in your favor! ♠️♥️♣️♦️
