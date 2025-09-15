# Legacy to FSM Migration Plan

## Overview
Remove all legacy (non-FSM) poker engine code and focus exclusively on the FSM implementation.

## Phase 1: Assessment
- [ ] Identify all files in `src/core/engine/deprecated/`
- [ ] Find all references to legacy `Player` class
- [ ] Document FSM-only interfaces that need implementation
- [ ] List all tests that need conversion from golden master to unit/integration

## Phase 2: CurrentHandContext Cleanup
```cpp
struct PlayersInHandContext
{
    int nbPlayers = 0;
    PlayerFsmList actingPlayersList; // Remove "Fsm" suffix, make this the primary
    std::shared_ptr<PlayerFsm> preflopLastRaiser = nullptr; // Remove "Fsm" suffix
    std::shared_ptr<PlayerFsm> flopLastRaiser = nullptr;
    std::shared_ptr<PlayerFsm> turnLastRaiser = nullptr;
    std::shared_ptr<PlayerFsm> lastVPIPPlayer = nullptr;
    std::vector<pkt::core::PlayerPosition> callersPositions;
    std::vector<pkt::core::PlayerPosition> raisersPositions;
};
```

## Phase 3: Strategy Classes Update
- Remove all defensive "if legacy else FSM" code
- Update all strategies to use simplified FSM-only context
- Rename `PlayerFsm` to `Player` (make it the primary player class)

## Phase 4: Test Strategy Replacement
Replace golden master tests with:
- Comprehensive unit tests for each FSM state
- Integration tests for complete hand scenarios
- Property-based tests for poker rules validation
- Performance benchmarks

## Phase 5: Cleanup
- Remove `src/core/engine/deprecated/` directory
- Remove legacy includes and dependencies
- Update build system
- Clean up interfaces

## Benefits
1. Single implementation to maintain
2. Cleaner, more consistent API
3. Better testability
4. Easier future development
5. No more dual-system complexity

## Risks & Mitigation
- **Risk**: Losing proven legacy functionality
  - **Mitigation**: Comprehensive test suite before removal
- **Risk**: Introduction of new bugs
  - **Mitigation**: Gradual migration with extensive testing at each step