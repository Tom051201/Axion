#pragma once

#include <vector>

#include "AxionEngine/Source/core/Core.h"
#include "AxionEngine/Source/scene/Entity.h"
#include "AxionEngine/Source/scene/Components.h"

namespace Axion {

	class EditorCommand {
	public:

		virtual ~EditorCommand() = default;

		virtual void execute() = 0;
		virtual void undo() = 0;

		virtual std::string getName() const = 0;

	};

	class EditorCommandManager {
	public:

		static void push(const Shared<EditorCommand>& command);
		static void clear();

		static void undo();
		static void redo();

		static void setHistoryChangedCallback(const std::function<void()>& callback) { s_historyChangedCallback = callback; }
		static const std::vector<Shared<EditorCommand>>& getHistory() { return s_history; }
		static size_t getCurrentIndex() { return s_currentIndex; }
		static void jumpTo(size_t targetIndex);

	private:

		inline static std::vector<Shared<EditorCommand>> s_history;
		inline static size_t s_currentIndex = 0;
		inline static size_t s_maxHistorySize = 250;
		inline static std::function<void()> s_historyChangedCallback;

	};



	// ----- Transform Command -----
	class TransformCommand : public EditorCommand {
	public:

		TransformCommand(Entity entity, const TransformComponent& startState, const TransformComponent& endState);

		void execute() override;
		void undo() override;

		std::string getName() const override { return "Transform " + m_entityName; }

	private:

		Scene* m_scene;
		UUID m_entityUUID;
		std::string m_entityName;
		TransformComponent m_startState;
		TransformComponent m_endState;

	};



	// ----- Delete Entity Command -----
	class DeleteEntityCommand : public EditorCommand {
	public:

		DeleteEntityCommand(Shared<Scene> scene, Entity entity);

		void execute() override;
		void undo() override;

		std::string getName() const override { return "Delete Entity " + m_entityName; }

	private:

		Shared<Scene> m_scene;
		UUID m_rootUUID;
		UUID m_parentUUID = UUID(0, 0);
		std::string m_entityName;
		std::string m_serializedData;

		void gatherHierarchy(Entity e, std::vector<Entity>& outList);
		void destroyHierarchy(Entity e);

	};



	// ----- Paste Component Command -----
	template<typename T>
	class PasteComponentCommand : public EditorCommand {
	public:

		PasteComponentCommand(Entity entity, const std::string& compName, const T& newData)
			: m_compName(compName), m_newData(newData) {
			m_scene = entity.getScene();
			m_entityUUID = entity.getComponent<UUIDComponent>().id;

			if (entity.hasComponent<TagComponent>()) {
				m_entityName = entity.getComponent<TagComponent>().tag;
			}
			else {
				m_entityName = "Entity";
			}

			// -- Capture Old State If Entity Already Had This Component --
			if (entity.hasComponent<T>()) {
				m_oldData = entity.getComponent<T>();
			}
		}

		void execute() override {
			if (!m_scene) return;
			Entity entity = m_scene->getEntityByUUID(m_entityUUID);
			if (entity) {
				if (entity.hasComponent<T>()) {
					entity.getComponent<T>() = m_newData;
				}
				else {
					entity.addComponent<T>(m_newData);
				}
			}
		}

		void undo() override {
			if (!m_scene) return;
			Entity entity = m_scene->getEntityByUUID(m_entityUUID);
			if (entity) {
				if (m_oldData.has_value()) {
					// -- Restore Old Data --
					if (entity.hasComponent<T>()) {
						entity.getComponent<T>() = m_oldData.value();
					}
					else {
						entity.addComponent<T>(m_oldData.value());
					}
				}
				else {
					// -- Remove Pasted Component --
					if (entity.hasComponent<T>()) {
						entity.removeComponent<T>();
					}
				}
			}
		}

		std::string getName() const override { return "Paste " + m_compName + " to " + m_entityName; }

	private:

		Scene* m_scene;
		UUID m_entityUUID;
		std::string m_entityName;
		std::string m_compName;
		std::optional<T> m_oldData;
		T m_newData;

	};



	// ----- Remove Component Command -----
	template<typename T>
	class RemoveComponentCommand : public EditorCommand {
	public:

		RemoveComponentCommand(Entity entity, const std::string& compName)
			: m_compName(compName) {

			m_scene = entity.getScene();
			m_entityUUID = entity.getComponent<UUIDComponent>().id;

			if (entity.hasComponent<TagComponent>()) {
				m_entityName = entity.getComponent<TagComponent>().tag;
			}
			else {
				m_entityName = "Entity";
			}

			if (entity.hasComponent<T>()) {
				m_oldData = entity.getComponent<T>();
			}
		}

		void execute() override {
			if (!m_scene) return;
			Entity entity = m_scene->getEntityByUUID(m_entityUUID);
			if (entity && entity.hasComponent<T>()) {
				entity.removeComponent<T>();
			}
		}

		void undo() override {
			if (!m_scene) return;
			Entity entity = m_scene->getEntityByUUID(m_entityUUID);
			if (entity && m_oldData.has_value()) {
				entity.addComponent<T>(m_oldData.value());
			}
		}

		std::string getName() const override { return "Remove " + m_compName + " from " + m_entityName; }

	private:

		Scene* m_scene;
		UUID m_entityUUID;
		std::string m_entityName;
		std::string m_compName;
		std::optional<T> m_oldData;

	};



	// ----- Add Component Command -----
	template<typename T>
	class AddComponentCommand : public EditorCommand {
	public:
		AddComponentCommand(Entity entity, const std::string& compName)
			: m_compName(compName) {

			m_scene = entity.getScene();
			m_entityUUID = entity.getComponent<UUIDComponent>().id;

			if (entity.hasComponent<TagComponent>()) {
				m_entityName = entity.getComponent<TagComponent>().tag;
			}
			else {
				m_entityName = "Entity";
			}
		}

		void execute() override {
			if (!m_scene) return;
			Entity entity = m_scene->getEntityByUUID(m_entityUUID);
			if (entity && !entity.hasComponent<T>()) {
				entity.addComponent<T>();
			}
		}

		void undo() override {
			if (!m_scene) return;
			Entity entity = m_scene->getEntityByUUID(m_entityUUID);
			if (entity && entity.hasComponent<T>()) {
				entity.removeComponent<T>();
			}
		}

		std::string getName() const override { return "Add " + m_compName + " to " + m_entityName; }

	private:

		Scene* m_scene;
		UUID m_entityUUID;
		std::string m_entityName;
		std::string m_compName;

	};

}
