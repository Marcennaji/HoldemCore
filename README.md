# PokerTraining

**PokerTraining** is a No-Limit Texas Hold'em training software focused exclusively on **cash game strategy**. It is designed for offline use, whether you're playing casually or working to improve your skills through structured practice. The software features smart, adaptable AI opponents and a variety of tools to analyze play and support your growth as a cash game player.

Play against up to 9 bot players, each with customizable and dynamic playing styles ranging from ultra-tight to loose-aggressive. Bots adjust their behavior in response to opponents' patterns, offering a more realistic and educational experience.

> 🎯 **Focused on Cash Games**: Tournament play is intentionally excluded to keep the software specialized on deep-stack, cash game strategy — where long-term decision making, range balancing, and exploitative adjustments matter most.

---

## Features

- **Player Statistics**  
  Track and analyze both your performance and that of the AI opponents over time.

  For each player — including both humans and bots — PokerTraining tracks detailed statistics across all four betting rounds (preflop, flop, turn, river). These include:

  - **Preflop stats**:  
    - Voluntarily Put Money in Pot (VPIP)  
    - Preflop Raise (PFR)  
    - 3-bet and 4-bet frequencies  
    - 3-bet opportunities  
    - Historical action patterns from recent hands

  - **Postflop stats (Flop, Turn, River)**:  
    - Aggression factor and aggression frequency  
    - Continuation bet frequency (on the flop)  
    - Counts of folds, checks, calls, bets, raises, and re-raises

  - **Showdown tendency**:  
    - Overall "Went to Showdown" (WTSD) frequency

  These stats are calculated over time and resettable, allowing players to analyze trends, adapt strategy, and better understand both human and bot opponent behavior.

- **Range Evaluation**  
  Estimate the possible hand ranges of each player based on their actions in the current hand — great for learning range analysis and narrowing.

- **Customizable AI Opponents**  
  Configure each bot’s base playing style (tight, loose, aggressive, passive, etc.) and observe how they adapt throughout a session.

- **Qt-Based GUI**  
  A responsive and user-friendly graphical interface built with Qt, ensuring smooth interaction across platforms.

---

## Who Is This For?

- 🧠 **Players looking to improve cash game fundamentals**
- 🤖 **Users who want to train against intelligent, adaptive AI opponents**
- ✈️ **Anyone wanting to play poker offline without needing internet access**

---

## Installation

> 🛠️ *Note: Build instructions may vary depending on your OS. Below is a general overview.*

### Prerequisites
- C++17-compatible compiler
- [Qt 5 or 6](https://www.qt.io/)
- CMake (3.15+ recommended)

### Build (Linux/macOS/Windows)
```bash
git clone https://github.com/Marcennaji/PokerTraining.git
cd PokerTraining
mkdir build && cd build
cmake ..
make
./PokerTraining
