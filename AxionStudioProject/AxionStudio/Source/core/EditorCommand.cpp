#include "studiopch.h"
#include "EditorCommand.h"

#include "AxionEngine/Source/scene/SceneSerializer.h"

namespace Axion {

	// ----- Editor Command Manager -----
	void EditorCommandManager::push(const Shared<EditorCommand>& command) {
		if (s_currentIndex < s_history.size()) {
			s_history.erase(s_history.begin() + s_currentIndex, s_history.end());
		}

		s_history.push_back(command);
		s_currentIndex++;

		if (s_history.size() > s_maxHistorySize) {
			s_history.erase(s_history.begin());
			s_currentIndex--;
		}

		if (s_historyChangedCallback) s_historyChangedCallback();
	}

	void EditorCommandManager::clear() {
		s_history.clear();
		s_currentIndex = 0;

		if (s_historyChangedCallback) s_historyChangedCallback();
	}

	void EditorCommandManager::undo() {
		if (s_currentIndex > 0) {
			s_currentIndex--;
			s_history[s_currentIndex]->undo();

			if (s_historyChangedCallback) s_historyChangedCallback();
		}
	}

	void EditorCommandManager::redo() {
		if (s_currentIndex < s_history.size()) {
			s_history[s_currentIndex]->execute();
			s_currentIndex++;

			if (s_historyChangedCallback) s_historyChangedCallback();
		}
	}

	void EditorCommandManager::jumpTo(size_t targetIndex) {
		if (targetIndex > s_history.size()) return;

		auto cachedCallback = s_historyChangedCallback;
		s_historyChangedCallback = nullptr;

		// -- Move Backward --
		while (s_currentIndex > targetIndex) {
			s_currentIndex--;
			s_history[s_currentIndex]->undo();
		}

		// -- Move Forward --
		while (s_currentIndex < targetIndex) {
			s_history[s_currentIndex]->execute();
			s_currentIndex++;
		}

		s_historyChangedCallback = cachedCallback;
		if (s_historyChangedCallback) s_historyChangedCallback();
	}



	// ----- Transform Command -----
	TransformCommand::TransformCommand(Entity entity, const TransformComponent& startState, const TransformComponent& endState)
		: m_startState(startState), m_endState(endState) {

		m_scene = entity.getScene();
		m_entityUUID = entity.getComponent<UUIDComponent>().id;

		if (entity.hasComponent<TagComponent>()) {
			m_entityName = entity.getComponent<TagComponent>().tag;
		}
		else {
			m_entityName = "Entity";
		}
	}

	void TransformCommand::execute() {
		if (!m_scene) return;
		Entity entity = m_scene->getEntityByUUID(m_entityUUID);
		if (entity) {
			entity.getComponent<TransformComponent>() = m_endState;
		}
	}

	void TransformCommand::undo() {
		if (!m_scene) return;
		Entity entity = m_scene->getEntityByUUID(m_entityUUID);
		if (entity) {
			entity.getComponent<TransformComponent>() = m_startState;
		}
	}



	// ----- Create Entity Command -----
	CreateEntityCommand::CreateEntityCommand(Shared<Scene> scene, const std::string& name, Entity parent)
		: m_scene(scene), m_entityName(name) {
		if (parent) {
			m_parentUUID = parent.getComponent<UUIDComponent>().id;
		}
	}

