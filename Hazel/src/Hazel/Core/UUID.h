#pragma once

namespace Hazel
{
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		bool operator ==(const UUID& other) { return _uuid == other._uuid; }

		operator uint64_t() const { return _uuid; }

		static UUID Invalid() { return UUID(0); }

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
