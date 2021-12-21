module;

#include <array>

export module cheat.csgo.math.Color;

template <typename T>
static uint8_t _To_uint(T num)
{
	if constexpr (std::is_floating_point_v<T>)
	{
#ifdef _DEBUG
		if (num < 0 || num > 1)
			throw;
#endif
		return static_cast<uint8_t>(num * static_cast<T>(255));
	}
	else if constexpr (std::is_integral_v<T>)
		return static_cast<uint8_t>(num);
	else if constexpr (std::constructible_from<size_t, T>)
		return static_cast<uint8_t>(static_cast<size_t>(num));
	else
	{
		static_assert(std::_Always_false<T>, "Unknown type");
		throw;
	}
}

export namespace cheat::csgo
{
	class Color : std::array<uint8_t, 4>
	{
	public:
		using array::operator[];
		using array::data;
		using array::begin;
		using array::end;
		using array::_Unchecked_begin;
		using array::_Unchecked_end;

		Color( );

		template <class R, class G, class B, class A = uint8_t>
		Color(R r, G g, B b, A a = 255)
			: array{_To_uint(r), _To_uint(g), _To_uint(b), _To_uint(a)}
		{
		}

		const uint32_t* data_raw( ) const;
		uint32_t* data_raw( );

		bool operator==(const Color& other) const;
		bool operator!=(const Color& other) const;

		int r( ) const;
		int g( ) const;
		int b( ) const;
		int a( ) const;

		static Color FromHSB(float hue, float saturation, float brightness);
	};
}

