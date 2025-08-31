#pragma once

#include <string>
#include <filesystem>

namespace Axion {

	class Project {
	public:

		Project(const std::string& name);

		void setName(const std::string& name) { m_name = name; }
		void setProjectPath(const std::string& path) { m_projectPath = path; }
		void setAssetsPath(const std::string& path) { m_assetsPath = path; }
		void setScenesPath(const std::string& path) { m_scenesPath = path; }

		const std::string& getName() const { return m_name; }
		const std::string& getProjectPath() const { return m_projectPath; }
		const std::string& getAssetsPath() const { return m_assetsPath; }
		const std::string& getScenesPath() const { return m_scenesPath; }

		void load(const std::string& path);
		void save(const std::string& path);

		static Ref<Project> createNew(const std::string& location, const std::string& name);

	private:

		std::string m_name;
		std::string m_projectPath;
		std::string m_assetsPath;
		std::string m_scenesPath;

	};

}
