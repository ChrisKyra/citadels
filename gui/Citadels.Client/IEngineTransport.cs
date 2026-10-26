namespace Citadels.Client;

/// <summary>How the engine ended: its exit code and everything it wrote to stderr.</summary>
public sealed record EngineExit(int ExitCode, string StandardError);

/// <summary>
/// The line-based connection to the engine. The real one is <see cref="EngineProcess"/>;
/// the tests use a fake, which is why <see cref="GameSession"/> only knows this interface.
/// </summary>
public interface IEngineTransport : IAsyncDisposable
{
    /// <summary>The next line the engine wrote, or null when the engine has closed its output.</summary>
    Task<string?> ReadLineAsync(CancellationToken cancellationToken);

    /// <summary>Sends one line (without the line end) to the engine and flushes it.</summary>
    Task WriteLineAsync(string line, CancellationToken cancellationToken);

    /// <summary>Completes when the engine has ended.</summary>
    Task<EngineExit> WaitForExitAsync();
}
