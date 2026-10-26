using System.Collections.ObjectModel;
using System.Text.RegularExpressions;
using Avalonia.Media;
using Citadels.App.Controls;
using Citadels.App.Infrastructure;
using Citadels.Client;

namespace Citadels.App.ViewModels;

/// <summary>
/// The table during a game. It reads the engine's messages one by one and turns them into what the
/// screen shows; button clicks are sent back as answers.
/// </summary>
public sealed class GameViewModel : ObservableObject, IDisposable
{
    // "Anna plays the King."  (Turn::play)
    private static readonly Regex PlaysRegex = new(@"^(?<name>.+) plays the (?<character>\w+)\.$", RegexOptions.CultureInvariant);

    // "1. Anna: 27 points (districts 20, colours 3, abilities 0, completion 4)"  (Game::finish)
    private static readonly Regex ScoreRegex = new(@"^(?<rank>\d+)\. (?<name>.+): (?<points>\d+) points \((?<details>.*)\)$", RegexOptions.CultureInvariant);

    // "The winner is Anna"  (Game::finish)
    private const string WinnerPrefix = "The winner is ";

    private readonly GameSession _session;
    private readonly Action _backToSetup;
    private readonly CancellationTokenSource _cancellation = new();
    private readonly List<string> _scoreLines = new();
    private bool _collectingScoreboard;
    private bool _disposed;
    private int _activeSeat = -1;

    private string _roundText = "Getting ready";
    private string _roundInfo = string.Empty;
    private string _crownText = string.Empty;
    private string _turnText = string.Empty;
    private Geometry? _turnIcon;
    private bool _showHand;
    private string _handTitle = string.Empty;
    private string _handGold = string.Empty;
    private bool _isHandOff;
    private string _handOffName = string.Empty;
    private string _handOffInitial = string.Empty;
    private IBrush _handOffBrush = Palette.Gold;
    private string _questionEyebrow = "YOUR MOVE";
    private string _questionPrompt = string.Empty;
    private bool _showChoices;
    private bool _showConfirm;
    private bool _isFinished;
    private bool _isError;
    private string _finishTitle = string.Empty;
    private string _finishSubtitle = string.Empty;

    public GameViewModel(GameSession session, Action backToSetup)
    {
        _session = session;
        _backToSetup = backToSetup;
        ReadyCommand = new RelayCommand(() => _ = ReadyAsync());
        YesCommand = new RelayCommand(() => _ = AnswerConfirmAsync(true));
        NoCommand = new RelayCommand(() => _ = AnswerConfirmAsync(false));
        NewGameCommand = new RelayCommand(_backToSetup);
    }

    public ObservableCollection<PlayerViewModel> Players { get; } = new();

    public ObservableCollection<CardViewModel> Hand { get; } = new();

    public ObservableCollection<string> Log { get; } = new();

    public ObservableCollection<OptionViewModel> Options { get; } = new();

    public ObservableCollection<ScoreRowViewModel> ScoreRows { get; } = new();

    public RelayCommand ReadyCommand { get; }

    public RelayCommand YesCommand { get; }

    public RelayCommand NoCommand { get; }

    public RelayCommand NewGameCommand { get; }

    // ----- top bar -----

    public string RoundText
    {
        get => _roundText;
        private set => SetProperty(ref _roundText, value);
    }

    /// <summary>Which characters are face up. Announced before the first hand-off, so it gets its own place.</summary>
    public string RoundInfo
    {
        get => _roundInfo;
        private set
        {
            if (SetProperty(ref _roundInfo, value))
            {
                OnPropertyChanged(nameof(HasRoundInfo));
            }
        }
    }

    public bool HasRoundInfo => _roundInfo.Length > 0;

    public string CrownText
    {
        get => _crownText;
        private set
        {
            if (SetProperty(ref _crownText, value))
            {
                OnPropertyChanged(nameof(HasCrownText));
            }
        }
    }

    public bool HasCrownText => _crownText.Length > 0;

    /// <summary>"Anna · King" while a character plays.</summary>
    public string TurnText
    {
        get => _turnText;
        private set
        {
            if (SetProperty(ref _turnText, value))
            {
                OnPropertyChanged(nameof(HasTurn));
            }
        }
    }

    public Geometry? TurnIcon
    {
        get => _turnIcon;
        private set => SetProperty(ref _turnIcon, value);
    }

    public bool HasTurn => _turnText.Length > 0;

    // ----- your hand -----

    /// <summary>True while the private hand of the player whose turn it is may be shown.</summary>
    public bool ShowHand
    {
        get => _showHand;
        private set => SetProperty(ref _showHand, value);
    }

