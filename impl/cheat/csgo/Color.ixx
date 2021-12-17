module;

#include <nstd/runtime_assert_fwd.h>

#include <array>

export module cheat.csgo.math:Color;

export namespace cheat::csgo
{
	class Color : std::array<uint8_t, 4>
	{
		template <typename T>
		static uint8_t To_uint_(T num)
		{
			if constexpr (std::is_floating_point_v<T>)
			{
				runtime_assert(num >= 0 && num <= 1);
				return static_cast<uint8_t>(num * static_cast<T>(255));
			}
			else if constexpr (std::is_integral_v<T>)
				return static_cast<uint8_t>(num);
			else if constexpr (std::constructible_from<size_t, T>)
				return static_cast<uint8_t>(static_cast<size_t>(num));
			else
			{
				static_assert(std::_Always_false<T>, "Unknown type");
				return {};
			}
		}

	public:
		using array::operator[];
		using array::data;
		using array::begin;
		using array::end;
		using array::_Unchecked_begin;
		using array::_Unchecked_end;

		template <class R, class G, class B, class A = uint8_t>
		Color(R r, G g, B b, A a = 255)
			: array{ To_uint_(r), To_uint_(g), To_uint_(b), To_uint_(a) }
		{
		}

		//Color Black(0, 0, 0, 255);
//Color White(255, 255, 255, 255);
//Color Red(255, 0, 0, 255);
//Color Green(0, 128, 0, 255);
//Color Blue(0, 0, 255, 255);

		Color()
		{
			*data_raw() = 0;
		}

		//Color(float* rgb): Color(rgb[0], rgb[1], rgb[2], 1.0f)
		//{
		//}
		//
		//Color(unsigned long argb): Color((argb & 0x000000FF) >> (0 * 8),
		//										(argb & 0x0000FF00) >> (1 * 8),
		//										(argb & 0x00FF0000) >> (2 * 8),
		//										(argb & 0xFF000000) >> (3 * 8))
		//{
		//}

		const uint32_t* data_raw() const
		{
			return reinterpret_cast<const uint32_t*>(this);
		}

		uint32_t* data_raw()
		{
			return reinterpret_cast<uint32_t*>(this);
		}

		bool operator==(const Color& other) const
		{
			return data_raw() == other.data_raw();
		}

		bool operator!=(const Color& other) const
		{
			return !(*this == other);
		}

		int r() const { return data()[0]; }
		int g() const { return data()[1]; }
		int b() const { return data()[2]; }
		int a() const { return data()[3]; }

		Color FromHSB(float hue, float saturation, float brightness)
		{
			const auto h = hue == 1.0f ? 0 : hue * 6.0f;
			const auto f = h - static_cast<int>(h);
			const auto p = brightness * (1.0f - saturation);
			const auto q = brightness * (1.0f - saturation * f);
			const auto t = brightness * (1.0f - (saturation * (1.0f - f)));

			if (h < 1)
				return Color(brightness, t, p);

			if (h < 2)
				return Color(q, brightness, p);

			if (h < 3)
				return Color(p, brightness, t);

			if (h < 4)
				return Color(q, p, brightness);

			if (h < 5)
				return Color(t, p, brightness);

			return Color(brightness, p, q);
		}
	};
}

