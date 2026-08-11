#pragma once

#include <string>
#include <memory>
#include <vector>

#include "SWidget.h"
#include "FontAtlas.h"
#include "Renderer.h"
#include "InputCodes.h"

#include "TextBuffer.h"
#include "LanguageProfile.h"
#include "EditorCommand.h"

namespace Quartz {

	class SQuartzEditor : public Silica::SWidget {
	public:

		struct Args {
			std::string initialText = "";
			Silica::FontAtlas* font = nullptr;
			LanguageProfile language = LanguageProfile::CPlusPlus();
			std::function<void()> onOpenFile;
			std::function<void()> onSaveFile;
			std::function<void()> onSaveFileAs;
			std::function<void()> onNewFile;
		};

		void construct(const Args& args);
		void computeDesiredSize() override;
		void onDraw(Silica::DrawList& outDrawList, const Silica::Geometry& allocatedGeometry) const override;

		Silica::EventReply onMouseMove(const Silica::Geometry& allocatedGeometry, const Silica::Vec2& mousePos) override;
		Silica::EventReply onMouseButtonDown(const Silica::Geometry& allocatedGeometry, const Silica::Vec2& mousePos, Silica::MouseButton button) override;
		Silica::EventReply onMouseButtonUp(const Silica::Geometry& allocatedGeometry, const Silica::Vec2& mousePos, Silica::MouseButton button) override;
		Silica::EventReply onMouseWheel(const Silica::Geometry& allocatedGeometry, const Silica::Vec2& mousePos, float scrollDelta) override;
		Silica::EventReply onKeyDown(Silica::Key key) override;
		Silica::EventReply onKeyUp(Silica::Key key) override;
		Silica::EventReply onChar(char c) override;

		void openFile(const std::filesystem::path& path);
		void saveFile();
		void saveFileAs(const std::filesystem::path& path);
		void newFile();

		void setLanguageProfile(const LanguageProfile& profile) { m_language = profile; }
		const LanguageProfile& getLanguageProfile() const { return m_language; }
		const std::filesystem::path& getCurrentFilePath() const { return m_buffer.getCurrentFilePath(); }

		void setOpenFileCallback(std::function<void()> cb) { m_onOpenFile = cb; }
		void setSaveFileCallback(std::function<void()> cb) { m_onSaveFile = cb; }
		void setSaveFileAsCallback(std::function<void()> cb) { m_onSaveFileAs = cb; }
		void setNewFileCallback(std::function<void()> cb) { m_onNewFile = cb; }

	private:

		TextBuffer m_buffer;
		LanguageProfile m_language;
		Silica::FontAtlas* m_font = nullptr;

		std::function<void()> m_onOpenFile;
		std::function<void()> m_onSaveFile;
		std::function<void()> m_onSaveFileAs;
		std::function<void()> m_onNewFile;

		float m_lineHeight = 18.0f;

		// -- Zoom & Input State --
		float m_zoom = 1.0f;
		bool m_isCtrlDown = false;

		// -- Scroll And View State --
		mutable float m_scrollY = 0.0f;
		mutable float m_scrollX = 0.0f;

		mutable float m_maxScrollY = 0.0f;
		mutable float m_maxScrollX = 0.0f;
		mutable float m_maxContentWidth = 0.0f;

		// -- Dragging State --
		bool m_isDraggingVThumb = false;
		bool m_isDraggingHThumb = false;
		float m_dragClickOffset = 0.0f;

		// -- Selection & Input State --
		bool m_isShiftDown = false;
		bool m_isDraggingText = false;

		int m_selectionAnchorLine = 0;
		int m_selectionAnchorColumn = 0;

		// -- Cursor State --
		int m_cursorLine = 0;
		int m_cursorColumn = 0;
		int m_idealCursorColumn = 0;

		const float GUTTER_WIDTH = 50.0f;
		const float PADDING_LEFT = 10.0f;
		const float PADDING_TOP = 10.0f;

		// -- Colors --
		Silica::Color m_commentColor = Silica::Color(106, 153, 85, 255);
		Silica::Color m_commentUnclosedColor = Silica::Color(206, 153, 85, 255);

		// -- Undo / Redo Stacks --
		void executeCommand(std::unique_ptr<EditorCommand> command);
		void undo();
		void redo();

		std::vector<std::unique_ptr<EditorCommand>> m_undoStack;
		std::vector<std::unique_ptr<EditorCommand>> m_redoStack;

		// -- Helpers --
		void scrollToCursor();
		Silica::Rect getVThumbRect(const Silica::Geometry& geo) const;
		Silica::Rect getHThumbRect(const Silica::Geometry& geo) const;

		bool hasSelection() const;
		void getSelectionRange(int& outStartLine, int& outStartCol, int& outEndLine, int& outEndCol) const;
		void deleteSelection();

	};

}
