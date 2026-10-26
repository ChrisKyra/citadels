# The C++ side of the GUI

Everything the graphical program needs from the game is in place:

- `src/hpp/protocol_ui.hpp`, `src/cpp/protocol_ui.cpp`: `ProtocolUI`, the third `IUserInterface` next to `ConsoleUI` and `ScriptedUI`. It writes one compact JSON line per call, flushes it, and reads plain-text answers (0-based index for `choose`, `y`/`n` for `confirm`, any line for `handOff`). A bad answer sends an `error` line and repeats the question. The format is in `docs/GUI_PROTOCOL.md` and tested in `tests/test_protocol_ui.cpp`.
- `src/cpp/main.cpp`: two ways to start.
  - `citadels [path/to/districts.csv]`: the terminal game, as before.
  - `citadels --protocol --players N [--seed S] [--data path]`: the GUI mode. The N player names are read from stdin, one per line (not from the command line, because Windows converts the command line to the local code page), and stdout carries nothing but protocol lines. Bad arguments, a missing name or a player count outside 2 to 7 give a message on stderr and exit code 2; any other failure exits with code 1.
  - `--seed S` (a whole number) and `--data path` also work in the terminal mode when given as flags.
- `CMakeLists.txt`: `protocol_ui.cpp` is part of `citadels_core`, `test_protocol_ui.cpp` is part of `tests`, and the warning flags are `/W4 /utf-8` for Visual Studio and `-Wall -Wextra -Wpedantic` for GCC and Clang.

## Trying the protocol by hand (no GUI)

```
./build/citadels --protocol --players 2
```

Type `Anna`, Enter, `Ben`, Enter, and you see JSON lines. Answer a `choose` with a number (`0` is the first option), a `confirm` with `y` or `n`, and a `handOff` with Enter. That is exactly what the C# program does with buttons.

## Not checked yet

Nothing of this has been compiled into a program or run: the files were only syntax-checked. The first real build, the Catch2 run and a first Windows (Visual Studio) build will show whether `/W4` reports warnings in older files.
