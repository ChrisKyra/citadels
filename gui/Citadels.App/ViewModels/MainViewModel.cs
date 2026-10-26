using Citadels.App.Infrastructure;
using Citadels.Client;

namespace Citadels.App.ViewModels;

/// <summary>The window's view model: it only decides which screen is showing.</summary>
public sealed class MainViewModel : ObservableObject
{
    private ObservableObject _current;

    public MainViewModel()
    {
        _current = new SetupViewModel(StartGame);
    }

    /// <summary>The screen that is showing: <see cref="SetupViewModel"/> or <see cref="GameViewModel"/>.</summary>
    public ObservableObject Current
    {
        get => _current;
        private set => SetProperty(ref _current, value);
    }

    /// <summary>Stops a running game (the engine process) when the window closes.</summary>
    public void Shutdown()
    {
        if (_current is GameViewModel game)
        {
            game.Dispose();
        }
    }

    private string? StartGame(IReadOnlyList<string> names)
    {
        EngineLocation? location = EngineLocator.Find(
            AppContext.BaseDirectory,
            Environment.GetEnvironmentVariable("CITADELS_ENGINE_DIR"));
        if (location is null)
        {
            return "The game engine was not found. It belongs in the folder \"engine\" next to this program "
                   + "(with data/districts.csv inside), or build the C++ game in the repository first "
                   + "(cmake --build build), or set the environment variable CITADELS_ENGINE_DIR.";
        }

        try
        {
            var options = new EngineLaunchOptions(location.ExecutablePath, location.WorkingDirectory, names);
            var session = new GameSession(EngineProcess.Start(options));
            var game = new GameViewModel(session, ShowSetup);
            Current = game;
            _ = game.RunAsync();   // handles its own errors, so nothing is left unobserved
            return null;
        }
        catch (Exception ex)
        {
            return "The engine could not be started: " + ex.Message;
        }
    }

    private void ShowSetup()
    {
        if (_current is GameViewModel game)
        {
            game.Dispose();
        }

        Current = new SetupViewModel(StartGame);
    }
}
