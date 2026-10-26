using System.Diagnostics;
using System.Text;

namespace Citadels.Client;

/// <summary>The real engine: the C++ program, started as a child process and talked to over stdin/stdout.</summary>
public sealed class EngineProcess : IEngineTransport
{
    private readonly Process _process;
    private readonly StringBuilder _standardError = new();
    private readonly object _standardErrorLock = new();
    private readonly Task _standardErrorPump;
    private int _disposed;

    private EngineProcess(Process process)
    {
        _process = process;
        _standardErrorPump = Task.Run(PumpStandardErrorAsync);
    }

    /// <summary>Starts the engine and sends the player names.</summary>
    /// <exception cref="ArgumentException">The options are not valid.</exception>
    /// <exception cref="InvalidOperationException">The process could not be started.</exception>
    public static EngineProcess Start(EngineLaunchOptions options)
    {
        ArgumentNullException.ThrowIfNull(options);
        IReadOnlyList<string> arguments = options.BuildArguments();

        MakeExecutable(options.ExecutablePath);

        var startInfo = new ProcessStartInfo
        {
            FileName = options.ExecutablePath,
            WorkingDirectory = options.WorkingDirectory,
            UseShellExecute = false,
            CreateNoWindow = true,
            RedirectStandardInput = true,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            // UTF-8 without a byte-order mark, in both directions.
            StandardInputEncoding = new UTF8Encoding(false),
            StandardOutputEncoding = new UTF8Encoding(false),
            StandardErrorEncoding = new UTF8Encoding(false),
        };
        foreach (string argument in arguments)
        {
            startInfo.ArgumentList.Add(argument);
        }

        Process process = Process.Start(startInfo)
            ?? throw new InvalidOperationException("The engine process could not be started.");
        var engine = new EngineProcess(process);

        process.StandardInput.NewLine = "\n";
        process.StandardInput.AutoFlush = true;
        foreach (string name in options.PlayerNames)
        {
            process.StandardInput.WriteLine(name);
        }

        return engine;
    }

    public Task<string?> ReadLineAsync(CancellationToken cancellationToken)
    {
        return _process.StandardOutput.ReadLineAsync(cancellationToken).AsTask();
    }

    public async Task WriteLineAsync(string line, CancellationToken cancellationToken)
    {
        // AutoFlush is on, so the engine sees the line at once.
        await _process.StandardInput.WriteLineAsync(line.AsMemory(), cancellationToken).ConfigureAwait(false);
    }

    public async Task<EngineExit> WaitForExitAsync()
    {
        await _process.WaitForExitAsync().ConfigureAwait(false);
        await _standardErrorPump.ConfigureAwait(false);
        lock (_standardErrorLock)
        {
            return new EngineExit(_process.ExitCode, _standardError.ToString().Trim());
        }
    }

    public ValueTask DisposeAsync()
    {
        if (Interlocked.Exchange(ref _disposed, 1) == 0)
        {
            try
            {
                if (!_process.HasExited)
                {
                    _process.Kill(entireProcessTree: true);
                }
            }
            catch (Exception)
            {
                // Already gone, or not ours any more: nothing left to stop.
            }

            _process.Dispose();
        }

        return ValueTask.CompletedTask;
    }

    private async Task PumpStandardErrorAsync()
    {
        try
        {
            string? line;
            while ((line = await _process.StandardError.ReadLineAsync().ConfigureAwait(false)) is not null)
            {
                lock (_standardErrorLock)
                {
                    _standardError.AppendLine(line);
                }
            }
        }
        catch (Exception)
        {
            // The process was killed while we were reading: what we have is what we get.
        }
    }

    /// <summary>A program unpacked from a zip file has lost its "executable" flag on macOS and Linux.</summary>
    private static void MakeExecutable(string path)
    {
        if (OperatingSystem.IsWindows() || !File.Exists(path))
        {
            return;
        }

        try
        {
            UnixFileMode mode = File.GetUnixFileMode(path);
            File.SetUnixFileMode(path, mode | UnixFileMode.UserExecute);
        }
        catch (Exception)
        {
            // If this fails, starting the process will report the real problem.
        }
    }
}
