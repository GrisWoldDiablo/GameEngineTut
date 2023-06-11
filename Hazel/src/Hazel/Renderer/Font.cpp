#include "hzpch.h"
#include "Font.h"
#include "Hazel/Core/Timer.h"
#include "MSDFData.h"

#include "msdf-atlas-gen.h"
#include "FontGeometry.h"
#include "GlyphGeometry.h"

namespace Hazel
{
	template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
	static Ref<Texture2D> CreateAndCacheAtlas(
		const std::string& fontName, float fontSize,
		const std::vector<msdf_atlas::GlyphGeometry>& glyphsGeometry,
		const msdf_atlas::FontGeometry& fontGeometry, uint32_t width, uint32_t height)
	{
		msdf_atlas::GeneratorAttributes attributes;
		attributes.config.overlapSupport = true;
		attributes.scanlinePass = true;
		msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(width, height);
		generator.setAttributes(attributes);
		generator.setThreadCount(8); // TODO Check what is available get half of that.
		generator.generate(glyphsGeometry.data(), static_cast<int>(glyphsGeometry.size()));

		const auto bitmap = static_cast<msdfgen::BitmapConstRef<T, N>>(generator.atlasStorage());

		TextureSpecification specification;
		specification.Width = bitmap.width;
		specification.Height = bitmap.height;
		specification.Format = ImageFormat::RGB8;
		specification.GenerateMips = false;

		Ref<Texture2D> texture = Texture2D::Create(specification);
		texture->SetData(const_cast<void*>(reinterpret_cast<const void*>(bitmap.pixels)), bitmap.width * bitmap.height * 3);
		return texture;
	}

	Font::Font(const std::filesystem::path& filepath)
		: _data(new MSDFData())
	{
		Timer timer;
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

		static constexpr CharsetRange charsetRanges[] =
		{
			{0x0020, 0x00FF} // Basic Latin + Latin Supplement, From imgui_draw.cpp [ImFontAtlas::GetGlyphRangesDefault]
		};

		msdf_atlas::Charset charset;
		for (const auto& [begin, end] : charsetRanges)
		{
			for (uint32_t c = begin; c <= end; c++)
			{
				charset.add(c);
			}
		}

		constexpr double fontScale = 1.0;
		_data->FontGeometry = msdf_atlas::FontGeometry(&_data->GlyphsGeometry);

		const int glyphsLoaded = _data->FontGeometry.loadCharset(font, fontScale, charset);
		HZ_CORE_LINFO("Loaded {1} glyphs out of {2} from font [{0}]", filepath.filename(), glyphsLoaded, charset.size());

		double emSize = 40.0;
		msdf_atlas::TightAtlasPacker atlasPacker;
		atlasPacker.setPixelRange(2.0);
		atlasPacker.setMiterLimit(1.0);
		atlasPacker.setPadding(0.0);
		atlasPacker.setScale(emSize);
		const int remaining = atlasPacker.pack(_data->GlyphsGeometry.data(), _data->GlyphsGeometry.size());
		HZ_CORE_ASSERT(remaining == 0, "Failed to pack font.")

		int width;
		int height;
		atlasPacker.getDimensions(width, height);
		emSize = atlasPacker.getScale();

#define LCG_MULTIPLIER 6364136223846793005ull
#define LCG_INCREMENT 1442695040888963407ull
#define DEFAULT_ANGLE_THRESHOLD 3.0
#define THREAD_COUNT 8

		uint64_t coloringSeed = 0;
		constexpr bool isExpensiveColoring = false;
		if (isExpensiveColoring)
		{
			msdf_atlas::Workload([&glyphs = _data->GlyphsGeometry, &coloringSeed](int i, int threadNo) -> bool
			{
				const unsigned long long glyphSeed = (LCG_MULTIPLIER * (coloringSeed ^ i) + LCG_INCREMENT) * !!coloringSeed;
				glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
				return true;
			}, _data->GlyphsGeometry.size()).finish(THREAD_COUNT);
		}
		else
		{
			unsigned long long glyphSeed = coloringSeed;
			for (msdf_atlas::GlyphGeometry& glyph : _data->GlyphsGeometry)
			{
				glyphSeed *= LCG_MULTIPLIER;
				glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
			}
		}

		_atlasTexture = CreateAndCacheAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>("Test", static_cast<float>(emSize), _data->GlyphsGeometry, _data->FontGeometry, width, height);
		HZ_CORE_LINFO("Time to create Font Atlas {0}ms", timer.ElapsedMillis());

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
