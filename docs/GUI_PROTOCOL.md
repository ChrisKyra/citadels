# GUI protocol (version 1)

The C# program starts the game as a child process and talks to it over stdin and stdout.

```
citadels --protocol --players N [--seed S] [--data path/to/districts.csv]
```

then writes the **N player names, one per line, UTF-8, on stdin** (names do not go on the command line: on Windows the command line is converted to the local code page, which would turn Greek names into question marks). After that the game runs.

The program is started in the folder that contains `data/districts.csv`, so `--data` is only for development.

## Lines

UTF-8, one JSON object per line, `\n` ends a line. The engine writes **only** these lines on stdout and flushes after each one; errors and crashes go to stderr (exit code 1; bad arguments exit code 2). End of the output with exit code 0 means the game is over. Output is compact: no spaces outside strings, keys in exactly the order shown.

Engine to GUI:

```
{"type":"announce","text":"Anna plays the King."}
{"type":"handOff","name":"Anna","seat":0}
{"type":"choose","prompt":"Choose an action:","options":["Take 2 gold","Draw 2 cards"]}
{"type":"confirm","prompt":"..."}
{"type":"table","round":3,"crown":1,"players":[{"seat":0,"name":"Anna","gold":4,"handSize":3,"city":[CARD,...]},...]}
{"type":"private","name":"Anna","seat":0,"gold":4,"hand":[CARD,...]}
{"type":"error","text":"Please enter a number from 0 to 1."}
```

`CARD` = `{"name":"Manor","cost":3,"color":"noble","ability":null}`. Colours: `noble`, `trade`, `religious`, `military`, `special`. `ability` is the text of `DistrictAbility::description()` or `null`.

Strings: `"` is `\"`, `\` is `\\`, newline `\n`, carriage return `\r`, tab `\t`, every other character below 0x20 is `\u00xx` (lower-case hex). Everything from 0x20 up, UTF-8 included, is copied unchanged.

GUI to engine (plain lines, no JSON):

| after | answer |
|---|---|
| `handOff` | any line, an empty line is fine |
| `choose` | the **0-based** index of the option, as a whole-line integer |
| `confirm` | `y`/`yes` or `n`/`no`, any case |

A bad answer makes the engine send an `error` line and then repeat the same question. End of input while the engine waits makes the engine fail with exit code 1.

## What the C# side does with it

- `handOff`: the whole window is covered by a "pass the computer" screen, the log and the private hand are cleared (the terminal version clears the screen at the same moment), and the answer is sent when the player clicks "I am ready".
- `choose`: one button per option. `confirm`: Yes/No buttons.
- `table` and `private` replace the table and the hand on screen.
- `announce` lines go to the log; the lines from "Final scoreboard" on go to the closing screen.
