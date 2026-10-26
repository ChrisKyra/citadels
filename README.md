# Citadels

A terminal version of the card game **Citadels** (base game, 2 to 7 players, hot-seat: everybody plays on the same computer and passes it around), written in C++20.

Each round the players secretly draft one of eight characters (Assassin, Thief, Magician, King, Bishop, Merchant, Architect, Warlord), then the characters take their turns in rank order: take gold or draw cards, build districts, collect income and use the character's power. The game ends after the round in which a player completes a city of 8 districts, and the best score wins. The 14 special (purple) districts are included.

## Build

You need a C++20 compiler and CMake 3.20 or newer.

```
cmake -B build -DCMAKE_BUILD_TYPE=Release -DSANITIZER=none
cmake --build build
```

(Without those two options CMake makes a Debug build with AddressSanitizer, which is slower but good for development.)

## Play

Start the game from the repository folder, because it reads `data/districts.csv` relative to where you start it:

```
./build/citadels
```

Or give the path of the card list yourself: `./build/citadels path/to/districts.csv`.

- The game asks for the number of players (2 to 7) and their names. The first player gets the crown.
- Every question is a numbered menu: type the number and press Enter.
- When the computer changes hands, the screen is cleared and the next player is asked to press Enter, so nobody sees someone else's cards.
- In your turn you can pick "Card help" to read what your special districts do.

## Project layout

- `src/hpp`, `src/cpp`: the game (cards, characters, districts, rounds, scoring, console interface)
- `data/districts.csv`: the district cards (name, colour, cost, copies, ability)
- `docs/UML.md`: the design as UML diagrams (class model, characters, game lifecycle, turn, draft, scoring); GitHub draws them in the browser. The sources are in `docs/uml/*.mmd`
- `CMakeLists.txt`: builds the `citadels_core` library and the `citadels` program
