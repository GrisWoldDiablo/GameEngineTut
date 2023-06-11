#pragma once

#include "FontGeometry.h"
#include "GlyphGeometry.h"

namespace Hazel
{
	struct MSDFData
	{
		std::vector<msdf_atlas::GlyphGeometry> GlyphsGeometry;
		msdf_atlas::FontGeometry FontGeometry;
	};
}
