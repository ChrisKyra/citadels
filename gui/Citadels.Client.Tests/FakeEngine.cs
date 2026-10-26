using System.Threading.Channels;
using Citadels.Client;

namespace Citadels.Client.Tests;

/// <summary>An engine that exists only in the test: the test sends its lines and reads the answers.</summary>
internal sealed class FakeEngine : IEngineTransport
{
    private readonly Channel<string> _toGui = Channel.CreateUnbounded<string>();
    private readonly TaskCompletionSource<EngineExit> _exit = new(TaskCreationOptions.RunContinuationsAsynchronously);

    /// <summary>Everything the GUI wrote to the engine, in order.</summary>
    public List<string> Written { get; } = new();

    public bool Disposed { get; private set; }

    /// <summary>The engine "prints" one line.</summary>
    public void Send(string line) => _toGui.Writer.TryWrite(line);

    /// <summary>The engine ends: its output closes and the exit code becomes known.</summary>
    public void End(int exitCode = 0, string standardError = "")
    {
        _toGui.Writer.TryComplete();
        _exit.TrySetResult(new EngineExit(exitCode, standardError));
    }

    public async Task<string?> ReadLineAsync(CancellationToken cancellationToken)
    {
        while (await _toGui.Reader.WaitToReadAsync(cancellationToken).ConfigureAwait(false))
        {
            if (_toGui.Reader.TryRead(out string? line))
            {
                return line;
            }
        }

        return null;
    }

    public Task WriteLineAsync(string line, CancellationToken cancellationToken)
    {
        Written.Add(line);
        return Task.CompletedTask;
    }

    public Task<EngineExit> WaitForExitAsync() => _exit.Task;

    public ValueTask DisposeAsync()
    {
        Disposed = true;
        return ValueTask.CompletedTask;
    }
}
