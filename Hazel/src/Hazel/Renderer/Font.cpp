#include "hzpch.h"
#include "Font.h"

#include "msdf-atlas-gen.h"
#include "FontGeometry.h"
#include "GlyphGeometry.h"

namespace Hazel
{
	struct MSDFData
	{
		std::vector<msdf_atlas::GlyphGeometry> GlyphsGeometry;
		msdf_atlas::FontGeometry FontGeometry;
	};

	Font::Font(const std::filesystem::path& filepath)
		: _data(new MSDFData())
	{
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		HZ_CORE_ASSERT(ft, "freetype failed to initialize");

		const std::string filestring = filepath.string();
		// TODO , msdf::loadFontData loads from memory buffer which we'll need.
		msdfgen::FontHandle* font = msdfgen::loadFont(ft, filestring.c_str());
		if (!font)
		{
			HZ_CORE_LERROR("Failed to load font: {0}", filestring);
			return;
		}

		struct CharsetRange
		{
			uint32_t Begin, End;
		};

		// From imgui_draw.cpp [ImFontAtlas::GetGlyphRangesDefault]
		static constexpr CharsetRange charsetRanges[] =
		{
			{0x0020, 0x00FF} // Basic Latin + Latin Supplement
		};

		msdf_atlas::Charset charset;
		for (const auto charsetRange : charsetRanges)
		{
			for (uint32_t c = charsetRange.Begin; c <= charsetRange.End; c++)
			{
				charset.add(c);
			}
		}

		constexpr double fontScale = 1.0;
		_data->FontGeometry = msdf_atlas::FontGeometry(&_data->GlyphsGeometry);
		const int glyphsLoaded = _data->FontGeometry.loadCharset(font, fontScale, charset);
		HZ_CORE_LINFO("Loaded {1} glyphs out of {2} from font [{0}]", filepath.filename(), glyphsLoaded, charset.size());

#if 0
		msdfgen::Shape shape;
		const std::string griswold = "GRISWOLD";
		for (const auto& letter : griswold)
		{
			if (msdfgen::loadGlyph(shape, font, letter))
			{
				shape.normalize();
				//                      max. angle
				msdfgen::edgeColoringSimple(shape, 3.0);
				//           image width, height
				msdfgen::Bitmap<float, 3> msdf(32, 32);
				//                     range, scale, translation
				msdfgen::generateMSDF(msdf, shape, 4.0, 1.0, msdfgen::Vector2(4.0, 4.0));
				std::string filename = fmt::format("{0}.png", letter);
				msdfgen::savePng(msdf, filename.c_str());
			}
		}
#endif

		msdfgen::destroyFont(font);
		msdfgen::deinitializeFreetype(ft);
	}

	Font::~Font()
	{
		delete _data;
	}
}
