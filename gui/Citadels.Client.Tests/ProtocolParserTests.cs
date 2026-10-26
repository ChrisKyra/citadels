using Citadels.Client;

namespace Citadels.Client.Tests;

public class ProtocolParserTests
{
    [Fact]
    public void Announce_is_parsed()
    {
        var message = Assert.IsType<AnnounceMessage>(
            ProtocolParser.Parse("""{"type":"announce","text":"Anna plays the King."}"""));

        Assert.Equal("Anna plays the King.", message.Text);
    }

    [Fact]
    public void Announce_unescapes_the_text()
    {
        // JSON: a"b\c<newline>d<backspace>, then an accented letter and Greek letters as raw UTF-8.
        string line = """{"type":"announce","text":"a\"b\\c\nd\u0008 café Ζωή"}""";

        var message = Assert.IsType<AnnounceMessage>(ProtocolParser.Parse(line));

        Assert.Equal("a\"b\\c\nd\b café Ζωή", message.Text);
    }

    [Fact]
    public void HandOff_is_parsed()
    {
        var message = Assert.IsType<HandOffMessage>(
            ProtocolParser.Parse("""{"type":"handOff","name":"Carol","seat":2}"""));

        Assert.Equal("Carol", message.Name);
        Assert.Equal(2, message.Seat);
    }

    [Fact]
    public void Choose_is_parsed()
    {
        var message = Assert.IsType<ChooseMessage>(ProtocolParser.Parse(
            """{"type":"choose","prompt":"Choose an action:","options":["Take 2 gold","Draw 2 cards"]}"""));

        Assert.Equal("Choose an action:", message.Prompt);
        Assert.Equal(new[] { "Take 2 gold", "Draw 2 cards" }, message.Options);
    }

    [Fact]
    public void Confirm_is_parsed()
    {
        var message = Assert.IsType<ConfirmMessage>(
            ProtocolParser.Parse("""{"type":"confirm","prompt":"Sure?"}"""));

        Assert.Equal("Sure?", message.Prompt);
    }

    [Fact]
    public void Error_is_parsed()
    {
        var message = Assert.IsType<ErrorMessage>(
            ProtocolParser.Parse("""{"type":"error","text":"Please enter a number from 0 to 1."}"""));

        Assert.Equal("Please enter a number from 0 to 1.", message.Text);
    }

    private const string TableLine =
        "{\"type\":\"table\",\"round\":3,\"crown\":1,\"players\":["
        + "{\"seat\":0,\"name\":\"Alice\",\"gold\":7,\"handSize\":3,\"city\":["
        + "{\"name\":\"Manor\",\"cost\":3,\"color\":\"noble\",\"ability\":null},"
        + "{\"name\":\"Keep\",\"cost\":3,\"color\":\"special\",\"ability\":\"Cannot be destroyed.\"}]},"
        + "{\"seat\":1,\"name\":\"Bob\",\"gold\":12,\"handSize\":4,\"city\":[]}"
        + "]}";

    [Fact]
    public void Table_is_parsed()
    {
        var message = Assert.IsType<TableMessage>(ProtocolParser.Parse(TableLine));

        Assert.Equal(3, message.Round);
        Assert.Equal(1, message.Crown);
        Assert.Equal(2, message.Players.Count);

        PlayerSummary alice = message.Players[0];
        Assert.Equal(0, alice.Seat);
        Assert.Equal("Alice", alice.Name);
        Assert.Equal(7, alice.Gold);
        Assert.Equal(3, alice.HandSize);
        Assert.Equal(2, alice.City.Count);
        Assert.Equal(new Card("Manor", 3, CardColor.Noble, null), alice.City[0]);
        Assert.Equal(new Card("Keep", 3, CardColor.Special, "Cannot be destroyed."), alice.City[1]);

        PlayerSummary bob = message.Players[1];
        Assert.Equal("Bob", bob.Name);
        Assert.Empty(bob.City);
    }

