namespace Citadels.Client;

/// <summary>Where the engine program is, and the folder it must be started in (the one that holds <c>data/</c>).</summary>
public sealed record EngineLocation(string ExecutablePath, string WorkingDirectory);

/// <summary>Finds the engine next to the app (a release) or in the repository (development).</summary>
public static class EngineLocator
{
    private const string DataFile = "districts.csv";

    public static string ExecutableName => OperatingSystem.IsWindows() ? "citadels.exe" : "citadels";

    /// <param name="appBaseDirectory">The folder of the GUI program, usually <c>AppContext.BaseDirectory</c>.</param>
    /// <param name="overrideDirectory">A folder chosen by the user (environment variable <c>CITADELS_ENGINE_DIR</c>), or null.</param>
    /// <returns>The location, or null when nothing was found.</returns>
    public static EngineLocation? Find(string appBaseDirectory, string? overrideDirectory = null)
    {
        if (!string.IsNullOrWhiteSpace(overrideDirectory))
        {
            EngineLocation? chosen = TryPackaged(overrideDirectory) ?? TryRepository(overrideDirectory);
            if (chosen is not null)
            {
                return chosen;
            }
        }

        // Release layout: <app folder>/engine/citadels(.exe) and <app folder>/engine/data/districts.csv
        EngineLocation? packaged = TryPackaged(Path.Combine(appBaseDirectory, "engine"));
        if (packaged is not null)
        {
            return packaged;
        }

        // Development layout: somewhere above the GUI's output folder is the repository root.
        for (DirectoryInfo? directory = new DirectoryInfo(appBaseDirectory); directory is not null; directory = directory.Parent)
        {
            EngineLocation? found = TryRepository(directory.FullName);
            if (found is not null)
            {
                return found;
            }
        }

        return null;
    }

    /// <summary>A folder with the program and a <c>data</c> folder side by side.</summary>
    private static EngineLocation? TryPackaged(string folder)
    {
        string executable = Path.Combine(folder, ExecutableName);
        if (File.Exists(executable) && HasData(folder))
        {
            return new EngineLocation(executable, folder);
        }

        return null;
    }

    /// <summary>A repository root: <c>data/districts.csv</c> and the program in <c>build</c> (or <c>build/Release</c>, <c>build/Debug</c>).</summary>
    private static EngineLocation? TryRepository(string root)
    {
        if (!HasData(root))
        {
            return null;
        }

        string[] candidates =
        {
            Path.Combine(root, "build", ExecutableName),
            Path.Combine(root, "build", "Release", ExecutableName),
            Path.Combine(root, "build", "Debug", ExecutableName),
        };
        foreach (string candidate in candidates)
        {
            if (File.Exists(candidate))
            {
                return new EngineLocation(candidate, root);
            }
        }

        return null;
    }

    private static bool HasData(string folder)
    {
        return File.Exists(Path.Combine(folder, "data", DataFile));
    }
}
