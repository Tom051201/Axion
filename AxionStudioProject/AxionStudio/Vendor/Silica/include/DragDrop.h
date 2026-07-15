#pragma once

#include <string>
#include <any>

#include "FontAtlas.h"

namespace Silica {

	struct DragDropPayload {
		std::string type;
		std::any data;
		std::string tooltip = "";
		FontAtlas* font = nullptr;
	};



	class DragDrop {
	public:

		static void beginDrag(const std::string& type, std::any data, const std::string& tooltip = "", FontAtlas* font = nullptr);
		static void endDrag();

		static bool isDragging();
		static bool isDraggingType(const std::string& type);

		static const DragDropPayload& getPayload();

	private:

		static DragDropPayload s_payload;
		static bool s_isDragging;

	};

}
