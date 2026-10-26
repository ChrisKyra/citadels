using Avalonia.Controls;
using Avalonia.Markup.Xaml;

namespace Citadels.App.Views;

public partial class CardView : UserControl
{
    public CardView()
    {
        AvaloniaXamlLoader.Load(this);
    }
}
