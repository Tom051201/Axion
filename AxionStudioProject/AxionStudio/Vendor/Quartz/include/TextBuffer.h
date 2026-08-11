#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace Quartz {

	class TextBuffer {
	public:

		TextBuffer();

		bool loadFromFile(const std::filesystem::path& filepath);
		bool saveToFile(const std::filesystem::path& filepath);
		bool save();
		void clear();

		void insertString(int line, int col, const std::string& text);
		void deleteChars(int line, int col, int count);
		void insertNewLine(int line, int col);
		void mergeLineWithPrevious(int line);
		void deleteRange(int startLine, int startCol, int endLine, int endCol);

		std::string getTextRange(int startLine, int startCol, int endLine, int endCol) const;
		const std::vector<std::string>& getLines() const;
		std::string getLine(int index) const;
		size_t getLineCount() const;
		const std::filesystem::path& getCurrentFilePath() const;

	private:

		std::vector<std::string> m_lines;
		std::filesystem::path m_currentFilePath;

	};

}