    public string HandTitle
    {
        get => _handTitle;
        private set => SetProperty(ref _handTitle, value);
    }

    public string HandGold
    {
        get => _handGold;
        private set => SetProperty(ref _handGold, value);
    }

    // ----- pass the computer -----

    /// <summary>The "pass the computer" screen covers everything, so nobody sees the previous player's cards.</summary>
    public bool IsHandOff
    {
        get => _isHandOff;
        private set => SetProperty(ref _isHandOff, value);
    }

    public string HandOffName
    {
        get => _handOffName;
        private set => SetProperty(ref _handOffName, value);
    }

    public string HandOffInitial
    {
        get => _handOffInitial;
        private set => SetProperty(ref _handOffInitial, value);
    }

    public IBrush HandOffBrush
    {
        get => _handOffBrush;
        private set => SetProperty(ref _handOffBrush, value);
    }

    // ----- the question -----

    public string QuestionEyebrow
    {
        get => _questionEyebrow;
        private set => SetProperty(ref _questionEyebrow, value);
    }

    public string QuestionPrompt
    {
        get => _questionPrompt;
        private set => SetProperty(ref _questionPrompt, value);
    }

    public bool ShowChoices
    {
        get => _showChoices;
        private set
        {
            if (SetProperty(ref _showChoices, value))
            {
                OnPropertyChanged(nameof(HasQuestion));
                OnPropertyChanged(nameof(IsWaiting));
            }
        }
    }

    public bool ShowConfirm
    {
        get => _showConfirm;
        private set
        {
            if (SetProperty(ref _showConfirm, value))
            {
                OnPropertyChanged(nameof(HasQuestion));
                OnPropertyChanged(nameof(IsWaiting));
            }
        }
    }

    public bool HasQuestion => _showChoices || _showConfirm;

    public bool IsWaiting => !HasQuestion;

    // ----- the end -----

    /// <summary>The game is over (or broke): the closing screen with the scoreboard or the error.</summary>
    public bool IsFinished
    {
        get => _isFinished;
        private set => SetProperty(ref _isFinished, value);
    }

    public bool IsError
    {
        get => _isError;
        private set
        {
            if (SetProperty(ref _isError, value))
            {
                OnPropertyChanged(nameof(IsVictory));
            }
        }
    }

    public bool IsVictory => !_isError;

    public string FinishTitle
    {
        get => _finishTitle;
        private set => SetProperty(ref _finishTitle, value);
    }

    public string FinishSubtitle
    {
        get => _finishSubtitle;
        private set => SetProperty(ref _finishSubtitle, value);
    }

    /// <summary>Reads messages until the engine ends. Never throws: problems end up on the closing screen.</summary>
    public async Task RunAsync()
    {
        try
        {
            await foreach (EngineMessage message in _session.ReadMessagesAsync(_cancellation.Token))
            {
                Handle(message);
            }

            EngineExit exit = await _session.WaitForExitAsync();
            Finish(exit);
        }
        catch (OperationCanceledException)
        {
            // The window was closed or a new game was started.
        }
        catch (ProtocolException ex)
        {
            Fail("The game engine and the screen no longer understand each other.", ex.Message);
        }
        catch (Exception ex)
        {
            Fail("Something went wrong.", ex.Message);
        }
    }

    public void Dispose()
    {
        if (_disposed)
        {
            return;
        }

        _disposed = true;
        _cancellation.Cancel();
        _ = _session.DisposeAsync().AsTask();
    }

    private void Handle(EngineMessage message)
    {
        switch (message)
        {
            case AnnounceMessage announce:
                Announce(announce.Text);
                break;
            case TableMessage table:
                ApplyTable(table);
                break;
            case PrivateMessage mine:
                ApplyPrivate(mine);
                break;
            case HandOffMessage handOff:
                BeginHandOff(handOff);
                break;
            case ChooseMessage choose:
                ShowChoose(choose);
                break;
            case ConfirmMessage confirm:
                ShowConfirmQuestion(confirm);
                break;
            case ErrorMessage error:
                Log.Add("The engine did not accept the answer: " + error.Text);
                break;
            case UnknownMessage unknown:
                Log.Add("(message of unknown type \"" + unknown.Type + "\")");
                break;
        }
    }

