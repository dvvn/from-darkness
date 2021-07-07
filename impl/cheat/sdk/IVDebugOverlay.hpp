#pragma once

namespace cheat::csgo
{
	class OverlayText_t;

	class IVDebugOverlay
	{
	public:
		virtual      ~IVDebugOverlay( ) = 0;
		virtual auto AddEntityTextOverlay(int ent_index, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) -> void = 0;
		virtual auto AddBoxOverlay(const utl::Vector& origin, const utl::Vector& mins, const utl::Vector& max,
								   const utl::QAngle& orientation, int           r, int                   g, int b, int a, float duration) -> void = 0;
		virtual auto AddSphereOverlay(const utl::Vector& vOrigin, float flRadius, int nTheta, int nPhi, int r, int g, int b, int a, float flDuration) -> void = 0;
		virtual auto AddTriangleOverlay(const utl::Vector& p1, const utl::Vector& p2, const utl::Vector& p3, int r, int g, int b, int a, bool noDepthTest, float duration) -> void = 0;
		virtual auto AddLineOverlay(const utl::Vector& origin, const utl::Vector& dest, int r, int g, int b, bool noDepthTest, float duration) -> void = 0;
		virtual auto AddTextOverlay(const utl::Vector& origin, float duration, const char* format, ...) -> void = 0;
		virtual auto AddTextOverlay(const utl::Vector& origin, int line_offset, float duration, const char* format, ...) -> void = 0;
		virtual auto AddScreenTextOverlay(float flXPos, float flYPos, float flDuration, int r, int g, int b, int a, const char* text) -> void = 0;
		virtual auto AddSweptBoxOverlay(const utl::Vector& start, const utl::Vector& end,
										const utl::Vector& mins, const utl::Vector&  max, const utl::QAngle& angles, int r, int g, int b, int a, float flDuration) -> void = 0;
		virtual auto AddGridOverlay(const utl::Vector& origin) -> void = 0;
		virtual auto AddCoordFrameOverlay(const utl::matrix3x4_t& frame, float flScale, int vColorTable[3][3] = 0) -> void = 0;
		virtual auto ScreenPosition(const utl::Vector& point, utl::Vector& screen) -> int = 0;
		virtual auto ScreenPosition(float flXPos, float flYPos, utl::Vector& screen) -> int = 0;
		virtual auto GetFirst( ) -> OverlayText_t* = 0;
		virtual auto GetNext(OverlayText_t* current) -> OverlayText_t* = 0;
		virtual auto ClearDeadOverlays( ) -> void = 0;
		virtual auto ClearAllOverlays( ) -> void = 0;
		virtual auto AddTextOverlayRGB(const utl::Vector& origin, int line_offset, float duration, float r, float g, float b, float alpha, const char* format, ...) -> void = 0;
		virtual auto AddTextOverlayRGB(const utl::Vector& origin, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) -> void = 0;
		virtual auto AddLineOverlayAlpha(const utl::Vector& origin, const utl::Vector& dest, int r, int g, int b, int a, bool noDepthTest, float duration) -> void = 0;
		virtual auto AddBoxOverlay2(const utl::Vector& origin, const utl::Vector&  mins, const utl::Vector&  max,
									const utl::QAngle& orientation, const uint8_t* faceColor, const uint8_t* edgeColor, float duration) -> void = 0;
		virtual auto PurgeTextOverlays( ) -> void = 0;
		virtual auto DrawPill(const utl::Vector& mins, const utl::Vector& max, float& diameter, int r, int g, int b, int a, float duration) -> void = 0;
	};
}
