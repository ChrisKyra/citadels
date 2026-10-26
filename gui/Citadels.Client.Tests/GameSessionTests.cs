using Citadels.Client;

namespace Citadels.Client.Tests;

public class GameSessionTests
{
    private static readonly TimeSpan Patience = TimeSpan.FromSeconds(5);

    private const string ChooseLine =
        """{"type":"choose","prompt":"Choose an action:","options":["Take 2 gold","Draw 2 cards"]}""";

    private static async Task<EngineMessage> NextAsync(IAsyncEnumerator<EngineMessage> messages)
    {
        bool hasNext = await messages.MoveNextAsync().AsTask().WaitAsync(Patience);
        Assert.True(hasNext, "The message stream ended, but a message was expected.");
        return messages.Current;
    }

    [Fact]
    public async Task Messages_arrive_in_order_and_the_stream_ends_with_the_engine()
    {
        var engine = new FakeEngine();
        var session = new GameSession(engine);
        engine.Send("""{"type":"announce","text":"one"}""");
        engine.Send("""{"type":"announce","text":"two"}""");
        engine.End();

        var texts = new List<string>();
        await foreach (EngineMessage message in session.ReadMessagesAsync())
        {
            texts.Add(Assert.IsType<AnnounceMessage>(message).Text);
        }

        Assert.Equal(new[] { "one", "two" }, texts);
    }

    [Fact]
    public async Task Blank_lines_are_skipped()
    {
        var engine = new FakeEngine();
        var session = new GameSession(engine);
        engine.Send("");
        engine.Send("   ");
        engine.Send("""{"type":"announce","text":"hello"}""");
        engine.End();

        var messages = new List<EngineMessage>();
        await foreach (EngineMessage message in session.ReadMessagesAsync())
        {
            messages.Add(message);
        }

        Assert.Single(messages);
    }

    [Fact]
    public async Task A_choice_is_answered_with_its_zero_based_index()
    {
        var engine = new FakeEngine();
        var session = new GameSession(engine);
        engine.Send(ChooseLine);
        await using var messages = session.ReadMessagesAsync().GetAsyncEnumerator();

        Assert.IsType<ChooseMessage>(await NextAsync(messages));
        await session.AnswerChooseAsync(1);

        Assert.Equal(new[] { "1" }, engine.Written);
    }

    [Fact]
    public async Task Answering_without_an_open_question_is_refused()
    {
        var engine = new FakeEngine();
        var session = new GameSession(engine);

        await Assert.ThrowsAsync<InvalidOperationException>(() => session.AnswerChooseAsync(0));
        await Assert.ThrowsAsync<InvalidOperationException>(() => session.AnswerConfirmAsync(true));
        await Assert.ThrowsAsync<InvalidOperationException>(() => session.AcknowledgeHandOffAsync());
        Assert.Empty(engine.Written);
    }

    [Theory]
    [InlineData(-1)]
    [InlineData(2)]
    [InlineData(100)]
    public async Task An_index_outside_the_options_is_refused_and_the_question_stays_open(int index)
    {
        var engine = new FakeEngine();
        var session = new GameSession(engine);
        engine.Send(ChooseLine);
        await using var messages = session.ReadMessagesAsync().GetAsyncEnumerator();
        await NextAsync(messages);

        await Assert.ThrowsAsync<ArgumentOutOfRangeException>(() => session.AnswerChooseAsync(index));
        Assert.Empty(engine.Written);

        await session.AnswerChooseAsync(0);   // the question was still open
        Assert.Equal(new[] { "0" }, engine.Written);
    }

    [Fact]
    public async Task A_question_can_only_be_answered_once()
    {
        var engine = new FakeEngine();
        var session = new GameSession(engine);
        engine.Send(ChooseLine);
        await using var messages = session.ReadMessagesAsync().GetAsyncEnumerator();
        await NextAsync(messages);

        await session.AnswerChooseAsync(0);

        await Assert.ThrowsAsync<InvalidOperationException>(() => session.AnswerChooseAsync(1));
        Assert.Single(engine.Written);
    }

    [Fact]
    public async Task A_confirmation_is_answered_with_y_or_n()
    {
        var engine = new FakeEngine();
        var session = new GameSession(engine);
        engine.Send("""{"type":"confirm","prompt":"Sure?"}""");
        engine.Send("""{"type":"confirm","prompt":"Really?"}""");
        await using var messages = session.ReadMessagesAsync().GetAsyncEnumerator();

        Assert.IsType<ConfirmMessage>(await NextAsync(messages));
        await session.AnswerConfirmAsync(true);
        Assert.IsType<ConfirmMessage>(await NextAsync(messages));
        await session.AnswerConfirmAsync(false);

        Assert.Equal(new[] { "y", "n" }, engine.Written);
    }

