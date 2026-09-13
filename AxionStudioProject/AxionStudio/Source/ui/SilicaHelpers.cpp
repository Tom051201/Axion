#include "studiopch.h"
#include "SilicaHelpers.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SSeparator.h>
#include <Silica/include/SWrappedTextBlock.h>
#include <Silica/include/SAlign.h>
#include <Silica/include/SScissorBox.h>

namespace Axion::SilicaHelpers {

	Silica::WidgetPtr MakeDetailRow(const std::string& label, const std::string& value, float labelWidth) {
		return Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 15.0f,
			.slots = {
				{ {0,0}, Silica::MakeWidget<Silica::SBox>({
					.explicitSize = Silica::Vec2{labelWidth, 0.0f},
					.backgroundColor = Silica::Color::transparent(),
					.child = Silica::MakeWidget<Silica::STextBlock>({
						.text = label,
						.color = Silica::GetTheme().Text_Dim
					})
				})},
				{ {1,0}, Silica::MakeWidget<Silica::STextBlock>({.text = value }) }
			}
		});
	}

	Silica::WidgetPtr MakeHeader(const std::string& title) {
		auto box = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 4.0f });
		box->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
			.text = title,
			.color = Silica::GetTheme().Accent_Primary
		}) });
		box->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) });
		return box;
	}

	Silica::WidgetPtr MakeEmptyState(const std::string& message, float wrapWidth) {
		auto emptyText = Silica::MakeWidget<Silica::SWrappedTextBlock>({
			.text = message,
			.wrapWidth = wrapWidth,
			.color = Silica::GetTheme().Text_Dim
		});

		auto centeredState = Silica::MakeWidget<Silica::SAlign>({
			.horizontalAlign = Silica::HorizontalAlign::Center,
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = emptyText
		});

		return Silica::MakeWidget<Silica::SScissorBox>({ .child = centeredState });
	}

	Silica::WidgetPtr MakePropertyRow(const std::string& label, Silica::WidgetPtr valueWidget, float labelWidth) {
		return Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 10.0f,
			.slots = {
				{ {0, 0}, Silica::MakeWidget<Silica::SBox>({
					.explicitSize = Silica::Vec2(labelWidth, 0.0f),
					.backgroundColor = Silica::Color::transparent(),
					.child = Silica::MakeWidget<Silica::SAlign>({
						.verticalAlign = Silica::VerticalAlign::Center,
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = label })
					})
				})},
				{ {1, 0}, valueWidget }
			}
		});
	}

	Silica::WidgetPtr MakeAssetDropZone(const std::string& extension, std::function<void(const std::filesystem::path&)> onValidDrop, Silica::WidgetPtr child, Silica::Vec2 padding) {
		return Silica::MakeWidget<Silica::SBox>({
			.padding = padding,
			.onDragOver = [extension](const Silica::DragDropPayload& payload) {
				if (payload.type == "AssetPath" && std::any_cast<std::filesystem::path>(payload.data).extension() == extension) return Silica::EventReply::handled();
				return Silica::EventReply::unhandled();
			},
			.onDrop = [extension, onValidDrop](const Silica::DragDropPayload& payload) {
				if (payload.type == "AssetPath") {
					auto path = std::any_cast<std::filesystem::path>(payload.data);
					if (path.extension() == extension) {
						onValidDrop(path);
						return Silica::EventReply::handled();
					}
				}
				return Silica::EventReply::unhandled();
			},
			.child = child
		});
	}

}
