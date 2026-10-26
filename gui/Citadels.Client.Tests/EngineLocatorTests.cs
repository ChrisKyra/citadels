using Citadels.Client;

namespace Citadels.Client.Tests;

public sealed class EngineLocatorTests : IDisposable
{
    private readonly string _root = Path.Combine(Path.GetTempPath(), "citadels-locator-" + Guid.NewGuid().ToString("N"));

    public EngineLocatorTests()
    {
        Directory.CreateDirectory(_root);
    }

    public void Dispose()
    {
        try
        {
            Directory.Delete(_root, recursive: true);
        }
        catch (IOException)
        {
        }
    }

    private string Folder(params string[] parts)
    {
        string path = Path.Combine(new[] { _root }.Concat(parts).ToArray());
        Directory.CreateDirectory(path);
        return path;
    }

    private static void Touch(string folder, params string[] relativePath)
    {
        string path = Path.Combine(new[] { folder }.Concat(relativePath).ToArray());
        Directory.CreateDirectory(Path.GetDirectoryName(path)!);
        File.WriteAllText(path, "x");
    }

    private static void PutEngineAndData(string folder)
    {
        Touch(folder, EngineLocator.ExecutableName);
        Touch(folder, "data", "districts.csv");
    }

    [Fact]
    public void A_release_finds_the_engine_in_the_engine_folder_next_to_the_app()
    {
        string app = Folder("app");
        PutEngineAndData(Path.Combine(app, "engine"));

        EngineLocation? location = EngineLocator.Find(app);

        Assert.NotNull(location);
        Assert.Equal(Path.Combine(app, "engine", EngineLocator.ExecutableName), location!.ExecutablePath);
        Assert.Equal(Path.Combine(app, "engine"), location.WorkingDirectory);
    }

    [Fact]
    public void The_override_folder_wins()
    {
        string app = Folder("app");
        PutEngineAndData(Path.Combine(app, "engine"));
        string custom = Folder("custom");
        PutEngineAndData(custom);

        EngineLocation? location = EngineLocator.Find(app, custom);

        Assert.NotNull(location);
        Assert.Equal(custom, location!.WorkingDirectory);
    }

    [Fact]
    public void An_override_without_an_engine_falls_back_to_the_normal_search()
    {
        string app = Folder("app");
        PutEngineAndData(Path.Combine(app, "engine"));
        string empty = Folder("empty");

        EngineLocation? location = EngineLocator.Find(app, empty);

        Assert.NotNull(location);
        Assert.Equal(Path.Combine(app, "engine"), location!.WorkingDirectory);
    }

    [Fact]
    public void A_development_build_is_found_from_a_folder_deep_inside_the_repository()
    {
        string repository = Folder("repo");
        Touch(repository, "data", "districts.csv");
        Touch(repository, "build", EngineLocator.ExecutableName);
        string appOutput = Folder("repo", "gui", "Citadels.App", "bin", "Debug", "net10.0");

        EngineLocation? location = EngineLocator.Find(appOutput);

        Assert.NotNull(location);
        Assert.Equal(Path.Combine(repository, "build", EngineLocator.ExecutableName), location!.ExecutablePath);
        Assert.Equal(repository, location.WorkingDirectory);
    }

    [Theory]
    [InlineData("Release")]
    [InlineData("Debug")]
    public void The_build_configuration_folders_of_Visual_Studio_are_searched(string configuration)
    {
        string repository = Folder("repo");
        Touch(repository, "data", "districts.csv");
        Touch(repository, "build", configuration, EngineLocator.ExecutableName);
        string appOutput = Folder("repo", "gui", "out");

        EngineLocation? location = EngineLocator.Find(appOutput);

        Assert.NotNull(location);
        Assert.Equal(Path.Combine(repository, "build", configuration, EngineLocator.ExecutableName), location!.ExecutablePath);
        Assert.Equal(repository, location.WorkingDirectory);
    }

    [Fact]
    public void An_engine_without_the_data_file_is_not_accepted()
    {
        string app = Folder("app");
        Touch(Path.Combine(app, "engine"), EngineLocator.ExecutableName);

        Assert.Null(EngineLocator.Find(app));
    }

    [Fact]
    public void A_data_folder_without_the_engine_is_not_accepted()
    {
        string app = Folder("app");
        Touch(Path.Combine(app, "engine"), "data", "districts.csv");

        Assert.Null(EngineLocator.Find(app));
    }
}
