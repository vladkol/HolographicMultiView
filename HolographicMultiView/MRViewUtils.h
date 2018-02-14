#pragma once

#include <ppltasks.h>

inline concurrency::task<int> GetViewIdAsync(Windows::ApplicationModel::Core::CoreApplicationView^ view)
{
	Platform::Agile<Windows::ApplicationModel::Core::CoreApplicationView> agileView(view);
	std::vector<int>* views = new std::vector<int>();

	return concurrency::create_task(
		view->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High,
			ref new Windows::UI::Core::DispatchedHandler([agileView, views]()
	{
		int resultId = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->Id;
		agileView->CoreWindow->Activate();

		views->push_back(resultId);

	}))).then([views] () -> int
	{
		return views->at(0);
	});;
}

inline concurrency::task<bool> SwitchToViewAsync(int viewId, bool showAsStandalone = false)
{
	if (showAsStandalone)
	{
		return concurrency::create_task(Windows::UI::ViewManagement::ApplicationViewSwitcher::TryShowAsStandaloneAsync(viewId));
	}
	else
	{
		auto currentViewWindow = Windows::UI::Core::CoreWindow::GetForCurrentThread();
		int currentViewId = Windows::UI::ViewManagement::ApplicationView::GetApplicationViewIdForWindow(currentViewWindow);

		return concurrency::create_task(Windows::UI::ViewManagement::ApplicationViewSwitcher::SwitchAsync(viewId, currentViewId)).then([]() -> bool
		{
			return true;
		});
	}

}


