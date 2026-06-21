#include "SystemInfoModal.h"

#include "AxionEngine/Source/render/GraphicsContext.h"
#include "AxionEngine/Source/render/Renderer.h"
#include "AxionEngine/Source/core/PlatformUtils.h"

#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SAlign.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"
#include "AxionStudio/Source/core/EditorModalManager.h"

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

	Silica::WidgetPtr SystemInfoModal::getWidget(Silica::FontAtlas* font) {
		m_font = font;

		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
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

		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 15.0f });


		// -- Helper Functions --
		auto MakeHeader = [&](const std::string& title) {
			auto box = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 4.0f });
			box->addSlot({ {0,0}, Silica::MakeWidget<Silica::STextBlock>({.text = title, .color = Silica::Color(100, 200, 255, 255), .font = m_font}) });
			box->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({.explicitSize = Silica::Vec2{0, 1}, .backgroundColor = Silica::Color(80, 80, 80, 255)}) });
			return box;
		};

		auto MakePropertyRow = [&](const std::string& label, const std::string& value) {
			return Silica::MakeWidget<Silica::SHorizontalBox>({
				.spacing = 10.0f,
				.slots = {
					{ {0, 0}, Silica::MakeWidget<Silica::SBox>({
						.explicitSize = Silica::Vec2(100.0f, 0.0f),
						.backgroundColor = Silica::Color::transparent(),
						.child = Silica::MakeWidget<Silica::STextBlock>({.text = label, .color = Silica::Color(180, 180, 180, 255), .font = m_font})
					})},
					{ {1, 0}, Silica::MakeWidget<Silica::STextBlock>({.text = value, .font = m_font}) }
				}
			});
		};


		// -- GPU --
		contentBox->addSlot({ {0,0}, MakeHeader("GPU") });
		contentBox->addSlot({ {0,0}, MakePropertyRow("GPU:", m_gpuName) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("VRAM:", std::to_string(m_vramMB) + " MB") });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Driver:", m_gpuDriverVersion) });


		// -- CPU --
		contentBox->addSlot({ {0,0}, MakeHeader("CPU") });
		contentBox->addSlot({ {0,0}, MakePropertyRow("CPU:", m_cpuName) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("Cores:", std::to_string(m_cores)) });
		contentBox->addSlot({ {0,0}, MakePropertyRow("RAM:", std::to_string(m_totalRamMB) + " MB") });


		// -- Snapshot Of Current Performance --
		contentBox->addSlot({ {0,0}, MakePropertyRow("Frame Time:", std::to_string(Renderer::getFrameTimeMs()) + " ms") });
		contentBox->addSlot({ {0,0}, MakePropertyRow("FPS:", std::to_string((int)(1000.0 / Renderer::getFrameTimeMs())) + " FPS") });


		// -- OS --
		contentBox->addSlot({ {0,0}, MakeHeader("Operating System") });
		contentBox->addSlot({ {0,0}, MakePropertyRow("OS:", m_os) });


		// -- Footer Buttons --
		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SBox>({.explicitSize = Silica::Vec2{0, 1}, .backgroundColor = Silica::Color(60, 60, 60, 255)}) });

		auto closeBtn = Silica::MakeWidget<Silica::SButton>({
			.padding = { 30.0f, 8.0f }, .color = Silica::Color(80, 80, 80, 255),
			.onClick = []() {
				EditorModalManager::close();
				return Silica::EventReply::handled();
			},
			.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Close", .font = m_font})
		});

		contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SAlign>({
			.horizontalAlign = Silica::HorizontalAlign::Right,
			.child = closeBtn
		}) });


		// -- Assemble Modal --
		auto modalPanel = Silica::MakeWidget<Silica::SBox>({
			.padding = { 20.0f, 20.0f },
			.explicitSize = Silica::Vec2{ 500.0f, 0.0f },
			.backgroundColor = Silica::Color(30, 30, 30, 255),
			.child = contentBox
		});

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SAlign>({
			.horizontalAlign = Silica::HorizontalAlign::Center,
			.verticalAlign = Silica::VerticalAlign::Center,
			.child = modalPanel
		}));
	}

}
