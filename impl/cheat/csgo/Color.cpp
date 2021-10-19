#include "Color.hpp"

using namespace cheat::csgo;

//Color Color::Black(0, 0, 0, 255);
//Color Color::White(255, 255, 255, 255);
//Color Color::Red(255, 0, 0, 255);
//Color Color::Green(0, 128, 0, 255);
//Color Color::Blue(0, 0, 255, 255);

Color::Color()
{
	*data_raw( ) = 0;
}

//Color::Color(float* rgb): Color(rgb[0], rgb[1], rgb[2], 1.0f)
//{
//}
//
//Color::Color(unsigned long argb): Color((argb & 0x000000FF) >> (0 * 8),
//										(argb & 0x0000FF00) >> (1 * 8),
//										(argb & 0x00FF0000) >> (2 * 8),
//										(argb & 0xFF000000) >> (3 * 8))
//{
//}

const uint32_t* Color::data_raw() const
{
	return reinterpret_cast<const uint32_t*>(this);
}

uint32_t* Color::data_raw()
{
	return reinterpret_cast<uint32_t*>(this);
}

bool Color::operator==(const Color& other) const
{
	return data_raw( ) == other.data_raw( );
}

bool Color::operator!=(const Color& other) const
{
	return !(*this == other);
}

int Color::r() const { return data( )[0]; }

int Color::g() const { return data( )[1]; }

int Color::b() const { return data( )[2]; }

int Color::a() const { return data( )[3]; }

Color Color::FromHSB(float hue, float saturation, float brightness)
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
