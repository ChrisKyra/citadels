using System.Windows.Input;
using Avalonia.Media;

namespace Citadels.App.ViewModels;

/// <summary>One answer button of a question, with an optional icon in an accent colour.</summary>
public sealed class OptionViewModel
{
    public OptionViewModel(string text, ICommand command, Geometry? icon, IBrush accent, bool isMuted)
    {
        Text = text;
        Command = command;
        Icon = icon;
        Accent = accent;
        IsMuted = isMuted;
    }

    public string Text { get; }

    public ICommand Command { get; }

    public Geometry? Icon { get; }

    public bool HasIcon => Icon is not null;

    public IBrush Accent { get; }

    /// <summary>"Back", "Done", "End turn": drawn quieter than the real choices.</summary>
    public bool IsMuted { get; }
}
