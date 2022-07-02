#pragma once

namespace Hazel
{
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return _uuid; }

	private:
		uint64_t _uuid;
	};
}

template<>
struct std::hash<Hazel::UUID>
{
	std::size_t operator()(const Hazel::UUID& uuid) const noexcept
	{
		return uuid;
	}
};
