# HoldemCore Qt Quick/QML Mobile UI

## Overview

This is a Qt Quick/QML-based mobile interface for HoldemCore, designed following the same hexagonal architecture as the Qt Widgets version but optimized for mobile and touch interfaces.

## Architecture

### Directory Structure

```
src/ui/qtqml/
├── controller/
│   ├── QmlAppController.h/cpp      # Main application controller
│   ├── TableViewModel.h/cpp        # View model exposing game state to QML
│   └── QmlBridge.h/cpp            # Bridge between engine events and view model
├── qml/
│   ├── Main.qml                   # Application root with StackView
│   ├── SetupScreen.qml            # Game configuration screen
│   ├── TableScreen.qml            # Main poker table screen
│   └── components/
│       ├── PokerTable.qml         # Poker table layout
│       ├── PlayerWidget.qml       # Player display component
│       ├── CardWidget.qml         # Playing card component
│       └── ActionPanel.qml        # Action buttons for human player
└── CMakeLists.txt
```

### Component Responsibilities

#### C++ Layer

**QmlAppController**
- Orchestrates application lifecycle
- Creates and manages core dependencies (Logger, HandEvaluator, etc.)
- Initializes Session, EngineFactory
- Exposes components to QML context

**TableViewModel** 
- Exposes game state as Q_PROPERTY for QML binding
- All properties use Qt types (QString, QVariantList, etc.)
- Provides methods to update individual player data
- Emits signals on property changes for automatic QML updates

**QmlBridge**
- Connects engine GameEvents to TableViewModel updates
- Translates user actions from QML to engine commands
- Handles HumanStrategy interaction
- Converts engine data types to QML-friendly formats

#### QML Layer

**Main.qml**
- Application window with StackView navigation
- Manages screen transitions
- Registers C++ context properties

**SetupScreen.qml**
- Game configuration UI
- Player count, blinds, bot profile, game speed
- Dark theme with styled controls

**TableScreen.qml**
- Main game interface
- Displays poker table, players, community cards
- Shows action panel when human input needed
- Hand results and "Next Hand" button

**Components**
- `PokerTable`: Elliptical table with community cards and players arranged around it
- `PlayerWidget`: Shows player info (name, chips, cards, last action)
- `CardWidget`: Playing card display with face/back rendering
- `ActionPanel`: Betting controls (Fold, Check, Call, Bet/Raise, All-In)

## Data Flow

### Game Initialization
1. User configures game in SetupScreen
2. QML calls `appController.startGame()` with configuration
3. QmlAppController initializes Session with game settings
4. TableViewModel is updated and QML screen transitions to TableScreen

### Game Events
1. Engine fires event (e.g., cards dealt, player action)
2. QmlBridge receives event via lambda callback
3. Bridge updates TableViewModel properties
4. Qt's property system automatically notifies QML
5. QML UI updates instantly via bindings

### User Actions
1. User clicks action button in ActionPanel
2. QML calls `qmlBridge.onPlayerFold()` (or similar)
3. Bridge creates PlayerAction and sends to HumanStrategy
4. Engine processes action and fires new events
5. Cycle continues...

## Building

### Configuration

The project uses CMake presets for different build configurations:

**VSCode (Clang)**
- `vscode-debug-qml`: Debug build with clang-cl
- `vscode-release-qml`: Release build with clang-cl

**Qt Creator (MSVC)**
- `qtcreator-debug-qml`: Debug build with MSVC
- `qtcreator-release-qml`: Release build with MSVC

**Visual Studio (MSVC)**
- `visualstudio-debug-qml`: Debug build with MSVC
- `visualstudio-release-qml`: Release build with MSVC

### Build Commands

```bash
# Configure
cmake --preset=vscode-debug-qml

# Build
cmake --build build/vscode/clang/debug-qml --config Debug

# Run
./build/vscode/clang/debug-qml/src/app/HoldemCore-qml.exe
```

### CMake Options

- `USE_QML=ON`: Enable QML build (default: OFF)
- `USE_WIDGETS=ON`: Enable Widgets build (default: ON)

You can build both UI versions simultaneously by setting both options to ON.

## QML/C++ Integration

### Context Properties

Three C++ objects are registered with the QML engine:

```cpp
rootContext->setContextProperty("appController", qmlController);
rootContext->setContextProperty("tableViewModel", qmlController->getTableViewModel());
rootContext->setContextProperty("qmlBridge", qmlController->getBridge());
```

### Usage in QML

```qml
// Access view model properties
Text { text: "Pot: $" + tableViewModel.potAmount }

// Bind to lists
Repeater {
    model: tableViewModel.players
    PlayerWidget { playerData: modelData }
}

// Call controller methods
Button {
    onClicked: appController.startGame(6, 25, 50, "Tight", 1.0)
}

// Trigger actions
Button {
    onClicked: qmlBridge.onPlayerFold()
}
```

## Design Patterns

### MVVM (Model-View-ViewModel)
- **Model**: Core engine (Session, Engine, Players)
- **ViewModel**: TableViewModel with bindable properties
- **View**: QML components

### Observer Pattern
- Qt signals/slots for property change notifications
- QML automatically observes Q_PROPERTY changes

### Bridge Pattern
- QmlBridge decouples engine events from UI updates
- Translates between domain types and UI types

### Dependency Injection
- QmlAppController receives all dependencies
- Components passed via constructor, not created internally

## Styling

The UI uses a dark theme with:
- Background: `#1e1e1e` (VS Code dark)
- Poker table: `#0d6e3a` (green felt)
- Accents: `#007acc` (blue), `#ffd700` (gold)
- Text: `#ffffff` (white), `#cccccc` (gray)

All colors and styling are inline in QML for this prototype. For production, consider using:
- Qt Quick Controls 2 styling
- QML style sheets
- Theme management system

## Future Enhancements

### Mobile Optimization
- Touch gesture support (swipe, pinch-to-zoom)
- Responsive layout for different screen sizes
- Portrait and landscape orientations
- Haptic feedback

### Animation
- Card dealing animations
- Chip movement to pot
- Player action indicators
- Smooth transitions

### Additional Features
- Hand history display
- Player statistics overlay
- Settings menu
- Sound effects
- Multi-language support

### Technical Improvements
- Separate QML resources into .qrc file
- Add QML type registration for better IDE support
- Implement QML singleton for theme
- Add unit tests for view model
- Performance profiling and optimization

## Testing

To test the QML UI:

1. Build with QML enabled
2. Run `HoldemCore-qml` executable
3. Configure game in setup screen
4. Click "Start Game"
5. Play hands using the action panel

The same core engine is used, so all game logic is identical to the Widgets version.

## Troubleshooting

### QML Module Not Found
Ensure Qt Quick modules are installed:
```bash
qt-unified-windows.exe install qt.qt6.6XX.qml
```

### Black Screen on Launch
- Check that QML files are properly embedded in resources
- Verify `qrc:/qml/Main.qml` path is correct
- Enable QML debugging: `QT_LOGGING_RULES="qt.qml.binding=true"`

### UI Not Updating
- Verify TableViewModel properties emit change signals
- Check that QML bindings are correctly written
- Use `console.log()` in QML for debugging

### Build Errors
- Ensure Qt6 Quick, Qml, and Widgets are installed
- Check that `USE_QML=ON` in CMake configuration
- Verify include paths are correct

## License

Same as HoldemCore project - MIT License.

Copyright (c) 2025 Marc Ennaji
