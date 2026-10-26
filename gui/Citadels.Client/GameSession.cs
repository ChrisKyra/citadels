using System.Globalization;
using System.Runtime.CompilerServices;

namespace Citadels.Client;

/// <summary>
/// One game: reads the engine's messages and sends the answers. It remembers which question is open,
/// so a wrong answer (no question open, index out of range) is caught here and not by a stuck engine.
/// </summary>
public sealed class GameSession : IAsyncDisposable
{
    private enum Pending
    {
        None,
        HandOff,
        Choose,
        Confirm,
    }

    private readonly IEngineTransport _transport;
    private Pending _pending = Pending.None;
    private int _optionCount;

    public GameSession(IEngineTransport transport)
    {
        ArgumentNullException.ThrowIfNull(transport);
        _transport = transport;
    }

    /// <summary>
    /// All messages until the engine closes its output. Blank lines are skipped.
    /// Throws <see cref="ProtocolException"/> for a line that breaks the protocol.
    /// </summary>
    public async IAsyncEnumerable<EngineMessage> ReadMessagesAsync(
        [EnumeratorCancellation] CancellationToken cancellationToken = default)
    {
        while (true)
        {
            string? line = await _transport.ReadLineAsync(cancellationToken).ConfigureAwait(false);
            if (line is null)
            {
                yield break;
            }

            if (line.Trim().Length == 0)
            {
                continue;
            }

            EngineMessage message = ProtocolParser.Parse(line);
            switch (message)
            {
                case HandOffMessage:
                    _pending = Pending.HandOff;
                    break;
                case ChooseMessage choose:
                    _pending = Pending.Choose;
                    _optionCount = choose.Options.Count;
                    break;
                case ConfirmMessage:
                    _pending = Pending.Confirm;
                    break;
            }

            yield return message;
        }
    }

    /// <summary>Answers the open <see cref="ChooseMessage"/> with the 0-based index of an option.</summary>
    public async Task AnswerChooseAsync(int index, CancellationToken cancellationToken = default)
    {
        if (_pending != Pending.Choose)
        {
            throw new InvalidOperationException("The engine is not waiting for a choice.");
        }

        if (index < 0 || index >= _optionCount)
        {
            throw new ArgumentOutOfRangeException(nameof(index), index, "There are " + _optionCount + " options.");
        }

        _pending = Pending.None;
        await _transport.WriteLineAsync(index.ToString(CultureInfo.InvariantCulture), cancellationToken)
            .ConfigureAwait(false);
    }

    /// <summary>Answers the open <see cref="ConfirmMessage"/>.</summary>
    public async Task AnswerConfirmAsync(bool yes, CancellationToken cancellationToken = default)
    {
        if (_pending != Pending.Confirm)
        {
            throw new InvalidOperationException("The engine is not waiting for a yes/no answer.");
        }

        _pending = Pending.None;
        await _transport.WriteLineAsync(yes ? "y" : "n", cancellationToken).ConfigureAwait(false);
    }

    /// <summary>Tells the engine the next player has the computer and is ready.</summary>
    public async Task AcknowledgeHandOffAsync(CancellationToken cancellationToken = default)
    {
        if (_pending != Pending.HandOff)
        {
            throw new InvalidOperationException("The engine is not waiting for a hand-off.");
        }

        _pending = Pending.None;
        await _transport.WriteLineAsync(string.Empty, cancellationToken).ConfigureAwait(false);
    }

    /// <summary>Completes when the engine has ended; carries the exit code and stderr.</summary>
    public Task<EngineExit> WaitForExitAsync() => _transport.WaitForExitAsync();

    public ValueTask DisposeAsync() => _transport.DisposeAsync();
}