	void CreateEntityCommand::execute() {
		if (!m_scene) return;

		if (m_isFirstExecution) {
			Entity e = m_scene->createEntity(m_entityName);
			m_entityUUID = e.getComponent<UUIDComponent>().id;

			if (m_parentUUID.isValid() && m_parentUUID != UUID(0, 0)) {
				Entity parent = m_scene->getEntityByUUID(m_parentUUID);
				if (parent) {
					e.setParent(parent);
				}
			}

			YAML::Emitter out;
			out << YAML::BeginSeq;
			SceneSerializer serializer(m_scene);
			serializer.serializeEntity(out, e);
			out << YAML::EndSeq;
			m_serializedData = std::string(out.c_str());

			m_isFirstExecution = false;
		}
		else {
			YAML::Node data = YAML::Load(m_serializedData);
			if (data.IsSequence()) {
				for (auto entityNode : data) {
					Entity deserializedEntity = SceneSerializer::deserializeEntityNode(m_scene.get(), entityNode, false);
					if (m_parentUUID.isValid() && m_parentUUID != UUID(0, 0)) {
						Entity parent = m_scene->getEntityByUUID(m_parentUUID);
						if (parent) {
							deserializedEntity.setParent(parent);
						}
					}
				}
			}
		}
	}

	void CreateEntityCommand::undo() {
		if (!m_scene) return;
		Entity e = m_scene->getEntityByUUID(m_entityUUID);
		if (e) {
			if (e.hasComponent<RelationshipComponent>()) {
				auto& rel = e.getComponent<RelationshipComponent>();
				if (rel.parent != entt::null) {
					Entity parent = { rel.parent, m_scene.get() };
					auto& parentRel = parent.getComponent<RelationshipComponent>();
					auto it = std::find(parentRel.children.begin(), parentRel.children.end(), (entt::entity)e);
					if (it != parentRel.children.end()) parentRel.children.erase(it);
				}
			}
			m_scene->destroyEntity(e);
		}
	}



	// ----- Delete Entity Command -----
	DeleteEntityCommand::DeleteEntityCommand(Shared<Scene> scene, Entity entity)
		: m_scene(scene) {
		
		m_rootUUID = entity.getComponent<UUIDComponent>().id;

		if (entity.hasComponent<TagComponent>()) {
			m_entityName = entity.getComponent<TagComponent>().tag;
		}
		else {
			m_entityName = "Entity";
		}

		if (entity.hasComponent<RelationshipComponent>()) {
			entt::entity parentHandle = entity.getComponent<RelationshipComponent>().parent;
			if (parentHandle != entt::null) {
				m_parentUUID = Entity{ parentHandle, m_scene.get() }.getComponent<UUIDComponent>().id;
			}
		}

		// -- Gather Entity And All Descendants --
		std::vector<Entity> entitiesToSerialize;
		gatherHierarchy(entity, entitiesToSerialize);

		// -- Serialize Everything To A YAML String Buffer --
		YAML::Emitter out;
		out << YAML::BeginSeq;
		SceneSerializer serializer(m_scene);
		for (Entity e : entitiesToSerialize) {
			serializer.serializeEntity(out, e);
		}
		out << YAML::EndSeq;

		m_serializedData = std::string(out.c_str());
	}

	void DeleteEntityCommand::execute() {
		Entity root = m_scene->getEntityByUUID(m_rootUUID);
		if (!root) return;

		// -- Remove From Parent --
		if (m_parentUUID.isValid()) {
			Entity parent = m_scene->getEntityByUUID(m_parentUUID);
			if (parent) {
				auto& parentRel = parent.getComponent<RelationshipComponent>();
				auto it = std::find(parentRel.children.begin(), parentRel.children.end(), (entt::entity)root);
				if (it != parentRel.children.end()) parentRel.children.erase(it);
			}
		}

		// -- Destroy Hierarchy --
		destroyHierarchy(root);
	}

