# Citadels — UML design

Diagrams for the C++ terminal edition (base game, 2–7 players, hot-seat). Sources live in `docs/uml/*.mmd` and render on GitHub.

## Class model

```mermaid
classDiagram
direction LR

class Game {
  -GameConfig config
  -GameState state
  -IUserInterface& ui
  -Random rng
  +run() void
  -setup() void
  -playRound() void
  -finish() void
}

class GameConfig {
  +int playerCount
  +vector~string~ playerNames
  +int districtsToFinish
  +vector~CharacterId~ characterSet
  +string districtDataFile
  +uint32 seed
}

class GameState {
  +vector~Player~ players
  +CharacterDeck characters
  +Deck~DistrictCard~ districtDeck
  +Bank bank
  +int crownHolder
  +int roundNumber
  +optional~int~ firstToComplete
  +leftOf(int seat) int
  +endTriggered() bool
}

class Round {
  -GameState& state
  -IUserInterface& ui
  -RoundState rs
  -CharacterDraft draft
  +play() void
  -removeCharacters() void
  -chooseCharacters() void
  -callCharacters() void
  -endRound() void
}

class RoundState {
  +vector~CharacterId~ faceDown
  +vector~CharacterId~ faceUp
  +array~int~ ownerByRank
  +optional~CharacterId~ murdered
  +optional~CharacterId~ robbed
  +optional~int~ thiefSeat
  +ownerOf(CharacterId id) optional~int~
}

class CharacterDraft {
  <<abstract>>
  +faceUpCount(int players) int
  +run(GameState&, RoundState&, IUserInterface&) void
  +create(int players)$ DraftPtr
}
class StandardDraft
class SevenPlayerDraft
class ThreePlayerDraft
class TwoPlayerDraft

class Turn {
  -TurnContext ctx
  +play() void
  -takeAction() void
  -mainPhase() void
  -endOfTurn() void
}

class TurnContext {
  +GameState& state
  +RoundState& round
  +IUserInterface& ui
  +Player& player
  +Character& character
  +bool actionTaken
  +bool powerUsed
  +bool incomeTaken
  +int buildsLeft
}

class Player {
  +string name
  +int seat
  +int gold
  +Hand hand
  +City city
  +vector~CharacterId~ characters
  +canBuild(DistrictCard card) bool
  +build(int handIndex) void
}

class City {
  -vector~DistrictCard~ districts
  +size() int
  +contains(string name) bool
  +countColor(DistrictColor c) int
  +hasAllColors() bool
  +remove(int index) DistrictCard
  +isComplete(int target) bool
}

class Hand {
  -vector~DistrictCard~ cards
  +add(DistrictCard card) void
  +take(int index) DistrictCard
  +swapWith(Hand other) void
}

class DistrictCard {
  +string name
  +int cost
  +DistrictColor color
  +AbilityPtr ability
  +scoreValue() int
}

class DistrictAbility {
  <<interface>>
  +scoreBonus(City city) int
  +canBeDestroyed() bool
  +destroyCostModifier() int
}

class Deck~T~ {
  -deque~T~ cards
  +shuffle(Random rng) void
  +draw() optional~T~
  +putBottom(T card) void
  +size() int
}

class CharacterDeck {
  -vector~CharacterPtr~ pool
  +ids() vector~CharacterId~
  +get(CharacterId id) Character
}

class Bank {
  +payTo(Player p, int n) void
  +collectFrom(Player p, int n) void
}

class ScoreCalculator {
  +score(Player p, GameState s) ScoreBreakdown
  +ranking(GameState s) vector~ScoreBreakdown~
}

class IUserInterface {
  <<interface>>
  +announce(string msg) void
  +handOff(Player p) void
  +showTable(GameState s) void
  +showPrivate(Player p) void
  +choose(string prompt, vector~string~ options) int
  +confirm(string prompt) bool
}
class ConsoleUI

class CardLoader {
  +loadDistricts(string path)$ vector~DistrictCard~
}

class Character {
  <<abstract>>
}

Game *-- GameConfig
Game *-- GameState
Game --> IUserInterface
Game ..> Round : creates one per round
Game ..> ScoreCalculator : at game end
Game ..> CardLoader : setup
GameState *-- "2..7" Player
GameState *-- CharacterDeck
GameState *-- Deck
GameState *-- Bank
Round *-- RoundState
Round *-- CharacterDraft
Round ..> Turn : one per revealed character
CharacterDraft <|-- StandardDraft
StandardDraft <|-- SevenPlayerDraft
CharacterDraft <|-- ThreePlayerDraft
CharacterDraft <|-- TwoPlayerDraft
Turn *-- TurnContext
TurnContext --> Player
TurnContext --> Character
Player *-- Hand
Player *-- City
Hand o-- DistrictCard
City o-- DistrictCard
DistrictCard --> DistrictAbility
CharacterDeck *-- "8" Character
IUserInterface <|.. ConsoleUI
```

