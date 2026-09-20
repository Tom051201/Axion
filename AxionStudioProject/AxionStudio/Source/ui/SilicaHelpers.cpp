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
#include <Silica/include/SButton.h>
#include <Silica/include/SCheckbox.h>
#include <Silica/include/Renderer.h>

#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/ui/EditorTheme.h"

namespace Axion::SilicaHelpers {

	Silica::WidgetPtr MakeDetailRow(const std::string& label, const std::string& value, float labelWidth) {
		auto rowBox = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 15.0f,
			.slots = {
				{ {0,0}, Silica::MakeWidget<Silica::SBox>({
					.explicitSize = Silica::Vec2{labelWidth, 0.0f},
					.backgroundColor = Silica::Color::transparent(),
					.child = Silica::MakeWidget<Silica::SAlign>({
						.horizontalAlign = Silica::HorizontalAlign::Left,
						.verticalAlign = Silica::VerticalAlign::Center,
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = label })
					})
				})},
				{ {1,0}, Silica::MakeWidget<Silica::SAlign>({
					.horizontalAlign = Silica::HorizontalAlign::Left,
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = Silica::MakeWidget<Silica::STextBlock>({
						.text = value,
						.color = Silica::GetTheme().Text_Dim
					})
				}) }
			}
		});

		return Silica::MakeWidget<Silica::SBox>({
			.explicitSize = Silica::Vec2(0.0f, EditorTheme::ROW_HEIGHT_DEFAULT),
			.child = rowBox
		});
	}

	Silica::WidgetPtr MakeHeader(const std::string& title) {
		auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = 4.0f });
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
		auto rowBox = Silica::MakeWidget<Silica::SHorizontalBox>({
			.spacing = 10.0f,
			.slots = {
				{ {0, 0}, Silica::MakeWidget<Silica::SBox>({
					.explicitSize = Silica::Vec2(labelWidth, 0.0f),
					.backgroundColor = Silica::Color::transparent(),
					.child = Silica::MakeWidget<Silica::SAlign>({
						.horizontalAlign = Silica::HorizontalAlign::Left,
						.verticalAlign = Silica::VerticalAlign::Center,
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = label })
					})
				})},
				{ {1, 0}, Silica::MakeWidget<Silica::SAlign>({
					.horizontalAlign = Silica::HorizontalAlign::Left,
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = valueWidget
				})}
			}
		});

		return Silica::MakeWidget<Silica::SBox>({
			.explicitSize = Silica::Vec2(0.0f, EditorTheme::ROW_HEIGHT_DEFAULT),
			.child = rowBox
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

	Silica::WidgetPtr MakeContextMenuItem(const std::string& text, std::function<void()> onClick, std::optional<Silica::Color> hoverColor) {
		return Silica::MakeWidget<Silica::SButton>({
			.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
			.color = Silica::Color::transparent(),
			.hoverColor = hoverColor.value_or(Silica::GetTheme().Accent_Primary),
			.onClick = [onClick]() {
				EditorActionQueue::push([onClick]() {
					Silica::Renderer::closePopups();
					if (onClick) onClick();
				});
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Left,
				.verticalAlign = Silica::VerticalAlign::Center,
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = text})
			})
		});
	}

	// NEW: Use this ONLY inside your fixed-width Gear Options menus
	Silica::WidgetPtr MakeOptionsMenuItem(const std::string& text, std::function<void()> onClick, std::optional<Silica::Color> hoverColor) {
		return Silica::MakeWidget<Silica::SButton>({
			.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
			.color = Silica::Color::transparent(),
			.hoverColor = hoverColor.value_or(Silica::GetTheme().Accent_Primary),
			.onClick = [onClick]() {
				EditorActionQueue::push([onClick]() {
					Silica::Renderer::closePopups();
					if (onClick) onClick();
				});
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::SBox>({
				.explicitSize = Silica::Vec2{ EditorTheme::OPTIONS_MENU_WIDTH - (EditorTheme::PADDING_SMALL * 2.0f) - (EditorTheme::BUTTON_PADDING_X * 2.0f), 0.0f },
				.backgroundColor = Silica::Color::transparent(),
				.child = Silica::MakeWidget<Silica::SAlign>({
					.horizontalAlign = Silica::HorizontalAlign::Left,
					.verticalAlign = Silica::VerticalAlign::Center,
					.child = Silica::MakeWidget<Silica::STextBlock>({.text = text})
				})
			})
		});
	}

	Silica::WidgetPtr MakeCheckboxMenuItem(const std::string& text, bool isChecked, std::function<void(bool)> onToggle) {
		return Silica::MakeWidget<Silica::SBox>({
			.padding = { EditorTheme::BUTTON_PADDING_X, EditorTheme::BUTTON_PADDING_Y },
			.explicitSize = Silica::Vec2{ EditorTheme::OPTIONS_MENU_WIDTH - (EditorTheme::PADDING_SMALL * 2.0f), 0.0f },
			.backgroundColor = Silica::Color::transparent(),
			.child = Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 15.0f,
				.slots = {
					{ {0,0}, Silica::MakeWidget<Silica::SBox>({
						.explicitSize = Silica::Vec2{ 180.0f, 0.0f },
						.backgroundColor = Silica::Color::transparent(),
						.child = Silica::MakeWidget<Silica::SAlign>({
							.horizontalAlign = Silica::HorizontalAlign::Left,
							.verticalAlign = Silica::VerticalAlign::Center,
							.child = Silica::MakeWidget<Silica::STextBlock>({.text = text})
						})
					})},
					{ {0,0}, Silica::MakeWidget<Silica::SAlign>({
						.horizontalAlign = Silica::HorizontalAlign::Right,
						.verticalAlign = Silica::VerticalAlign::Center,
						.child = Silica::MakeWidget<Silica::SCheckBox>({
							.initialCheck = isChecked,
							.onCheckChanged = [onToggle](bool val) {
								EditorActionQueue::push([onToggle, val]() {
									if (onToggle) onToggle(val);
								});
							}
						})
					})}
				}
			})
		});
	}

	Silica::WidgetPtr MakeToolbarBtn(const std::string& text, Silica::Color color, std::function<void()> onClick, float padX, float padY) {
		return Silica::MakeWidget<Silica::SAlign>({
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = Silica::MakeWidget<Silica::SButton>({
				.padding = { padX, padY },
				.color = color,
				.onClick = [onClick]() {
					if (onClick) onClick();
					return Silica::EventReply::handled();
				},
				.child = Silica::MakeWidget<Silica::STextBlock>({.text = text })
			})
		});
	}

	Silica::Color MakeSilicaColor(const Axion::Vec4& c) {
		return Silica::Color((uint8_t)(c.x * 255), (uint8_t)(c.y * 255), (uint8_t)(c.z * 255), (uint8_t)(c.w * 255));
	}

	Axion::Vec4 MakeAxionColor(const Silica::Color& c) {
		return Vec4(c.r() / 255.0f, c.g() / 255.0f, c.b() / 255.0f, c.a() / 255.0f);
	}

}