    private void Announce(string text)
    {
        if (text.StartsWith("Final scoreboard", StringComparison.Ordinal))
        {
            _collectingScoreboard = true;
            return;
        }

        if (_collectingScoreboard)
        {
            _scoreLines.Add(text);
            return;
        }

        if (text.StartsWith("Face-up characters", StringComparison.Ordinal)
            || text.StartsWith("No characters are face up", StringComparison.Ordinal))
        {
            // A new round starts. This line comes before the first hand-off, which would wipe it from the log.
            RoundInfo = text.Replace("Face-up characters:", "Face up:", StringComparison.Ordinal);
            TurnText = string.Empty;
            TurnIcon = null;
            return;
        }

        Match plays = PlaysRegex.Match(text);
        if (plays.Success)
        {
            string character = plays.Groups["character"].Value;
            TurnText = plays.Groups["name"].Value + "  ·  " + character;
            TurnIcon = Palette.CharacterIcon(character);
        }

        Log.Add(text);
    }

    private void ApplyTable(TableMessage table)
    {
        RoundText = "Round " + table.Round;

        bool sameSeats = Players.Count == table.Players.Count
                         && Players.Select(p => p.Seat).SequenceEqual(table.Players.Select(p => p.Seat));
        if (!sameSeats)
        {
            Players.Clear();
            foreach (PlayerSummary player in table.Players)
            {
                Players.Add(new PlayerViewModel(player, player.Seat == table.Crown, player.Seat == _activeSeat));
            }
        }
        else
        {
            for (int i = 0; i < table.Players.Count; i++)
            {
                PlayerSummary player = table.Players[i];
                Players[i].Update(player, player.Seat == table.Crown, player.Seat == _activeSeat);
            }
        }

        PlayerSummary? crown = table.Players.FirstOrDefault(p => p.Seat == table.Crown);
        CrownText = crown is null ? string.Empty : crown.Name + " has the crown";
    }

    private void ApplyPrivate(PrivateMessage mine)
    {
        Hand.Clear();
        foreach (Card card in mine.Hand)
        {
            Hand.Add(new CardViewModel(card, large: true));
        }

        HandTitle = mine.Hand.Count == 0 ? $"{mine.Name} has no cards" : $"{mine.Name}'s hand";
        HandGold = mine.Gold + " gold";
        ShowHand = true;
    }

    private void BeginHandOff(HandOffMessage handOff)
    {
        // The same as the terminal version clearing the screen: nothing of the previous player stays visible.
        Log.Clear();
        Hand.Clear();
        ShowHand = false;
        HideQuestion();

        _activeSeat = handOff.Seat;
        foreach (PlayerViewModel player in Players)
        {
            player.IsActive = player.Seat == handOff.Seat;
        }

        HandOffName = handOff.Name;
        HandOffInitial = handOff.Name.Length > 0 ? handOff.Name.Substring(0, 1).ToUpperInvariant() : "?";
        HandOffBrush = Palette.Seat(handOff.Seat);
        IsHandOff = true;
    }

    private void ShowChoose(ChooseMessage choose)
    {
        Options.Clear();
        for (int i = 0; i < choose.Options.Count; i++)
        {
            int index = i;
            string text = choose.Options[i];
            (Geometry? icon, IBrush accent) = Decorate(text);
            Options.Add(new OptionViewModel(
                text,
                new RelayCommand(() => _ = AnswerChooseAsync(index)),
                icon,
                accent,
                IsQuiet(text)));
        }

        QuestionEyebrow = choose.Prompt.Contains("character", StringComparison.OrdinalIgnoreCase)
            ? "CHARACTER DRAFT"
            : "YOUR MOVE";
        QuestionPrompt = choose.Prompt;
        ShowConfirm = false;
        ShowChoices = true;
    }

    private void ShowConfirmQuestion(ConfirmMessage confirm)
    {
        Options.Clear();
        QuestionEyebrow = "YOUR MOVE";
        QuestionPrompt = confirm.Prompt;
        ShowChoices = false;
        ShowConfirm = true;
    }

    /// <summary>An icon and colour for an answer: characters, the turn menu, and districts of the hand.</summary>
    private (Geometry? Icon, IBrush Accent) Decorate(string option)
    {
        Geometry? character = Palette.CharacterIcon(option);
        if (character is not null)
        {
            return (character, Palette.GoldText);
        }

        switch (option)
        {
            case "Take 2 gold":
            case "Collect income":
                return (Icons.CrownCoin, Palette.GoldText);
            case "Draw 2 cards":
            case "Swap hands with a player":
                return (Icons.CardExchange, Palette.GoldText);
            case "Discard cards and draw the same number":
                return (Icons.Cards, Palette.GoldText);
            case "Build a district":
                return (Icons.Trowel, Palette.GoldText);
            case "Use power":
                return (Icons.MagicPalm, Palette.GoldText);
            case "Card help":
                return (Icons.SpellBook, Palette.GoldText);
            case "End turn":
                return (Icons.Hourglass, Palette.Muted);
        }

        if (option.StartsWith("Use ", StringComparison.Ordinal))
        {
            return (Icons.MagicSwirl, Palette.GoldText);
        }

        // "Owner: District (pay n)" is a Warlord target.
        if (option.Contains(": ", StringComparison.Ordinal) && option.Contains("(pay ", StringComparison.Ordinal))
        {
            return (Icons.TowerFall, Palette.Solid("#FF8A8A"));
        }

        // "District (cost)": colour it like the card in the hand or on the table.
        foreach (CardViewModel card in Hand)
        {
            if (option.StartsWith(card.Name + " (", StringComparison.Ordinal))
            {
                return (card.Icon, Palette.For(ColorOf(card)).Accent);
            }
        }

        PlayerViewModel? player = Players.FirstOrDefault(p => p.Name == option);
        if (player is not null)
        {
            return (null, player.SeatBrush);
        }

        return (null, Palette.GoldText);
    }