## Character hierarchy

```mermaid
classDiagram
direction TB

class CharacterId {
  <<enumeration>>
  Assassin = 1
  Thief = 2
  Magician = 3
  King = 4
  Bishop = 5
  Merchant = 6
  Architect = 7
  Warlord = 8
}

class DistrictColor {
  <<enumeration>>
  Noble
  Religious
  Trade
  Military
  Special
}

class Character {
  <<abstract>>
  +CharacterId id
  +string name
  +int rank
  +optional~DistrictColor~ incomeColor
  +collectIncome(TurnContext ctx) void
  +onReveal(TurnContext ctx) void
  +afterAction(TurnContext ctx) void
  +buildLimit() int
  +hasPower() bool
  +usePower(TurnContext ctx) void
  +atEndOfTurn(TurnContext ctx) void
  +protectsCity() bool
}

class Assassin {
  +usePower(TurnContext ctx) void
}
class Thief {
  +usePower(TurnContext ctx) void
}
class Magician {
  +usePower(TurnContext ctx) void
}
class King {
  +onReveal(TurnContext ctx) void
}
class Bishop {
  +protectsCity() bool
}
class Merchant {
  +afterAction(TurnContext ctx) void
}
class Architect {
  +afterAction(TurnContext ctx) void
  +buildLimit() int
}
class Warlord {
  +atEndOfTurn(TurnContext ctx) void
}

class CharacterFactory {
  +create(CharacterId id)$ CharacterPtr
}

Character <|-- Assassin
Character <|-- Thief
Character <|-- Magician
Character <|-- King
Character <|-- Bishop
Character <|-- Merchant
Character <|-- Architect
Character <|-- Warlord
Character --> CharacterId
Character --> DistrictColor
CharacterFactory ..> Character : creates
```

## Game lifecycle

```mermaid
stateDiagram-v2
  [*] --> Setup
  Setup : Load districts, shuffle both decks
  Setup : Deal 4 district cards and 2 gold to each player
  Setup : Give the Crown to the starting player

  state "Round N" as Round {
    [*] --> RemoveCharacters
    RemoveCharacters : 1 random card face-down
    RemoveCharacters : 0 to 2 cards face-up (never the King)
    RemoveCharacters --> ChooseCharacters
    ChooseCharacters : Crown holder picks first, then pass to the left
    ChooseCharacters --> CallCharacters
    CallCharacters : Call ranks 1 to 8, one Turn per revealed character
    CallCharacters --> EndOfRound
    EndOfRound : Murdered King's owner takes the Crown
    EndOfRound : Return all characters, reset RoundState
    EndOfRound --> [*]
  }

  Setup --> Round
  state endCheck <<choice>>
  Round --> endCheck
  endCheck --> Round : no city has 8 districts
  endCheck --> Scoring : endTriggered
  Scoring : ScoreCalculator ranks all players
  Scoring --> GameOver
  GameOver : Print scoreboard and winner
  GameOver --> [*]
```

## Calling ranks in a round

```mermaid
flowchart TD
  A(["callCharacters: rank = 1"]) --> B{"Does any player<br/>own this rank?"}
  B -- no --> N["Announce: no one answers"]
  B -- yes --> C{"Is this rank<br/>the murdered one?"}
  C -- yes --> N
  C -- no --> H["handOff to owner<br/>owner reveals the character"]
  H --> D{"Is this rank<br/>the robbed one?"}
  D -- yes --> E["Thief takes all of<br/>the owner's gold"]
  D -- no --> F
  E --> F["character.onReveal(ctx)<br/>King takes the Crown now"]
  F --> T[["Turn.play()"]]
  T --> X
  N --> X{"rank less than 8?"}
  X -- yes --> I["rank = rank + 1"] --> B
  X -- no --> K{"Was the King murdered<br/>and owned by someone?"}
  K -- yes --> KH["King's owner takes the Crown<br/>as the King's heir"]
  K -- no --> Z(["endRound()"])
  KH --> Z
```

## A player's turn

