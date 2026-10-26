using Avalonia;
using Avalonia.Media;
using Citadels.Client;

namespace Citadels.App.ViewModels;

/// <summary>
/// One district card as it is drawn: a tile in the colour of the district with its icon as a
/// watermark, the cost in a gold gem and the name in Cinzel. Large cards are the hand, small ones the cities.
/// </summary>
public sealed class CardViewModel
{
    public CardViewModel(Card card, bool large)
    {
        Palette.DistrictStyle style = Palette.For(card.Color);
        Name = card.Name;
        Cost = card.Cost;
        Ability = card.Ability;
        ColorLabel = style.Label;
        Fill = style.Fill;
        Ink = style.Ink;
        Icon = style.Icon;
        Glow = new BoxShadows(new BoxShadow
        {
            OffsetY = large ? 12 : 6,
            Blur = large ? 26 : 14,
            Spread = -8,
            Color = Color.FromArgb(0xAA, style.GlowColor.R, style.GlowColor.G, style.GlowColor.B),
        });

        Width = large ? 116 : 66;
        Height = large ? 160 : 92;
        IconSize = large ? 74 : 42;
        GemSize = large ? 30 : 19;
        GemFontSize = large ? 15 : 10.5;
        NameFontSize = large ? 13 : 9.5;
        NameMargin = large ? new Thickness(6, 0, 6, 10) : new Thickness(3, 0, 3, 6);
        Corner = new CornerRadius(large ? 14 : 10);
        GemCorner = new CornerRadius(GemSize / 2);
        GemMargin = new Thickness(large ? 7 : 4);
    }

    public string Name { get; }

    public int Cost { get; }

    public string? Ability { get; }

    public bool HasAbility => Ability is not null;

    public string ColorLabel { get; }

    public IBrush Fill { get; }

    public IBrush Ink { get; }

    public Geometry Icon { get; }

    public BoxShadows Glow { get; }

    public double Width { get; }

    public double Height { get; }

    public double IconSize { get; }

    public double GemSize { get; }

    public double GemFontSize { get; }

    public double NameFontSize { get; }

    public Thickness NameMargin { get; }

    public CornerRadius Corner { get; }

    public CornerRadius GemCorner { get; }

    public Thickness GemMargin { get; }

    /// <summary>Shown when the mouse rests on the card.</summary>
    public string ToolTipText =>
        Ability is null
            ? $"{Name}\n{ColorLabel} district, cost {Cost}"
            : $"{Name}\n{ColorLabel} district, cost {Cost}\n\n{Ability}";

    /// <summary>True when both describe the same card (used to keep cards on screen between updates).</summary>
    public bool SameCardAs(Card card) =>
        card.Name == Name && card.Cost == Cost && card.Ability == Ability && Palette.For(card.Color).Label == ColorLabel;
}
