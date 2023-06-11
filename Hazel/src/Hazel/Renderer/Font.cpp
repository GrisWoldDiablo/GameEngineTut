#include "hzpch.h"
#include "Font.h"

#include "msdf-atlas-gen.h"

namespace Hazel
{
	Font::Font(const std::filesystem::path& filepath)
	{
#if FALSE
		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		if (ft)
		{
			const std::string filestring = filepath.string();
			msdfgen::FontHandle* font = msdfgen::loadFont(ft, filestring.c_str());
			if (font)
			{
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
				msdfgen::destroyFont(font);
			}
			msdfgen::deinitializeFreetype(ft);
		}
#endif
	}
}
