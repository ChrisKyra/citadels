using System.Collections.Specialized;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Citadels.App.ViewModels;

namespace Citadels.App.Views;

public partial class GameView : UserControl
{
    private INotifyCollectionChanged? _log;

    public GameView()
    {
        AvaloniaXamlLoader.Load(this);
        DataContextChanged += OnDataContextChanged;
    }

    private void OnDataContextChanged(object? sender, EventArgs e)
    {
        if (_log is not null)
        {
            _log.CollectionChanged -= OnLogChanged;
        }

        _log = (DataContext as GameViewModel)?.Log;
        if (_log is not null)
        {
            _log.CollectionChanged += OnLogChanged;
        }
    }

    // New lines of the log appear at the bottom: keep the newest one in view.
    private void OnLogChanged(object? sender, NotifyCollectionChangedEventArgs e)
    {
        ScrollViewer? scroll = this.FindControl<ScrollViewer>("LogScroll");
        if (scroll is null)
        {
            return;
        }

        Avalonia.Threading.Dispatcher.UIThread.Post(() => scroll.ScrollToEnd());
    }
}
