namespace Citadels.Client;

/// <summary>The five district colours of the game, in the order of the C++ enum.</summary>
public enum CardColor
{
    Noble,
    Trade,
    Religious,
    Military,
    Special,
}

/// <summary>One district card. <see cref="Ability"/> is the rules text of a purple district, or null.</summary>
public sealed record Card(string Name, int Cost, CardColor Color, string? Ability);

/// <summary>What everybody may see about one player.</summary>
public sealed record PlayerSummary(int Seat, string Name, int Gold, int HandSize, IReadOnlyList<Card> City);

/// <summary>Something the engine told us, one JSON line of the protocol.</summary>
public abstract record EngineMessage;

public sealed record AnnounceMessage(string Text) : EngineMessage;

/// <summary>The computer must be handed to this player. The engine waits for an acknowledgement.</summary>
public sealed record HandOffMessage(string Name, int Seat) : EngineMessage;

/// <summary>A question with numbered answers. The engine waits for the 0-based index.</summary>
public sealed record ChooseMessage(string Prompt, IReadOnlyList<string> Options) : EngineMessage;

/// <summary>A yes/no question. The engine waits for the answer.</summary>
public sealed record ConfirmMessage(string Prompt) : EngineMessage;

/// <summary>The public state of the whole table.</summary>
public sealed record TableMessage(int Round, int Crown, IReadOnlyList<PlayerSummary> Players) : EngineMessage;

/// <summary>What one player may see: gold and hand.</summary>
public sealed record PrivateMessage(string Name, int Seat, int Gold, IReadOnlyList<Card> Hand) : EngineMessage;

/// <summary>The engine did not accept an answer. It repeats the question next.</summary>
public sealed record ErrorMessage(string Text) : EngineMessage;

/// <summary>A well-formed message of a type this version does not know (a newer engine).</summary>
public sealed record UnknownMessage(string Type, string RawLine) : EngineMessage;

/// <summary>The engine sent something that breaks the protocol.</summary>
public sealed class ProtocolException : Exception
{
    public ProtocolException(string message) : base(message)
    {
    }

    public ProtocolException(string message, Exception innerException) : base(message, innerException)
    {
    }
}
