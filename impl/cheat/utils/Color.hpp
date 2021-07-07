#pragma once

namespace cheat::utl
{
	class Color
	{
	public:
		Color( );
		Color(int r, int g, int b);
		Color(int r, int g, int b, int a);
		Color(float r, float g, float b);

		Color(float r, float g, float b, float a);

		explicit Color(float* rgb);

		explicit Color(unsigned long argb);

		void SetRawColor(int color32);
		int  GetRawColor( ) const;
		void SetColor(int r, int g, int b, int a = 0);
		void SetColor(float r, float g, float b, float a = 0);
		void GetColor(int& r, int& g, int& b, int& a) const;

		//auto GetNormalnijHexColor( ) const -> string;

		int r( ) const;
		int g( ) const;
		int b( ) const;
		int a( ) const;

		uint8_t& operator[](int index);

		const uint8_t& operator[](int index) const;

		bool   operator==(const Color& rhs) const;
		bool   operator!=(const Color& rhs) const;
		Color& operator=(const Color& rhs);

		static Color FromHSB(float hue, float saturation, float brightness);

	private:
		uint8_t color_stored[4];
	};
}