```mermaid
flowchart TD
  S(["Turn starts"]) --> M1{"Menu before action"}
  M1 -- "Take 2 gold" --> A1["gold += 2"]
  M1 -- "Draw 2 cards" --> A2["Draw 2, keep 1,<br/>other to bottom of deck"]
  M1 -- "Use power" --> P1["character.usePower(ctx)<br/>once per turn"] --> M1
  M1 -- "Collect income" --> I1["character.collectIncome(ctx)<br/>once per turn"] --> M1
  M1 -- "View table or hand" --> V1["ui.showTable / showPrivate"] --> M1

  A1 --> AA["actionTaken = true<br/>character.afterAction(ctx)<br/>Merchant +1 gold, Architect +2 cards"]
  A2 --> AA
  AA --> M2{"Menu after action"}

  M2 -- "Build district" --> B1{"buildsLeft > 0<br/>and enough gold<br/>and not a duplicate?"}
  B1 -- no --> M2
  B1 -- yes --> B2["Pay cost to bank<br/>move card from hand to city<br/>buildsLeft -= 1"]
  B2 --> C1{"city.size reaches target<br/>and firstToComplete is empty?"}
  C1 -- yes --> C2["firstToComplete = player<br/>endTriggered = true"] --> M2
  C1 -- no --> M2

  M2 -- "Use power" --> P2["character.usePower(ctx)"] --> M2
  M2 -- "Collect income" --> I2["character.collectIncome(ctx)"] --> M2
  M2 -- "End turn" --> E1["character.atEndOfTurn(ctx)<br/>Warlord may destroy one district"]
  E1 --> E2(["Turn over"])
```

## One round as a sequence

```mermaid
sequenceDiagram
  autonumber
  participant G as Game
  participant R as Round
  participant D as CharacterDraft
  participant UI as ConsoleUI
  participant T as Turn
  participant C as Character
  participant P as Player

  G->>R: play()
  R->>R: removeCharacters()
  R->>D: run(state, roundState, ui)
  loop each pick, from Crown holder to the left
    D->>UI: handOff(picker)
    D->>UI: choose(remaining characters)
    UI-->>D: CharacterId
    D->>P: characters.push_back(id)
  end
  loop rank 1 to 8
    R->>R: ownerOf(rank)
    alt not in play, or murdered
      R->>UI: announce(no one answers)
    else revealed
      R->>UI: handOff(owner)
      opt rank was robbed
        R->>P: move all gold to the Thief
      end
      R->>C: onReveal(ctx)
      R->>T: play()
      T->>UI: choose(gold or cards)
      T->>P: gold += 2, or keep 1 of 2 cards
      T->>C: afterAction(ctx)
      loop until End turn
        T->>UI: choose(build, power, income, end)
        T->>P: build(handIndex)
        T->>C: usePower(ctx) or collectIncome(ctx)
      end
      T->>C: atEndOfTurn(ctx)
    end
  end
  R->>R: endRound()
  R-->>G: round finished
  G->>G: endTriggered()?
```

## Character draft

```mermaid
flowchart TD
  S(["CharacterDraft.run()"]) --> SH["Shuffle the 8 character cards"]
  SH --> FD["Place 1 random card face-down"]
  FD --> N{"How many players?"}
  N -- "4" --> U2["Place 2 cards face-up"]
  N -- "5" --> U1["Place 1 card face-up"]
  N -- "2, 3, 6 or 7" --> PICK
  U2 --> K{"Is the King face-up?"}
  U1 --> K
  K -- yes --> KR["Replace it with a new random card<br/>shuffle the King back"] --> K
  K -- no --> PICK{"Which draft strategy?"}

  PICK -- "StandardDraft (4 to 6)" --> P1["Crown holder picks 1, passes left<br/>until everyone has 1<br/>last card goes face-down"]
  PICK -- "SevenPlayerDraft (7)" --> P7["Same as standard, but the 7th player<br/>may pick the face-down card instead<br/>and puts the other one face-down"]
  PICK -- "ThreePlayerDraft (3)" --> P3["Go around twice, each picks 2<br/>last player picks 1 of the final 2<br/>and puts the other face-down"]
  PICK -- "TwoPlayerDraft (2)" --> P2["A picks 1<br/>B picks 1, discards 1 face-down<br/>A picks 1, discards 1 face-down<br/>B picks 1, last card face-down"]
```

## Endgame and scoring

```mermaid
flowchart TD
  subgraph During["During a turn"]
    B["Player builds a district"] --> Q{"city.size >= districtsToFinish?"}
    Q -- no --> C["Continue the turn"]
    Q -- yes --> F{"firstToComplete already set?"}
    F -- no --> F1["firstToComplete = this player<br/>endTriggered = true"] --> C
    F -- yes --> C
  end

  subgraph After["After endRound()"]
    E{"endTriggered?"}
    E -- no --> NR["Start the next round"]
    E -- yes --> S["ScoreCalculator.score(player)<br/>for every player"]
    S --> S1["+ sum of district costs in the city"]
    S1 --> S2["+3 if the city has all five colors"]
    S2 --> S3{"Completed city?"}
    S3 -- "first to complete" --> S4["+4"]
    S3 -- "completed later" --> S5["+2"]
    S3 -- "not completed" --> W
    S4 --> W["Sort by total, announce the winner"]
    S5 --> W
  end

  C -. "round keeps going until rank 8 is called" .-> E
```
