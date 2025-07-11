# Log Tools - Developed with Unreal Engine
Helps with cpp and bp logs. Provides a BP node called LogGame as well as a few cpp macros and functions.
# Project Features
  - Adds context to logs including:
    - Network role (e.g., Listen Server, Client)
    - Actor label (as it appears in the World Outliner)
    - Calling function name
    - Last relevant Blueprint or C++ call in the call stack
  - Supports both Blueprint and C++ usage via:
    - A Blueprint-callable `LogGame()` function
    - A C++ macro `LOG_GAME(...)` for convenient use in native code
 
  Intended Usage:
  - Helpful for debugging multiplayer games with replicated actors
  - Designed for gameplay programmers to identify which actor or system is producing logs
  - Works in both PIE and standalone modes
 
  Limitations:
  - Should be used in development builds only
  - Not intended for performance-critical code (hidden in shipping builds)
 