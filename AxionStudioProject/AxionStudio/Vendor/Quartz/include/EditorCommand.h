#pragma once

#include <string>

#include "TextBuffer.h"

namespace Quartz {

	class EditorCommand {
	public:

		virtual ~EditorCommand() = default;
		virtual void undo(TextBuffer& buffer, int& cursorLine, int& cursorCol, int& anchorLine, int& anchorCol) = 0;
		virtual void redo(TextBuffer& buffer, int& cursorLine, int& cursorCol, int& anchorLine, int& anchorCol) = 0;

	};

	class InsertTextCommand : public EditorCommand {
	public:

		InsertTextCommand(int line, int col, const std::string& text);

		void undo(TextBuffer& buffer, int& cursorLine, int& cursorCol, int& anchorLine, int& anchorCol) override;
		void redo(TextBuffer& buffer, int& cursorLine, int& cursorCol, int& anchorLine, int& anchorCol) override;

	private:

		int m_line, m_col;
		std::string m_text;

	};

	class DeleteTextCommand : public EditorCommand {
	public:

		DeleteTextCommand(int startLine, int startCol, int endLine, int endCol, const std::string& deletedText);

		void undo(TextBuffer& buffer, int& cursorLine, int& cursorCol, int& anchorLine, int& anchorCol) override;
		void redo(TextBuffer& buffer, int& cursorLine, int& cursorCol, int& anchorLine, int& anchorCol) override;

	private:

		int m_startLine, m_startCol, m_endLine, m_endCol;
		std::string m_deletedText;

	};

}
