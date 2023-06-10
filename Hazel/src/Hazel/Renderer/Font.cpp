#include "hzpch.h"
#include "Font.h"

#include "msdf-atlas-gen.h"

namespace Hazel
{
	Font::Font(const std::filesystem::path& filepath)
	{
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		if (ft)
		{
			const std::string filestring = filepath.string();
			msdfgen::FontHandle* font = msdfgen::loadFont(ft, filestring.c_str());
			if (font)
			{
				msdfgen::Shape shape;
				constexpr char character = 'A';
				for (int i = 0; i < 4; ++i)
				{
					if (msdfgen::loadGlyph(shape, font, character + i))
					{
						shape.normalize();
						//                      max. angle
						msdfgen::edgeColoringSimple(shape, 3.0);
						//           image width, height
						msdfgen::Bitmap<float, 3> msdf(32, 32);
						//                     range, scale, translation
						msdfgen::generateMSDF(msdf, shape, 4.0, 1.0, msdfgen::Vector2(4.0, 4.0));
						std::string filename = fmt::format("{0}.png", character + i);
						msdfgen::savePng(msdf, filename.c_str());
					}
				}
				msdfgen::destroyFont(font);
			}
			msdfgen::deinitializeFreetype(ft);
		}
	}
}
