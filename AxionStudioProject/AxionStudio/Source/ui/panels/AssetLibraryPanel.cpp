#include "AssetLibraryPanel.h"

#include "AxionEngine/Source/project/ProjectManager.h"
#include "AxionEngine/Source/core/Logging.h"

#include "AxionStudio/Source/core/EditorActionQueue.h"

#include "AxionStudio/Vendor/Silica/include/SBorderLayout.h"
#include "AxionStudio/Vendor/Silica/include/SHorizontalBox.h"
#include "AxionStudio/Vendor/Silica/include/SVerticalBox.h"
#include "AxionStudio/Vendor/Silica/include/SButton.h"
#include "AxionStudio/Vendor/Silica/include/STextBlock.h"
#include "AxionStudio/Vendor/Silica/include/SScrollBox.h"
#include "AxionStudio/Vendor/Silica/include/SAlign.h"
#include "AxionStudio/Vendor/Silica/include/Theme.h"
#include "AxionStudio/Vendor/Silica/include/SBox.h"

namespace Axion {

	AssetLibraryPanel::AssetLibraryPanel() {
		m_engineDefaultAssetsPath = std::filesystem::current_path() / "AxionStudio" / "Resources" / "DefaultAssets";

		if (!std::filesystem::exists(m_engineDefaultAssetsPath)) {
			std::filesystem::create_directories(m_engineDefaultAssetsPath);
		}
	}

	Silica::WidgetPtr AssetLibraryPanel::getWidget() {

		if (!m_uiRoot) {
			m_uiRoot = Silica::MakeWidget<Silica::SBox>({
				.borderThickness = Silica::GetTheme().Border_Thickness,
				.backgroundColor = Silica::GetTheme().Background_Panel
			});
			rebuildUI_Internal();
		}
		return m_uiRoot;
	}

	void AssetLibraryPanel::rebuildUI() {
		if (m_rebuildQueued) return;
		m_rebuildQueued = true;

		EditorActionQueue::push([this]() {
			m_rebuildQueued = false;
			rebuildUI_Internal();
		});
	}

	void AssetLibraryPanel::rebuildUI_Internal() {
		if (!m_uiRoot) return;

		// -- Top Bar --
		auto topBarBox = Silica::MakeWidget<Silica::SBox>({
			.padding = { 10.0f, 10.0f },
			.backgroundColor = Silica::GetTheme().Surface_Tertiary,
			.child = Silica::MakeWidget<Silica::SAlign>({
				.verticalAlign = Silica::VerticalAlign::Center,
				.child = Silica::MakeWidget<Silica::STextBlock>({
					.text = "Engine Default Assets",
					.color = Silica::GetTheme().Text_Dim
				})
			})
		});

		// -- Content Area --
		auto contentBox = Silica::MakeWidget<Silica::SVerticalBox>({ .spacing = 6.0f });

		if (!std::filesystem::exists(m_engineDefaultAssetsPath) || std::filesystem::is_empty(m_engineDefaultAssetsPath)) {
			contentBox->addSlot({ {0,0}, Silica::MakeWidget<Silica::SAlign>({
				.horizontalAlign = Silica::HorizontalAlign::Center,
				.child = Silica::MakeWidget<Silica::STextBlock>({
					.text = "No default assets found in Engine directory.",
					.color = Silica::GetTheme().Text_Dim
				})
			}) });
		}
		else {
			for (const auto& entry : std::filesystem::directory_iterator(m_engineDefaultAssetsPath)) {
				if (entry.is_directory()) continue;

				std::filesystem::path assetPath = entry.path();
				std::string fileName = assetPath.filename().string();
				std::string extension = assetPath.extension().string();

				// -- Build a Row for Each Asset --
				auto assetRow = Silica::MakeWidget<Silica::SBox>({
					.padding = { 8.0f, 6.0f },
					.backgroundColor = Silica::GetTheme().Surface_Secondary,
					.child = Silica::MakeWidget<Silica::SHorizontalBox>({
						.spacing = 10.0f,
						.slots = {
							// -- Asset Name --
							{ {1, 0}, Silica::MakeWidget<Silica::SAlign>({
								.verticalAlign = Silica::VerticalAlign::Center,
								.child = Silica::MakeWidget<Silica::STextBlock>({.text = fileName })
							})},
								// -- Import Button --
								{ {0, 0}, Silica::MakeWidget<Silica::SButton>({
									.padding = { 10.0f, 4.0f },
									.enabled = ProjectManager::hasProject(),
									.color = Silica::GetTheme().Accent_Primary,
									.hoverColor = Silica::GetTheme().Element_Hover,
									.onClick = [this, assetPath]() {
										importAssetToProject(assetPath);
										return Silica::EventReply::handled();
									},
									.child = Silica::MakeWidget<Silica::STextBlock>({.text = "Import" })
								})}
							}
						})
					});

				contentBox->addSlot({ {0,0}, assetRow });
			}
		}

		// -- Assemble Layout --
		auto scrollBox = Silica::MakeWidget<Silica::SScrollBox>({
			.child = Silica::MakeWidget<Silica::SBox>({
				.padding = { 10.0f, 10.0f },
				.child = contentBox
			})
		});

		m_uiRoot->setChild(Silica::MakeWidget<Silica::SBorderLayout>({
			.topBar = topBarBox,
			.contentArea = scrollBox
		}));
	}

	void AssetLibraryPanel::importAssetToProject(const std::filesystem::path& sourceAssetPath) {
		if (!ProjectManager::hasProject()) return;

		EditorActionQueue::push([sourceAssetPath]() {
			try {
				std::filesystem::path projectAssetsPath = ProjectManager::getProject()->getAssetsPath();

				// -- Create A "DefaultAssets" Folder --
				std::filesystem::path destFolder = projectAssetsPath / "DefaultAssets";
				if (!std::filesystem::exists(destFolder)) {
					std::filesystem::create_directories(destFolder);
				}

				// -- Copy The File --
				std::filesystem::path destFilePath = destFolder / sourceAssetPath.filename();

				std::filesystem::copy(sourceAssetPath, destFilePath, std::filesystem::copy_options::overwrite_existing);

				AX_CORE_LOG_INFO("Successfully imported {} to Project!", sourceAssetPath.filename().string());

				// Optional: If you have an event to refresh your Content Browser, call it here!
				// EventDispatcher::dispatch(ContentBrowserRefreshEvent());

			}
			catch (const std::exception& e) {
				AX_CORE_LOG_ERROR("Failed to import asset: {}", e.what());
			}
		});
	}

}
