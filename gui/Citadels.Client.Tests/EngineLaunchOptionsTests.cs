using Citadels.Client;

namespace Citadels.Client.Tests;

public class EngineLaunchOptionsTests
{
    private static EngineLaunchOptions Options(IReadOnlyList<string> names, int? seed = null)
        => new("citadels", ".", names, seed);

    [Fact]
    public void The_arguments_announce_protocol_mode_and_the_player_count()
    {
        var options = Options(new[] { "Anna", "Ben", "Carol" });

        Assert.Equal(new[] { "--protocol", "--players", "3" }, options.BuildArguments());
    }

    [Fact]
    public void A_seed_is_added_when_given()
    {
        var options = Options(new[] { "Anna", "Ben" }, seed: 42);

        Assert.Equal(new[] { "--protocol", "--players", "2", "--seed", "42" }, options.BuildArguments());
    }

    [Fact]
    public void Names_never_appear_on_the_command_line()
    {
        var options = Options(new[] { "Anna", "Χρήστος" });

        Assert.DoesNotContain("Anna", options.BuildArguments());
        Assert.DoesNotContain("Χρήστος", options.BuildArguments());
    }

    [Theory]
    [InlineData(0)]
    [InlineData(1)]
    [InlineData(8)]
    public void The_number_of_players_must_be_two_to_seven(int count)
    {
        var names = Enumerable.Range(1, count).Select(i => "P" + i).ToList();

        Assert.Throws<ArgumentException>(() => Options(names).Validate());
    }

    [Theory]
    [InlineData(2)]
    [InlineData(7)]
    public void Two_and_seven_players_are_fine(int count)
    {
        var names = Enumerable.Range(1, count).Select(i => "P" + i).ToList();

        Options(names).Validate();
    }

    [Theory]
    [InlineData("")]
    [InlineData("   ")]
    [InlineData("Ann\nBen")]
    [InlineData("Ann\rBen")]
    public void A_bad_name_is_refused(string badName)
    {
        Assert.Throws<ArgumentException>(() => Options(new[] { "Anna", badName }).Validate());
    }

    [Fact]
    public void Without_an_executable_the_options_are_refused()
    {
        var options = new EngineLaunchOptions("", ".", new[] { "Anna", "Ben" });

        Assert.Throws<ArgumentException>(() => options.Validate());
    }
}
