using Avalonia.Media;
using Citadels.App.Infrastructure;

namespace Citadels.App.ViewModels;

/// <summary>One line of the setup screen: the avatar and the text box for a player's name.</summary>
public sealed class NameEntry : ObservableObject
{
    private string _name;

    public NameEntry(int seat, string name)
    {
        Seat = seat;
        Label = "Player " + (seat + 1);
        SeatBrush = Palette.Seat(seat);
        _name = name;
    }

    public int Seat { get; }

    public string Label { get; }

    public IBrush SeatBrush { get; }

    public string Name
    {
        get => _name;
        set
        {
            if (SetProperty(ref _name, value ?? string.Empty))
            {
                OnPropertyChanged(nameof(Initial));
            }
        }
    }

    public string Initial
    {
        get
        {
            string trimmed = _name.Trim();
            return trimmed.Length > 0 ? trimmed.Substring(0, 1).ToUpperInvariant() : "?";
        }
    }
}