    private static CardColor ColorOf(CardViewModel card) => card.ColorLabel switch
    {
        "Noble" => CardColor.Noble,
        "Religious" => CardColor.Religious,
        "Trade" => CardColor.Trade,
        "Military" => CardColor.Military,
        _ => CardColor.Special,
    };

    private static bool IsQuiet(string option) =>
        option is "Back" or "Done" or "Don't destroy" or "End turn";

    private void HideQuestion()
    {
        ShowChoices = false;
        ShowConfirm = false;
        Options.Clear();
        QuestionPrompt = string.Empty;
    }

    private async Task ReadyAsync()
    {
        if (!IsHandOff)
        {
            return;
        }

        IsHandOff = false;
        await SendAsync(() => _session.AcknowledgeHandOffAsync(_cancellation.Token));
    }

    private async Task AnswerChooseAsync(int index)
    {
        if (!ShowChoices)
        {
            return;
        }

        HideQuestion();
        await SendAsync(() => _session.AnswerChooseAsync(index, _cancellation.Token));
    }

    private async Task AnswerConfirmAsync(bool yes)
    {
        if (!ShowConfirm)
        {
            return;
        }

        HideQuestion();
        await SendAsync(() => _session.AnswerConfirmAsync(yes, _cancellation.Token));
    }

    private async Task SendAsync(Func<Task> send)
    {
        try
        {
            await send();
        }
        catch (OperationCanceledException)
        {
        }
        catch (Exception ex)
        {
            Fail("The answer could not be sent to the game engine.", ex.Message);
        }
    }

    private void Finish(EngineExit exit)
    {
        if (_disposed)
        {
            return;
        }

        if (exit.ExitCode != 0)
        {
            string details = exit.StandardError.Length > 0
                ? exit.StandardError
                : "The engine stopped with exit code " + exit.ExitCode + ".";
            Fail("The game engine stopped unexpectedly.", details);
            return;
        }

        HideQuestion();
        IsHandOff = false;
        ShowHand = false;
        ScoreRows.Clear();

        string winner = string.Empty;
        foreach (string line in _scoreLines)
        {
            if (line.StartsWith(WinnerPrefix, StringComparison.Ordinal))
            {
                winner = line.Substring(WinnerPrefix.Length);
                continue;
            }

            Match score = ScoreRegex.Match(line);
            if (score.Success)
            {
                string name = score.Groups["name"].Value;
                bool first = score.Groups["rank"].Value == "1";
                PlayerViewModel? player = Players.FirstOrDefault(p => p.Name == name);
                ScoreRows.Add(new ScoreRowViewModel(
                    score.Groups["rank"].Value,
                    name,
                    score.Groups["points"].Value,
                    score.Groups["details"].Value,
                    first,
                    player?.SeatBrush ?? Palette.Gold));
            }
            else
            {
                ScoreRows.Add(new ScoreRowViewModel(string.Empty, line, string.Empty, string.Empty, false, Palette.Gold));
            }
        }

        IsError = false;
        FinishTitle = winner.Length > 0 ? winner + " wins!" : "Game over";
        FinishSubtitle = winner.Length > 0 ? "The most splendid city in the kingdom" : string.Empty;
        IsFinished = true;
    }

    private void Fail(string title, string details)
    {
        if (_disposed)
        {
            return;
        }

        HideQuestion();
        IsHandOff = false;
        ShowHand = false;
        ScoreRows.Clear();
        ScoreRows.Add(new ScoreRowViewModel(string.Empty, details, string.Empty, string.Empty, false, Palette.Gold));
        IsError = true;
        FinishTitle = title;
        FinishSubtitle = "Start a new game to try again.";
        IsFinished = true;
    }
}
