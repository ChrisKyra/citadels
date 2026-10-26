using Citadels.Client;

namespace Citadels.Client.Tests;

/// <summary>
/// Plays the REAL C++ engine for a while with random answers. It runs only when the environment variable
/// CITADELS_ENGINE_DIR names a folder that holds the engine program and data/districts.csv
/// (or the repository root with a build); the CI sets it. Without it the test returns at once.
/// </summary>
public class EngineIntegrationTests
{
    [Fact]
    public async Task The_real_engine_can_be_played_with_random_answers()
    {
        string? folder = Environment.GetEnvironmentVariable("CITADELS_ENGINE_DIR");
        if (string.IsNullOrWhiteSpace(folder))
        {
            return;
        }

        EngineLocation? location = EngineLocator.Find(AppContext.BaseDirectory, folder);
        Assert.NotNull(location);

        var names = new[] { "Anna", "Ben", "Χρήστος" };   // a Greek name: it must survive the trip through stdin
        var options = new EngineLaunchOptions(location!.ExecutablePath, location.WorkingDirectory, names, Seed: 12345);
        await using var session = new GameSession(EngineProcess.Start(options));
        using var timeout = new CancellationTokenSource(TimeSpan.FromSeconds(90));

        var random = new Random(7);
        int questions = 0;
        int handOffs = 0;
        int tables = 0;
        int privateViews = 0;
        bool greekNameSeen = false;
        bool engineEnded = true;

        await foreach (EngineMessage message in session.ReadMessagesAsync(timeout.Token))
        {
            switch (message)
            {
                case HandOffMessage:
                    handOffs++;
                    await session.AcknowledgeHandOffAsync(timeout.Token);
                    break;
                case ChooseMessage choose:
                    questions++;
                    await session.AnswerChooseAsync(random.Next(choose.Options.Count), timeout.Token);
                    break;
                case ConfirmMessage:
                    questions++;
                    await session.AnswerConfirmAsync(random.Next(2) == 0, timeout.Token);
                    break;
                case TableMessage table:
                    tables++;
                    greekNameSeen |= table.Players.Any(p => p.Name == "Χρήστος");
                    break;
                case PrivateMessage:
                    privateViews++;
                    break;
                case ErrorMessage error:
                    Assert.Fail("The engine rejected an answer: " + error.Text);
                    break;
            }

            if (questions >= 300)
            {
                engineEnded = false;
                break;
            }
        }

        Assert.True(handOffs > 0, "no hand-off seen");
        Assert.True(tables > 0, "no table seen");
        Assert.True(privateViews > 0, "no private view seen");
        Assert.True(greekNameSeen, "the Greek player name did not arrive intact");

        if (engineEnded)
        {
            EngineExit exit = await session.WaitForExitAsync();
            Assert.True(exit.ExitCode == 0, "The engine failed: " + exit.StandardError);
        }
    }
}
