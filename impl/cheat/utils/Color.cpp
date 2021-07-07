#include "Color.hpp"

using namespace cheat::utl;

//Color Color::Black(0, 0, 0, 255);
//Color Color::White(255, 255, 255, 255);
//Color Color::Red(255, 0, 0, 255);
//Color Color::Green(0, 128, 0, 255);
//Color Color::Blue(0, 0, 255, 255);

Color::Color( )
{
	*((int*)this) = 0;
}

Color::Color(int r, int g, int b)
{
	SetColor(r, g, b, 255);
}

Color::Color(int r, int g, int b, int a)
{
	SetColor(r, g, b, a);
}

Color::Color(float r, float g, float b): Color(r, g, b, 1.0f)
{
}

Color::Color(float r, float g, float b, float a): Color(static_cast<int>(r * 255.0f),
														static_cast<int>(g * 255.0f),
														static_cast<int>(b * 255.0f),
														static_cast<int>(a * 255.0f))
{
}

Color::Color(float* rgb): Color(rgb[0], rgb[1], rgb[2], 1.0f)
{
}

Color::Color(unsigned long argb)
{
	color_stored[2] = static_cast<uint8_t>((argb & 0x000000FF) >> (0 * 8));
	color_stored[1] = static_cast<uint8_t>((argb & 0x0000FF00) >> (1 * 8));
	color_stored[0] = static_cast<uint8_t>((argb & 0x00FF0000) >> (2 * 8));
	color_stored[3] = static_cast<uint8_t>((argb & 0xFF000000) >> (3 * 8));
}

int Color::r( ) const { return color_stored[0]; }

int Color::g( ) const { return color_stored[1]; }

int Color::b( ) const { return color_stored[2]; }

int Color::a( ) const { return color_stored[3]; }

uint8_t& Color::operator[](int index)
{
	return color_stored[index];
}

const uint8_t& Color::operator[](int index) const
{
	return color_stored[index];
}

Color Color::FromHSB(float hue, float saturation, float brightness)
{
	const auto h = hue == 1.0f ? 0 : hue * 6.0f;
	const auto f = h - static_cast<int>(h);
	const auto p = brightness * (1.0f - saturation);
	const auto q = brightness * (1.0f - saturation * f);
	const auto t = brightness * (1.0f - (saturation * (1.0f - f)));

	if (h < 1)
	{
		return Color(static_cast<uint8_t>(brightness * 255),
					 static_cast<uint8_t>(t * 255),
					 static_cast<uint8_t>(p * 255));
	}
	if (h < 2)
	{
		return Color(static_cast<uint8_t>(q * 255),
					 static_cast<uint8_t>(brightness * 255),
					 static_cast<uint8_t>(p * 255));
	}
	if (h < 3)
	{
		return Color(static_cast<uint8_t>(p * 255),
					 static_cast<uint8_t>(brightness * 255),
					 static_cast<uint8_t>(t * 255));
	}
	if (h < 4)
	{
		return Color(static_cast<uint8_t>(p * 255),
					 static_cast<uint8_t>(q * 255),
					 static_cast<uint8_t>(brightness * 255));
	}
	if (h < 5)
	{
		return Color(static_cast<uint8_t>(t * 255),
					 static_cast<uint8_t>(p * 255),
					 static_cast<uint8_t>(brightness * 255));
	}
	return Color(static_cast<uint8_t>(brightness * 255),
				 static_cast<uint8_t>(p * 255),
				 static_cast<uint8_t>(q * 255));
}

void Color::SetRawColor(int color32)
{
	*((int*)this) = color32;
}

int Color::GetRawColor( ) const
{
	return *((int*)this);
}

void Color::SetColor(int r, int g, int b, int a)
{
	color_stored[0] = static_cast<uint8_t>(r);
	color_stored[1] = static_cast<uint8_t>(g);
	color_stored[2] = static_cast<uint8_t>(b);
	color_stored[3] = static_cast<uint8_t>(a);
}

void Color::SetColor(float r, float g, float b, float a)
{
	color_stored[0] = static_cast<uint8_t>(r * 255.0f);
	color_stored[1] = static_cast<uint8_t>(g * 255.0f);
	color_stored[2] = static_cast<uint8_t>(b * 255.0f);
	color_stored[3] = static_cast<uint8_t>(a * 255.0f);
}

void Color::GetColor(int& r, int& g, int& b, int& a) const
{
	r = color_stored[0];
	g = color_stored[1];
	b = color_stored[2];
	a = color_stored[3];
}

bool Color::operator==(const Color& rhs) const
{
	return (*((int*)this) == *((int*)&rhs));
}

bool Color::operator!=(const Color& rhs) const
{
	return !(operator==(rhs));
}

Color& Color::operator=(const Color& rhs)
{
	SetRawColor(rhs.GetRawColor( ));
	return *this;
}
