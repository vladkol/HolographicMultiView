//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "SlateUIPage.xaml.h"
#include "AppView.h"
#include "MRViewUtils.h"

#include <string>
#include <ppltasks.h>

using namespace HolographicXAMLView;

using namespace Platform;
using namespace Concurrency;
using namespace Microsoft::WRL;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Media::Imaging;
using namespace Windows::UI::Xaml::Navigation;

SlateUIPage::SlateUIPage() : m_switched(false), m_closed(false)
{
	InitializeComponent();
}

void SlateUIPage::Page_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (!Windows::Graphics::Holographic::HolographicSpace::IsSupported)
	{
		UpdateHolographicState(HolographicState::NotSupported);
	}
	else
	{
		UpdateHolographicState(
			Windows::Graphics::Holographic::HolographicSpace::IsAvailable ? 
			HolographicState::Available : HolographicState::NotAvailable);

		Windows::Graphics::Holographic::HolographicSpace::IsAvailableChanged += ref new Windows::Foundation::EventHandler<Platform::Object ^>(this, &SlateUIPage::OnIsAvailableChanged);
		isInImmersive->Visibility = Windows::ApplicationModel::Preview::Holographic::HolographicApplicationPreview::IsCurrentViewPresentedOnHolographicDisplay() ? 
			Windows::UI::Xaml::Visibility::Visible : Windows::UI::Xaml::Visibility::Collapsed;
	}

	// Handle closing primary (desktop) view. Once it's closed, we will close the immersive one too.
	Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->Consolidated += 
		ref new TypedEventHandler<Windows::UI::ViewManagement::ApplicationView^, Windows::UI::ViewManagement::ApplicationViewConsolidatedEventArgs^>(this, &SlateUIPage::OnConsolidatedOrClosed);
}


void SlateUIPage::OnIsAvailableChanged(Platform::Object ^sender, Platform::Object ^args)
{
	UpdateHolographicState(
		Windows::Graphics::Holographic::HolographicSpace::IsAvailable ?
		HolographicState::Available : HolographicState::NotAvailable);

	Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([this]()
	{
		eventLog->Text = "HMD connection status changed.";
	}));
}


void SlateUIPage::OnConsolidatedOrClosed(Windows::UI::ViewManagement::ApplicationView^, Windows::UI::ViewManagement::ApplicationViewConsolidatedEventArgs^)
{
	// Handle closing primary (desktop) view. Once it's closed, close the immersive one too.

	m_closed = true;
	if (m_immersiveView != nullptr)
	{
		m_immersiveView->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([this]()
		{
			// Closing the immersive view, so we are back to "Cliff House" 
			Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->TryConsolidateAsync();
		}));
	}
}

// If the user goes back to "Cliff House", the immersive view becomes hidden.  
void SlateUIPage::OnImmersiveViewWindowVisibilityChanged(Windows::UI::Core::CoreWindow ^ sender, Windows::UI::Core::VisibilityChangedEventArgs ^ args)
{
	if (!args->Visible && !m_closed)
	{
		Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([this]()
		{
			eventLog->Text = "Immersive view is hidden.";
		}));
	}
}

// Immersive shell may close the immersive view. Let's handle it. 
void SlateUIPage::OnImmersiveViewConsolidatedOrClosed(Windows::UI::ViewManagement::ApplicationView^, Windows::UI::ViewManagement::ApplicationViewConsolidatedEventArgs^)
{
	if (!m_closed)
	{
		m_switched = false;
		m_immersiveView = nullptr;
		UpdateHolographicState(HolographicState::Available);

		Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([this]()
		{
			eventLog->Text = "Immersive view was closed.";
		}));
	}
}

void SlateUIPage::ShowMixedRealityView()
{
	if (m_immersiveView == nullptr)
	{
		auto appViewSource = ref new HolographicMultiView::AppViewSource();
		m_immersiveView = Windows::ApplicationModel::Core::CoreApplication::CreateNewView(appViewSource);

		m_immersiveView->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([this]()
		{
			m_immersiveView->CoreWindow->VisibilityChanged +=
				ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &SlateUIPage::OnImmersiveViewWindowVisibilityChanged);

			Windows::UI::ViewManagement::ApplicationView::GetForCurrentView()->Consolidated +=
				ref new TypedEventHandler<Windows::UI::ViewManagement::ApplicationView^, Windows::UI::ViewManagement::ApplicationViewConsolidatedEventArgs^>(this, &SlateUIPage::OnImmersiveViewConsolidatedOrClosed);
		}));
	}

	if (m_immersiveView != nullptr)
	{
		GetViewIdAsync(m_immersiveView.Get()).then([this](int id)
		{
			if (id != 0)
			{
				SwitchToViewAsync(id, true).then([this](bool switched)
				{
					if (switched)
					{
						m_switched = true;
						UpdateHolographicState(HolographicState::Available);

						eventLog->Text = "";
					}
				});
			}
		});
	}
}

void SlateUIPage::UpdateHolographicState(HolographicState state)
{
	Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([this, state]()
	{
		if (m_switched)
		{
			warningText->Text = "Mixed Reality view has been activated.";
			showMRButton->IsEnabled = true;
		}
		else
		{
			switch (state)
			{
			case HolographicState::NotSupported:
				warningText->Text = "Windows Mixed Reality is not supported on your PC.";
				showMRButton->IsEnabled = false;
				break;
			case HolographicState::NotAvailable:
				warningText->Text = "You don't have a Mixed Reality HMD connected. Connect your HMD, then we switch views.";
				showMRButton->IsEnabled = false;
				break;
			case HolographicState::Available:
				warningText->Text = "You can switch to Mixed Reality!";
				showMRButton->IsEnabled = true;
				break;
			}
		}
	}));

}


void SlateUIPage::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	ShowMixedRealityView();
}





