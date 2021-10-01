#pragma once
#include <type_traits>

namespace nstd
{
	template <typename T>
		requires(std::is_enum_v<T>)
	class decayed_enum
	{
	public:
		using raw_type = std::underlying_type_t<T>;
		using direct_type = T;

		decayed_enum(const raw_type& val)
			: raw(val)
		{
		}

		decayed_enum(const T& val)
			: direct(val)
		{
		}

		//---

		decayed_enum operator ~() const { return ~raw; }

		decayed_enum operator|(decayed_enum other) const { return raw | other.raw; }
		decayed_enum operator&(decayed_enum other) const { return raw & other.raw; }
		decayed_enum operator^(decayed_enum other) const { return raw ^ other.raw; }

		decayed_enum& operator|=(decayed_enum other)
		{
			raw |= other.raw;
			return *this;
		}

		decayed_enum& operator&=(decayed_enum other)
		{
			raw &= other.raw;
			return *this;
		}

		decayed_enum& operator^=(decayed_enum other)
		{
			raw ^= other.raw;
			return *this;
		}

		//---

		operator bool() const { return !!raw; }
		bool operator==(decayed_enum other) const { return raw == other.raw; }
		bool operator!=(decayed_enum other) const { return !(*this == other); }

		//---

		decayed_enum& add(decayed_enum other)
		{
			return *this |= other;
		}

		decayed_enum& remove(decayed_enum other)
		{
			return *this &= ~other;
		}

		bool has(decayed_enum other) const
		{
			return *this & other;
		}

		//---

		operator raw_type() const { return raw; }
		operator T() const { return direct; }

	private:
		union
		{
			raw_type raw;
			T direct;
		};
	};

	template <typename T>
		requires(std::is_enum_v<T>)
	auto& unwrap_enum(T& val)
	{
		return (decayed_enum<T>&)(val);
	}

	template <typename T>
		requires(std::is_enum_v<T>)
	auto unwrap_enum(const T& val)
	{
		return (decayed_enum<T>&)(val);
	}
}
