//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "SlateUIPage.xaml.h"
#include "AppView.h"
#include "MRViewUtils.h"

#include <string>
#include <sstream> 
#include <algorithm>
#include <ppltasks.h>
#include <robuffer.h> // IBufferByteAccess


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

SlateUIPage::SlateUIPage()
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
	}
}


void SlateUIPage::OnIsAvailableChanged(Platform::Object ^sender, Platform::Object ^args)
{
	UpdateHolographicState(
		Windows::Graphics::Holographic::HolographicSpace::IsAvailable ?
		HolographicState::Available : HolographicState::NotAvailable);
}

void SlateUIPage::ShowMixedRealityView()
{
	auto appViewSource = ref new HolographicMultiView::AppViewSource();
	Windows::ApplicationModel::Core::CoreApplicationView^ view = Windows::ApplicationModel::Core::CoreApplication::CreateNewView(appViewSource);
	
	GetViewIdAsync(view).then([this](int id)
	{
		SwitchToViewAsync(id, true);
	});
}

void SlateUIPage::UpdateHolographicState(HolographicState state)
{
	Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::High, ref new Windows::UI::Core::DispatchedHandler([this, state]()
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
	}));

}


void SlateUIPage::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	ShowMixedRealityView();
}


