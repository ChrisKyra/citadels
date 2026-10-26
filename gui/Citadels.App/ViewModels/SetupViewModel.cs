using System.Collections.ObjectModel;
using Citadels.App.Infrastructure;
using Citadels.Client;

namespace Citadels.App.ViewModels;

/// <summary>The first screen: how many players and what they are called.</summary>
public sealed class SetupViewModel : ObservableObject
{
    private readonly Func<IReadOnlyList<string>, string?> _start;
    private int _playerCount;
    private string _validationMessage = string.Empty;
    private string _startError = string.Empty;

    /// <param name="start">Starts the game with the (trimmed) names. Returns an error text, or null on success.</param>
    public SetupViewModel(Func<IReadOnlyList<string>, string?> start)
    {
        _start = start;
        PlayerCounts = Enumerable.Range(EngineLaunchOptions.MinPlayers, EngineLaunchOptions.MaxPlayers - EngineLaunchOptions.MinPlayers + 1).ToArray();
        Names = new ObservableCollection<NameEntry>();
        StartCommand = new RelayCommand(Start, () => string.IsNullOrEmpty(_validationMessage));
        PlayerCount = 4;
    }

    public int[] PlayerCounts { get; }

    public ObservableCollection<NameEntry> Names { get; }

    public RelayCommand StartCommand { get; }

    public int PlayerCount
    {
        get => _playerCount;
        set
        {
            if (value < EngineLaunchOptions.MinPlayers || value > EngineLaunchOptions.MaxPlayers)
            {
                return;
            }

            if (SetProperty(ref _playerCount, value))
            {
                ResizeNames(value);
            }
        }
    }

    /// <summary>Why the game cannot start yet; empty when it can.</summary>
    public string ValidationMessage
    {
        get => _validationMessage;
        private set => SetProperty(ref _validationMessage, value);
    }

    /// <summary>The engine could not be found or started.</summary>
    public string StartError
    {
        get => _startError;
        private set
        {
            if (SetProperty(ref _startError, value))
            {
                OnPropertyChanged(nameof(HasStartError));
            }
        }
    }

    public bool HasStartError => _startError.Length > 0;

    private void ResizeNames(int count)
    {
        while (Names.Count < count)
        {
            var entry = new NameEntry(Names.Count, "Player " + (Names.Count + 1));
            entry.PropertyChanged += (_, _) => Revalidate();
            Names.Add(entry);
        }

        while (Names.Count > count)
        {
            Names.RemoveAt(Names.Count - 1);
        }

        Revalidate();
    }

    private void Revalidate()
    {
        var seen = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        string message = string.Empty;
        foreach (NameEntry entry in Names)
        {
            string name = entry.Name.Trim();
            if (name.Length == 0)
            {
                message = "Every player needs a name.";
                break;
            }

            if (!seen.Add(name))
            {
                message = "Two players are called \"" + name + "\". Use different names.";
                break;
            }
        }

        ValidationMessage = message;
        StartCommand.RaiseCanExecuteChanged();
    }

    private void Start()
    {
        StartError = string.Empty;
        var names = Names.Select(entry => entry.Name.Trim()).ToList();
        string? error = _start(names);
        if (error is not null)
        {
            StartError = error;
        }
    }
}
