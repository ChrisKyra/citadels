namespace Citadels.Client;

/// <summary>
/// How to start the engine: <c>citadels --protocol --players N [--seed S]</c>, then the N player names,
/// one per line, on stdin. (Names go through stdin and not through the command line, because a command
/// line is converted to the local code page on Windows and non-ASCII names such as Greek ones would break.)
/// </summary>
public sealed record EngineLaunchOptions(
    string ExecutablePath,
    string WorkingDirectory,
    IReadOnlyList<string> PlayerNames,
    int? Seed = null)
{
    public const int MinPlayers = 2;
    public const int MaxPlayers = 7;

    /// <exception cref="ArgumentException">The options cannot be used to start a game.</exception>
    public void Validate()
    {
        if (string.IsNullOrWhiteSpace(ExecutablePath))
        {
            throw new ArgumentException("No engine executable given.", nameof(ExecutablePath));
        }

        if (PlayerNames is null || PlayerNames.Count < MinPlayers || PlayerNames.Count > MaxPlayers)
        {
            throw new ArgumentException(
                $"A game needs {MinPlayers} to {MaxPlayers} players.", nameof(PlayerNames));
        }

        foreach (string name in PlayerNames)
        {
            if (string.IsNullOrWhiteSpace(name))
            {
                throw new ArgumentException("A player name cannot be empty.", nameof(PlayerNames));
            }

            if (name.Contains('\n') || name.Contains('\r'))
            {
                throw new ArgumentException("A player name cannot contain a line break.", nameof(PlayerNames));
            }
        }
    }

    /// <summary>The command-line arguments of the engine (not the program name).</summary>
    public IReadOnlyList<string> BuildArguments()
    {
        Validate();
        var arguments = new List<string>
        {
            "--protocol",
            "--players",
            PlayerNames.Count.ToString(System.Globalization.CultureInfo.InvariantCulture),
        };

        if (Seed is int seed)
        {
            arguments.Add("--seed");
            arguments.Add(seed.ToString(System.Globalization.CultureInfo.InvariantCulture));
        }

        return arguments;
    }
}
