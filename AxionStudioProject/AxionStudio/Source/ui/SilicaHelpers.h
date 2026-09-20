#pragma once

#include <string>
#include <functional>
#include <filesystem>
#include <optional>

#include <Silica/include/SWidget.h>

#include "AxionEngine/Source/core/Math.h"

#include "AxionStudio/Source/ui/EditorTheme.h"

namespace Axion::SilicaHelpers {

	Silica::WidgetPtr MakeDetailRow(const std::string& label, const std::string& value, float labelWidth = EditorTheme::ROW_LABEL_WIDTH);
	Silica::WidgetPtr MakeHeader(const std::string& title);
	Silica::WidgetPtr MakeEmptyState(const std::string& message, float wrapWidth = 250.0f);
	Silica::WidgetPtr MakePropertyRow(const std::string& label, Silica::WidgetPtr valueWidget, float labelWidth = EditorTheme::ROW_LABEL_WIDTH);
	Silica::WidgetPtr MakeAssetDropZone(const std::string& extension, std::function<void(const std::filesystem::path&)> onValidDrop, Silica::WidgetPtr child, Silica::Vec2 padding = { 10.0f, 5.0f });

	Silica::WidgetPtr MakeContextMenuItem(const std::string& text, std::function<void()> onClick, std::optional<Silica::Color> hoverColor = std::nullopt);
	Silica::WidgetPtr MakeOptionsMenuItem(const std::string& text, std::function<void()> onClick, std::optional<Silica::Color> hoverColor = std::nullopt);
	Silica::WidgetPtr MakeCheckboxMenuItem(const std::string& text, bool isChecked, std::function<void(bool)> onToggle);
	Silica::WidgetPtr MakeToolbarBtn(const std::string& text, Silica::Color color, std::function<void()> onClick, float padX = 8.0f, float padY = 4.0f);

	Silica::Color MakeSilicaColor(const Axion::Vec4& color);
	Axion::Vec4 MakeAxionColor(const Silica::Color& color);

}
