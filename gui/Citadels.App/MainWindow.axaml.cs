using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Citadels.App.ViewModels;

namespace Citadels.App;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        AvaloniaXamlLoader.Load(this);
        Closing += OnClosing;
    }

    private void OnClosing(object? sender, WindowClosingEventArgs e)
    {
        // Stops the engine process when the window is closed in the middle of a game.
        (DataContext as MainViewModel)?.Shutdown();
    }
}
