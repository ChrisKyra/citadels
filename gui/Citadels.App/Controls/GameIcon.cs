using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;

namespace Citadels.App.Controls;

/// <summary>
/// Draws one of the <see cref="Icons"/> shapes (a 512 x 512 design) scaled into its own size,
/// centred, in one colour. Always give it a Width and a Height.
/// </summary>
public sealed class GameIcon : Control
{
    private const double DesignSize = 512.0;

    public static readonly StyledProperty<Geometry?> DataProperty =
        AvaloniaProperty.Register<GameIcon, Geometry?>(nameof(Data));

    public static readonly StyledProperty<IBrush?> ForegroundProperty =
        AvaloniaProperty.Register<GameIcon, IBrush?>(nameof(Foreground), Brushes.White);

    static GameIcon()
    {
        AffectsRender<GameIcon>(DataProperty, ForegroundProperty);
    }

    public Geometry? Data
    {
        get => GetValue(DataProperty);
        set => SetValue(DataProperty, value);
    }

    public IBrush? Foreground
    {
        get => GetValue(ForegroundProperty);
        set => SetValue(ForegroundProperty, value);
    }

    public override void Render(DrawingContext context)
    {
        Geometry? data = Data;
        IBrush? brush = Foreground;
        double width = Bounds.Width;
        double height = Bounds.Height;
        if (data is null || brush is null || width <= 0 || height <= 0)
        {
            return;
        }

        double scale = Math.Min(width, height) / DesignSize;
        double offsetX = (width - DesignSize * scale) / 2;
        double offsetY = (height - DesignSize * scale) / 2;
        Matrix transform = Matrix.CreateScale(scale, scale) * Matrix.CreateTranslation(offsetX, offsetY);
        using (context.PushTransform(transform))
        {
            context.DrawGeometry(brush, null, data);
        }
    }
}