    [Fact]
    public async Task A_hand_off_is_acknowledged_with_an_empty_line()
    {
        var engine = new FakeEngine();
        var session = new GameSession(engine);
        engine.Send("""{"type":"handOff","name":"Bob","seat":1}""");
        await using var messages = session.ReadMessagesAsync().GetAsyncEnumerator();

        var handOff = Assert.IsType<HandOffMessage>(await NextAsync(messages));
        await session.AcknowledgeHandOffAsync();

        Assert.Equal("Bob", handOff.Name);
        Assert.Equal(new[] { "" }, engine.Written);
    }

    [Fact]
    public async Task The_wrong_kind_of_answer_is_refused()
    {
        var engine = new FakeEngine();
        var session = new GameSession(engine);
        engine.Send(ChooseLine);
        await using var messages = session.ReadMessagesAsync().GetAsyncEnumerator();
        await NextAsync(messages);

        await Assert.ThrowsAsync<InvalidOperationException>(() => session.AnswerConfirmAsync(true));
        await Assert.ThrowsAsync<InvalidOperationException>(() => session.AcknowledgeHandOffAsync());
        Assert.Empty(engine.Written);
    }

    [Fact]
    public async Task After_an_error_message_the_repeated_question_can_be_answered()
    {
        var engine = new FakeEngine();
        var session = new GameSession(engine);
        engine.Send(ChooseLine);
        engine.Send("""{"type":"error","text":"Please enter a number from 0 to 1."}""");
        engine.Send(ChooseLine);
        await using var messages = session.ReadMessagesAsync().GetAsyncEnumerator();

        Assert.IsType<ChooseMessage>(await NextAsync(messages));
        Assert.IsType<ErrorMessage>(await NextAsync(messages));
        Assert.IsType<ChooseMessage>(await NextAsync(messages));
        await session.AnswerChooseAsync(1);

        Assert.Equal(new[] { "1" }, engine.Written);
    }

    [Fact]
    public async Task A_line_that_breaks_the_protocol_throws_while_reading()
    {
        var engine = new FakeEngine();
        var session = new GameSession(engine);
        engine.Send("this is not json");
        await using var messages = session.ReadMessagesAsync().GetAsyncEnumerator();

        await Assert.ThrowsAsync<ProtocolException>(async () => await NextAsync(messages));
    }

    [Fact]
    public async Task Reading_can_be_cancelled()
    {
        var engine = new FakeEngine();
        var session = new GameSession(engine);
        using var cancellation = new CancellationTokenSource();
        await using var messages = session.ReadMessagesAsync(cancellation.Token).GetAsyncEnumerator();

        Task<bool> pending = messages.MoveNextAsync().AsTask();
        cancellation.Cancel();

        await Assert.ThrowsAnyAsync<OperationCanceledException>(async () => await pending.WaitAsync(Patience));
    }

    [Fact]
    public async Task The_exit_code_and_stderr_are_available_after_the_end()
    {
        var engine = new FakeEngine();
        var session = new GameSession(engine);
        engine.End(1, "Error: could not open data/districts.csv");

        await foreach (EngineMessage _ in session.ReadMessagesAsync())
        {
        }
        EngineExit exit = await session.WaitForExitAsync().WaitAsync(Patience);

        Assert.Equal(1, exit.ExitCode);
        Assert.Contains("districts.csv", exit.StandardError);
    }

    [Fact]
    public async Task Disposing_the_session_disposes_the_transport()
    {
        var engine = new FakeEngine();
        var session = new GameSession(engine);

        await session.DisposeAsync();

        Assert.True(engine.Disposed);
    }

    [Fact]
    public async Task A_short_game_from_start_to_end()
    {
        var engine = new FakeEngine();
        var session = new GameSession(engine);
        engine.Send("""{"type":"handOff","name":"Anna","seat":0}""");
        engine.Send("""{"type":"announce","text":"Anna plays the King."}""");
        engine.Send("""{"type":"table","round":1,"crown":0,"players":[{"seat":0,"name":"Anna","gold":2,"handSize":4,"city":[]},{"seat":1,"name":"Ben","gold":2,"handSize":4,"city":[]}]}""");
        engine.Send("""{"type":"private","name":"Anna","seat":0,"gold":2,"hand":[{"name":"Tavern","cost":1,"color":"trade","ability":null}]}""");
        engine.Send(ChooseLine);
        await using var messages = session.ReadMessagesAsync().GetAsyncEnumerator();

        Assert.IsType<HandOffMessage>(await NextAsync(messages));
        await session.AcknowledgeHandOffAsync();
        Assert.IsType<AnnounceMessage>(await NextAsync(messages));
        var table = Assert.IsType<TableMessage>(await NextAsync(messages));
        var mine = Assert.IsType<PrivateMessage>(await NextAsync(messages));
        Assert.IsType<ChooseMessage>(await NextAsync(messages));
        await session.AnswerChooseAsync(0);
        engine.End();

        Assert.False(await messages.MoveNextAsync().AsTask().WaitAsync(Patience));
        Assert.Equal(2, table.Players.Count);
        Assert.Equal("Tavern", mine.Hand[0].Name);
        Assert.Equal(new[] { "", "0" }, engine.Written);
    }
}
