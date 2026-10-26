using Avalonia.Media;

namespace Citadels.App.ViewModels;

/// <summary>One line of the final scoreboard.</summary>
public sealed class ScoreRowViewModel
{
    public ScoreRowViewModel(string rank, string name, string points, string details, bool isWinner, IBrush accent)
    {
        Rank = rank;
        Name = name;
        Points = points;
        Details = details;
        IsWinner = isWinner;
        Accent = accent;
    }

    public string Rank { get; }

    public string Name { get; }

    public string Points { get; }

    public string Details { get; }

    public bool IsWinner { get; }

    public bool HasRank => Rank.Length > 0;

    public IBrush Accent { get; }
}
