using Avalonia;
using Avalonia.Media;
using Citadels.App.Controls;
using Citadels.Client;

namespace Citadels.App.ViewModels;

/// <summary>Colours and icons shared by the view models (the XAML side is in Styles/Theme.axaml).</summary>
public static class Palette
{
    public static readonly IBrush Gold = Solid("#F2C14E");
    public static readonly IBrush GoldText = Solid("#F5D58A");
    public static readonly IBrush Muted = Solid("#9AA3C0");
    public static readonly IBrush DarkInk = Solid("#2A1C05");
    public static readonly IBrush LightInk = Brushes.White;

    // One colour per seat, for avatars.
    private static readonly IBrush[] SeatBrushes =
    {
        Solid("#F59E0B"), Solid("#38BDF8"), Solid("#A78BFA"), Solid("#34D399"),
        Solid("#F472B6"), Solid("#FB923C"), Solid("#94A3B8"),
    };

    public static IBrush Seat(int seat) => SeatBrushes[((seat % SeatBrushes.Length) + SeatBrushes.Length) % SeatBrushes.Length];

    /// <summary>The look of one district colour: gradient, ink for text and icon, glow, icon, label.</summary>
    public sealed record DistrictStyle(IBrush Fill, IBrush Ink, IBrush Accent, Color GlowColor, Geometry Icon, string Label);

    private static readonly DistrictStyle NobleStyle = Make("#F6D365", "#C9932B", DarkInk, Icons.Castle, "Noble");
    private static readonly DistrictStyle ReligiousStyle = Make("#6E9BFF", "#2D4FB8", LightInk, Icons.Church, "Religious");
    private static readonly DistrictStyle TradeStyle = Make("#5BE08F", "#15803D", Solid("#062B14"), Icons.Shop, "Trade");
    private static readonly DistrictStyle MilitaryStyle = Make("#FF8A8A", "#B91C1C", LightInk, Icons.Watchtower, "Military");
    private static readonly DistrictStyle SpecialStyle = Make("#D29BFF", "#7E22CE", LightInk, Icons.CrystalBall, "Special");

    public static DistrictStyle For(CardColor color) => color switch
    {
        CardColor.Noble => NobleStyle,
        CardColor.Religious => ReligiousStyle,
        CardColor.Trade => TradeStyle,
        CardColor.Military => MilitaryStyle,
        _ => SpecialStyle,
    };

    /// <summary>The icon of a character, or null when the text is not a character name.</summary>
    public static Geometry? CharacterIcon(string name) => name switch
    {
        "Assassin" => Icons.Hood,
        "Thief" => Icons.Robber,
        "Magician" => Icons.PointyHat,
        "King" => Icons.Crown,
        "Bishop" => Icons.Abbot,
        "Merchant" => Icons.Scales,
        "Architect" => Icons.HammerNails,
        "Warlord" => Icons.CrossedSwords,
        _ => null,
    };

    public static IBrush Solid(string hex) => new SolidColorBrush(Color.Parse(hex));

    private static DistrictStyle Make(string light, string dark, IBrush ink, Geometry icon, string label)
    {
        var fill = new LinearGradientBrush
        {
            StartPoint = new RelativePoint(0.15, 0, RelativeUnit.Relative),
            EndPoint = new RelativePoint(0.85, 1, RelativeUnit.Relative),
        };
        fill.GradientStops.Add(new GradientStop(Color.Parse(light), 0));
        fill.GradientStops.Add(new GradientStop(Color.Parse(dark), 1));
        return new DistrictStyle(fill, ink, Solid(light), Color.Parse(dark), icon, label);
    }
}
