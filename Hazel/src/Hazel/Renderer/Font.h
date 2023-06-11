#pragma once

namespace Hazel
{
	struct MSDFData;

	class Font
	{
	public:
		Font(const std::filesystem::path& filepath);
		~Font();

	private:
		MSDFData* _data;
	};
}
