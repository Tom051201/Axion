#pragma once

#include <vector>
#include <memory>

#include "SWidget.h"
#include "SDockSpace.h"
#include "SWindow.h"
#include "FontAtlas.h"

namespace Silica {

	class SWorkspace : public SWidget {
	public:

		struct Args {
			std::string initialTitle = "Workspace";
			WidgetPtr initialContent = nullptr;
			FontAtlas* font = nullptr;
		};

		void construct(const Args& args);

		void computeDesiredSize() override;
		void arrangeChildren(const Geometry& allocatedGeometry) override;
		void onDraw(DrawList& outDrawList, const Geometry& allocatedGeometry) const override;

		EventReply onMouseMove(const Geometry& allocatedGeometry, const Vec2& mousePos) override;
		EventReply onMouseButtonDown(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseButtonUp(const Geometry& allocatedGeometry, const Vec2& mousePos, MouseButton button) override;
		EventReply onMouseWheel(const Geometry& allocatedGeometry, const Vec2& mousePos, float scrollDelta) override;

		std::shared_ptr<SDockSpace> getDockSpace() const { return m_dockSpace; }
		void addFloatingWindow(std::shared_ptr<SWindow> window);

	private:

		std::shared_ptr<SDockSpace> m_dockSpace;
		std::vector<std::shared_ptr<SWindow>> m_floatingWindows;
		FontAtlas* m_font = nullptr;

	};

}