    [Fact]
    public void Private_is_parsed()
    {
        string line = "{\"type\":\"private\",\"name\":\"Alice\",\"seat\":0,\"gold\":7,\"hand\":["
            + "{\"name\":\"Tavern\",\"cost\":1,\"color\":\"trade\",\"ability\":null},"
            + "{\"name\":\"Fort\",\"cost\":2,\"color\":\"military\",\"ability\":null}]}";

        var message = Assert.IsType<PrivateMessage>(ProtocolParser.Parse(line));

        Assert.Equal("Alice", message.Name);
        Assert.Equal(0, message.Seat);
        Assert.Equal(7, message.Gold);
        Assert.Equal(2, message.Hand.Count);
        Assert.Equal(new Card("Tavern", 1, CardColor.Trade, null), message.Hand[0]);
        Assert.Equal(CardColor.Military, message.Hand[1].Color);
    }

    [Fact]
    public void Private_with_an_empty_hand_is_parsed()
    {
        var message = Assert.IsType<PrivateMessage>(ProtocolParser.Parse(
            """{"type":"private","name":"Carol","seat":2,"gold":0,"hand":[]}"""));

        Assert.Empty(message.Hand);
    }

    [Theory]
    [InlineData("noble", CardColor.Noble)]
    [InlineData("trade", CardColor.Trade)]
    [InlineData("religious", CardColor.Religious)]
    [InlineData("military", CardColor.Military)]
    [InlineData("special", CardColor.Special)]
    public void Every_colour_word_is_understood(string word, CardColor expected)
    {
        string line = "{\"type\":\"private\",\"name\":\"A\",\"seat\":0,\"gold\":0,\"hand\":["
            + "{\"name\":\"X\",\"cost\":1,\"color\":\"" + word + "\",\"ability\":null}]}";

        var message = Assert.IsType<PrivateMessage>(ProtocolParser.Parse(line));

        Assert.Equal(expected, message.Hand[0].Color);
    }

    [Fact]
    public void An_unknown_type_becomes_an_UnknownMessage()
    {
        string line = """{"type":"weather","sunny":true}""";

        var message = Assert.IsType<UnknownMessage>(ProtocolParser.Parse(line));

        Assert.Equal("weather", message.Type);
        Assert.Equal(line, message.RawLine);
    }

    [Theory]
    [InlineData("not json at all")]
    [InlineData("")]
    [InlineData("[]")]
    [InlineData("42")]
    [InlineData("{}")]
    [InlineData("""{"type":7}""")]
    [InlineData("""{"type":"announce"}""")]
    [InlineData("""{"type":"announce","text":5}""")]
    [InlineData("""{"type":"handOff","name":"A","seat":"0"}""")]
    [InlineData("""{"type":"handOff","name":"A","seat":1.5}""")]
    [InlineData("""{"type":"choose","prompt":"p"}""")]
    [InlineData("""{"type":"choose","prompt":"p","options":[1,2]}""")]
    [InlineData("""{"type":"table","round":1,"crown":0}""")]
    [InlineData("""{"type":"table","round":1,"crown":0,"players":[5]}""")]
    [InlineData("""{"type":"private","name":"A","seat":0,"gold":0,"hand":[{"name":"X","cost":1,"color":"green","ability":null}]}""")]
    [InlineData("""{"type":"private","name":"A","seat":0,"gold":0,"hand":[{"name":"X","cost":1,"color":"trade","ability":3}]}""")]
    public void A_line_that_breaks_the_protocol_is_rejected(string line)
    {
        Assert.Throws<ProtocolException>(() => ProtocolParser.Parse(line));
    }

    [Fact]
    public void A_missing_ability_field_means_no_ability()
    {
        string line = "{\"type\":\"private\",\"name\":\"A\",\"seat\":0,\"gold\":0,\"hand\":["
            + "{\"name\":\"X\",\"cost\":1,\"color\":\"trade\"}]}";

        var message = Assert.IsType<PrivateMessage>(ProtocolParser.Parse(line));

        Assert.Null(message.Hand[0].Ability);
    }
}
