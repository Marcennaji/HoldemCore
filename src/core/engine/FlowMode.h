namespace pkt::core
{

enum class FlowMode
{
    Legacy, // betting rounds : big ball of mud with some GUI calls to pilot the engine (old system). Ouch.
    Fsm // New engine-controlled flow (Finished State Machine for betting rounds), no engine piloting from the GUI. We
        // are happy now!
};

} // namespace pkt::core
