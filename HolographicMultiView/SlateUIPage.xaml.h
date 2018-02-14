//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "SlateUIPage.g.h"
#include "Common/StepTimer.h"
#include <vector>
#include <ppltasks.h>
#include <functional>
#include <collection.h>

namespace HolographicXAMLView
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class SlateUIPage sealed
	{
	private:
		enum class HolographicState
		{
			NotSupported = 0, 
			NotAvailable = 1, 
			Available = 2
		};
	public:
		SlateUIPage();

	private:
		void ShowMixedRealityView();
		void UpdateHolographicState(HolographicState state);

		void Page_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnIsAvailableChanged(Platform::Object ^sender, Platform::Object ^args);

		void Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
