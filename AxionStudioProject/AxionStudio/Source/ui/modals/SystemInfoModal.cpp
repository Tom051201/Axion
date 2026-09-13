#include "studiopch.h"
#include "SystemInfoModal.h"

#include <Silica/include/Theme.h>
#include <Silica/include/SBox.h>
#include <Silica/include/SHorizontalBox.h>
#include <Silica/include/SVerticalBox.h>
#include <Silica/include/SButton.h>
#include <Silica/include/STextBlock.h>
#include <Silica/include/SAlign.h>
#include <Silica/include/SSeparator.h>

#include "AxionEngine/Source/core/PlatformUtils.h"
#include "AxionEngine/Source/graphics/GraphicsContext.h"
#include "AxionEngine/Source/graphics/Renderer.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/EditorModalManager.h"

namespace {
	constexpr float MODAL_WIDTH = 500.0f;
	constexpr float LABEL_WIDTH = 100.0f;
	constexpr float SPACING_LARGE = 15.0f;
	constexpr float SPACING_SMALL = 4.0f;
	constexpr float PADDING_LARGE = 20.0f;
}

namespace Axion {

	SystemInfoModal::SystemInfoModal() {
		m_gpuName = GraphicsContext::get()->getGpuName();
		m_gpuDriverVersion = GraphicsContext::get()->getGpuDriverVersion();
		m_vramMB = GraphicsContext::get()->getVramMB();
		m_cpuName = PlatformInfo::getCpuName();
		m_cores = PlatformInfo::getCpuCores();
		m_totalRamMB = PlatformInfo::getRamMB();
		m_os = PlatformInfo::getOsVersion();
	}

	Silica::WidgetPtr SystemInfoModal::getWidget() {
		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.consumePointerEvents = true,
				.backgroundColor = Silica::Color(0, 0, 0, 180)
			});
			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void SystemInfoModal::rebuildUI() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;

		EditorActionQueue::push([this]() {
			m_rebuildQueued = false;
			rebuildUI_Internal();
		});
	}

	void SystemInfoModal::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = SPACING_LARGE });

		// -- Helper Functions --
		auto MakeHeader = [&](const std::string& title) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({.spacing = SPACING_SMALL });
			box->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({
				.text = title,
				.color = Silica::GetTheme().Accent_Primary
			}) });
			box->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) });
			return box;
		};

		auto MakePropertyRow = [&](const std::string& label, const std::string& value) {
			return Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {0, 0}, Silica::MakeWidget<Silica::SBox>({
						.explicitSize = Silica::Vec2(LABEL_WIDTH, 0.0f),
						.backgroundColor = Silica::Color::transparent(),
						.child = Silica::MakeWidget<Silica::STextBlock>({
							.text = label,
							.color = Silica::GetTheme().Text_Dim
						})
					})},
					{ {1, 0}, Silica::MakeWidget<Silica::STextBlock>({.text = value }) }
				}
			});
		};

		auto formatMemory = [](uint64_t mb) {
			if (mb >= 1024) {
				char buf[32];
				std::snprintf(buf, sizeof(buf), "%.1f GB", mb / 1024.0f);
				return std::string(buf);
			}
			return std::to_string(mb) + " MB";
		};


		// -- System Information Layout --

		// GPU
		contentBox->addSlot({ {0,0}, MakeHeader("GPU") });
		contentBox->addSlot({ {0,0}, MakePropertyRow("GPU:", m_gpuName) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("VRAM:", formatMemory(m_vramMB)) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Driver:", m_gpuDriverVersion) });

		// CPU
		contentBox->addSlot({ {0,0}, MakeHeader("CPU") });
		contentBox->addSlot({ {0,0}, MakePropertyRow("CPU:", m_cpuName) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Cores:", std::to_string(m_cores)) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("RAM:", formatMemory(m_totalRamMB)) });

		// Performance
		contentBox->addSlot({ {0,0}, MakeHeader("Performance Snapshot") });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Frame Time:", std::to_string(Renderer::getFrameTimeMs()) + " ms") });
		contentBox->addSlot({ {0,0}, MakePropertyRow("FPS:", std::to_string((int)(1000.0 / Renderer::getFrameTimeMs())) + " FPS") });

		// OS
		contentBox->addSlot({ {0,0}, MakeHeader("Operating System") });
		contentBox->addSlot({ {0,0}, MakePropertyRow("OS:", m_os) });


		// -- Footer Buttons --
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SSeparator>({}) });

		auto closeBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { 30.0f, 8.0f },
			.onClick = []() {
				EditorModalManager::close();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Close" })
		});

		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SAlign>({
			.horizontalAlign = Silica::HorizontalAlign::Right,
			.child = closeBtn
		}) });


		// -- Assemble Modal --
		auto modalPanel = Silica::MakeWidget<Silica::SBox>({
			.explicitSize = Silica::Vec2{ MODAL_WIDTH, 0.0f },
			.borderThickness = Silica::GetTheme().Border_Thickness,
			.backgroundColor = Silica::GetTheme().Background_Panel,
			.child = Silica::MakeWidget<Silica::SBox>({
				.padding = { PADDING_LARGE, PADDING_LARGE },
				.backgroundColor = Silica::Color::transparent(),
				.child = contentBox
			})
		});

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SAlign>({
			.horizontalAlign = Silica::HorizontalAlign::Center,
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = modalPanel
		}));
	}

}