	void DeleteEntityCommand::undo() {
		YAML::Node data = YAML::Load(m_serializedData);
		if (!data.IsSequence()) return;

		std::unordered_map<std::string, Entity> uuidToEntityMap;
		std::vector<std::pair<Entity, std::string>> relationshipsToBuild;

		// -- Recreate All Entities From YAML Buffer --
		for (auto entityNode : data) {
			Entity deserializedEntity = SceneSerializer::deserializeEntityNode(m_scene.get(), entityNode, false);

			std::string uuidStr = deserializedEntity.getComponent<UUIDComponent>().id.toString();
			uuidToEntityMap[uuidStr] = deserializedEntity;

			auto relationshipComponent = entityNode["RelationshipComponent"];
			if (relationshipComponent) {
				std::string parentUUID = relationshipComponent["Parent"].as<std::string>();
				if (parentUUID != "None") {
					relationshipsToBuild.push_back({ deserializedEntity, parentUUID });
				}
			}
		}

		// -- Reconstruct Internal Hierarchy --
		for (auto& pair : relationshipsToBuild) {
			Entity child = pair.first;
			std::string parentUUID = pair.second;
			if (uuidToEntityMap.find(parentUUID) != uuidToEntityMap.end()) {
				Entity parent = uuidToEntityMap[parentUUID];
				child.setParent(parent);
			}
		}

		// -- Re-Attach Root To Original External Parent --
		Entity root = m_scene->getEntityByUUID(m_rootUUID);
		if (root && m_parentUUID.isValid()) {
			Entity parent = m_scene->getEntityByUUID(m_parentUUID);
			if (parent) {
				root.setParent(parent);
			}
		}
	}

	void DeleteEntityCommand::gatherHierarchy(Entity e, std::vector<Entity>& outList) {
		outList.push_back(e);
		if (e.hasComponent<RelationshipComponent>()) {
			auto childrenCopy = e.getComponent<RelationshipComponent>().children;
			for (auto childHandle : childrenCopy) {
				gatherHierarchy(Entity{ childHandle, m_scene.get() }, outList);
			}
		}
	}

	void DeleteEntityCommand::destroyHierarchy(Entity e) {
		if (e.hasComponent<RelationshipComponent>()) {
			auto childrenCopy = e.getComponent<RelationshipComponent>().children;
			for (auto childHandle : childrenCopy) {
				destroyHierarchy(Entity{ childHandle, m_scene.get() });
			}
		}
		m_scene->destroyEntity(e);
	}



	// ----- Reparent Entity Command -----
	ReparentEntityCommand::ReparentEntityCommand(Shared<Scene> scene, Entity child, Entity newParent)
		: m_scene(scene) {

		m_childUUID = child.getComponent<UUIDComponent>().id;
		m_childName = child.hasComponent<TagComponent>() ? child.getComponent<TagComponent>().tag : "Entity";

		if (child.hasComponent<RelationshipComponent>()) {
			entt::entity parentHandle = child.getComponent<RelationshipComponent>().parent;
			if (parentHandle != entt::null) {
				m_oldParentUUID = Entity{ parentHandle, m_scene.get() }.getComponent<UUIDComponent>().id;
			}
		}

		if (newParent) {
			m_newParentUUID = newParent.getComponent<UUIDComponent>().id;
		}
	}

	void ReparentEntityCommand::execute() {
		performReparent(m_childUUID, m_newParentUUID);
	}

	void ReparentEntityCommand::undo() {
		performReparent(m_childUUID, m_oldParentUUID);
	}

	void ReparentEntityCommand::performReparent(UUID childId, UUID targetParentId) {
		if (!m_scene) return;
		Entity child = m_scene->getEntityByUUID(childId);
		if (!child) return;

		// -- Detach from current parent --
		if (child.hasComponent<RelationshipComponent>()) {
			auto& rel = child.getComponent<RelationshipComponent>();
			if (rel.parent != entt::null) {
				Entity oldParent = { rel.parent, m_scene.get() };
				auto& parentRel = oldParent.getComponent<RelationshipComponent>();
				auto it = std::find(parentRel.children.begin(), parentRel.children.end(), (entt::entity)child);
				if (it != parentRel.children.end()) parentRel.children.erase(it);
				rel.parent = entt::null;
			}
		}

		// -- Attach to new parent --
		if (targetParentId.isValid() && targetParentId != UUID(0, 0)) {
			Entity newParent = m_scene->getEntityByUUID(targetParentId);
			if (newParent) {
				child.setParent(newParent);
			}
		}
	}

}
