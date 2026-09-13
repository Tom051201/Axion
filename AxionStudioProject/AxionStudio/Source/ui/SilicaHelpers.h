#pragma once

#include <string>

#include <Silica/include/SWidget.h>

namespace Axion::SilicaHelpers {

	Silica::WidgetPtr MakeDetailRow(const std::string& label, const std::string& value, float labelWidth = 140.0f);
	Silica::WidgetPtr MakeHeader(const std::string& title);
	Silica::WidgetPtr MakeEmptyState(const std::string& message, float wrapWidth = 250.0f);
	Silica::WidgetPtr MakePropertyRow(const std::string& label, Silica::WidgetPtr valueWidget, float labelWidth = 100.0f);
	Silica::WidgetPtr MakeAssetDropZone(const std::string& extension, std::function<void(const std::filesystem::path&)> onValidDrop, Silica::WidgetPtr child, Silica::Vec2 padding = { 10.0f, 5.0f });

}
