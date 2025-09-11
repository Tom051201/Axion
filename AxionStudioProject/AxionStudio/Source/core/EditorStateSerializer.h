#pragma once

#include "AxionStudio/Source/core/PanelManager.h"

#include <string>

namespace Axion {

	class EditorStateSerializer {
	public:

		EditorStateSerializer(const std::string& filepath);

		void save(const PanelManager& panelManager);
		void load(PanelManager& panelManager);

	private:

		std::string m_filepath;

	};

}
