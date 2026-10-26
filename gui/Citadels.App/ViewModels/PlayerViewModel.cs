using System.Collections.ObjectModel;
using Avalonia.Media;
using Citadels.App.Infrastructure;
using Citadels.Client;

namespace Citadels.App.ViewModels;

/// <summary>
/// One player's public panel on the table. It is updated in place, so the cards that are already
/// on screen stay there and only new districts fade in.
/// </summary>
public sealed class PlayerViewModel : ObservableObject
{
    public const int DistrictsToFinish = 8;

    private int _gold;
    private int _handSize;
    private bool _hasCrown;
    private bool _isActive;

    public PlayerViewModel(PlayerSummary summary, bool hasCrown, bool isActive)
    {
        Seat = summary.Seat;
        Name = summary.Name;
        Initial = Name.Length > 0 ? Name.Substring(0, 1).ToUpperInvariant() : "?";
        SeatBrush = Palette.Seat(Seat);
        Update(summary, hasCrown, isActive);
    }

    public int Seat { get; }

    public string Name { get; }

    public string Initial { get; }

    public IBrush SeatBrush { get; }

    public ObservableCollection<CardViewModel> City { get; } = new();

    public int Gold
    {
        get => _gold;
        private set => SetProperty(ref _gold, value);
    }

    public int HandSize
    {
        get => _handSize;
        private set => SetProperty(ref _handSize, value);
    }

    public bool HasCrown
    {
        get => _hasCrown;
        private set => SetProperty(ref _hasCrown, value);
    }

    /// <summary>It is this player's turn (or pick in the draft).</summary>
    public bool IsActive
    {
        get => _isActive;
        set => SetProperty(ref _isActive, value);
    }

    public int DistrictCount => City.Count;

    public string DistrictText => $"{City.Count}/{DistrictsToFinish}";

    public bool IsCityEmpty => City.Count == 0;

    public void Update(PlayerSummary summary, bool hasCrown, bool isActive)
    {
        Gold = summary.Gold;
        HandSize = summary.HandSize;
        HasCrown = hasCrown;
        IsActive = isActive;
        SyncCity(summary.City);
    }

    private void SyncCity(IReadOnlyList<Card> cards)
    {
        int same = 0;
        while (same < City.Count && same < cards.Count && City[same].SameCardAs(cards[same]))
        {
            same++;
        }

        // Districts that changed (destroyed by the Warlord, for example) and everything after them go.
        while (City.Count > same)
        {
            City.RemoveAt(City.Count - 1);
        }

        for (int i = same; i < cards.Count; i++)
        {
            City.Add(new CardViewModel(cards[i], large: false));
        }

        OnPropertyChanged(nameof(DistrictCount));
        OnPropertyChanged(nameof(DistrictText));
        OnPropertyChanged(nameof(IsCityEmpty));
    }
}
