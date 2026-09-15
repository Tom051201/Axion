#pragma once

#include <filesystem>

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/events/Event.h"
#include "AxionEngine/Source/scene/Entity.h"

#include "AxionStudio/Source/core/EditorState.h"

namespace Axion {

	constexpr int EventCategoryEditor = BIT(5);

	enum class EditorEventType {
		AssetRenamed = 100,
		AssetDeleted,
		EntitySelected,
		EditorStateChanged,
		SceneModified,
		EditorHistoryChanged,
		EditorSettingsChanged
	};

	#define EDITOR_EVENT_CLASS_TYPE(type) \
		static EventType getStaticType() { return static_cast<EventType>(EditorEventType::##type); }\
		virtual EventType getEventType() const override { return getStaticType(); }\
		virtual const char* getName() const override { return #type; }



	// ----- Asset Renamed Event -----
	class AssetRenamedEvent : public Event {
	public:

		AssetRenamedEvent(const std::filesystem::path& oldPath, const std::filesystem::path& newPath)
			: m_oldPath(oldPath), m_newPath(newPath) {}

		const std::filesystem::path& getOldPath() const { return m_oldPath; }
		const std::filesystem::path& getNewPath() const { return m_newPath; }

		std::string toString() const override {
			return "AssetRenamedEvent: " + m_oldPath.string() + " to " + m_newPath.string();
		}

		EDITOR_EVENT_CLASS_TYPE(AssetRenamed)
		EVENT_CLASS_CATEGORY(EventCategoryEditor)

	private:

		std::filesystem::path m_oldPath;
		std::filesystem::path m_newPath;

	};

	// ----- Asset Deleted Event -----
	class AssetDeletedEvent : public Event {
	public:

		AssetDeletedEvent(const std::filesystem::path& path)
			: m_path(path) {}

		const std::filesystem::path& getPath() const { return m_path; }

		std::string toString() const override {
			return "AssetDeletedEvent: " + m_path.string();
		}

		EDITOR_EVENT_CLASS_TYPE(AssetDeleted)
		EVENT_CLASS_CATEGORY(EventCategoryEditor)

	private:

		std::filesystem::path m_path;

	};

	// ----- Entity Selected Event -----
	class EntitySelectedEvent : public Event {
	public:

		EntitySelectedEvent(Entity entity)
			: m_entity(entity) {}

		Entity getEntity() const { return m_entity; }

		std::string toString() const override {
			return "EntitySelectedEvent";
		}

		EDITOR_EVENT_CLASS_TYPE(EntitySelected)
		EVENT_CLASS_CATEGORY(EventCategoryEditor)

	private:

		Entity m_entity;

	};

	// ----- Editor State Changed Event -----
	class EditorStateChangedEvent : public Event {
	public:

		EditorStateChangedEvent(EditorState state)
			: m_newState(state) {}

		EditorState getState() const { return m_newState; }

		std::string toString() const override {
			return "EditorStateChangedEvent";
		}

		EDITOR_EVENT_CLASS_TYPE(EditorStateChanged)
		EVENT_CLASS_CATEGORY(EventCategoryEditor)

	private:

		EditorState m_newState;

	};

	// ----- Scene Modified Event -----
	enum class SceneModificationType {
		NameChanged,
		SkyboxChanged,
		PhysicsSettingsChanged,
		GeneralSettingsChanged
	};

	class SceneModifiedEvent : public Event {
	public:

		SceneModifiedEvent(SceneModificationType modType)
			: m_modType(modType) {}

		SceneModificationType getModificationType() const { return m_modType; }

		std::string toString() const override {
			return "SceneModifiedEvent";
		}

		EDITOR_EVENT_CLASS_TYPE(SceneModified)
		EVENT_CLASS_CATEGORY(EventCategoryEditor)

	private:

		SceneModificationType m_modType;

	};

	// ----- Editor History Changed Event -----
	class EditorHistoryChangedEvent : public Event {
	public:

		EditorHistoryChangedEvent() = default;

		std::string toString() const override {
			return "EditorHistoryChangedEvent";
		}

		EDITOR_EVENT_CLASS_TYPE(EditorHistoryChanged)
		EVENT_CLASS_CATEGORY(EventCategoryEditor)

	};

	// ----- Editor Settings Changed Event -----
	enum class EditorSettingType : uint32_t {
		None = 0,
		AssetLoadBudget = BIT(0),
		DiscordRPC = BIT(1),
		AssetLibraryPaths = BIT(2),
		ContentBrowserLayout = BIT(3),
		MaterialEditorCamera = BIT(4),
		All = 0xFFFFFFFF
	};

	inline EditorSettingType operator|(EditorSettingType a, EditorSettingType b) {
		return static_cast<EditorSettingType>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}
	inline EditorSettingType operator&(EditorSettingType a, EditorSettingType b) {
		return static_cast<EditorSettingType>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
	}
	inline EditorSettingType& operator|=(EditorSettingType& a, EditorSettingType b) {
		a = a | b;
		return a;
	}

	class EditorSettingsChangedEvent : public Event {
	public:

		EditorSettingsChangedEvent(EditorSettingType changedSettings)
			: m_changedSettings(changedSettings) {}

		EditorSettingType getChangedSettings() const { return m_changedSettings; }

		bool hasChange(EditorSettingType type) const {
			return (m_changedSettings & type) != EditorSettingType::None;
		}

		std::string toString() const override {
			return "EditorSettingsChangedEvent";
		}

		EDITOR_EVENT_CLASS_TYPE(EditorSettingsChanged)
		EVENT_CLASS_CATEGORY(EventCategoryEditor)

	private:

		EditorSettingType m_changedSettings;

	};

}
