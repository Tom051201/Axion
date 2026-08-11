#pragma once

#include <cstdint>
#include <vector>

#include "MathTypes.h"

namespace Silica {

	struct Glyph {
		Vec2 uvMin;
		Vec2 uvMax;
		Vec2 size;
		Vec2 offset;
		float advanceX;
	};



	class FontAtlas {
	public:

		bool loadFromFile(const char* filePath, float fontSize);

		const uint8_t* getPixels() const { return m_pixels.data(); }
		uint32_t getWidth() const { return m_width; }
		uint32_t getHeight() const { return m_height; }
		float getFontSize() const { return m_fontSize; }

		const Glyph& getGlyph(char c) const;

	private:

		std::vector<uint8_t> m_pixels;
		uint32_t m_width = 512;
		uint32_t m_height = 512;
		float m_fontSize = 0.0f;

		Glyph m_glyphs[96];

	};

}
