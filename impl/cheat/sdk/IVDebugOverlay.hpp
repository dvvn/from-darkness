#pragma once

namespace cheat::csgo
{
	class OverlayText_t;

	class IVDebugOverlay
	{
	public:
		virtual      ~IVDebugOverlay( ) = 0;
		virtual void AddEntityTextOverlay(int ent_index, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) = 0;
		virtual void AddBoxOverlay(const utl::Vector& origin, const utl::Vector& mins, const utl::Vector& max,
								   const utl::QAngle& orientation, int           r, int                   g, int b, int a, float duration) = 0;
		virtual void AddSphereOverlay(const utl::Vector& vOrigin, float flRadius, int nTheta, int nPhi, int r, int g, int b, int a, float flDuration) = 0;
		virtual void AddTriangleOverlay(const utl::Vector& p1, const utl::Vector& p2, const utl::Vector& p3, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;
		virtual void AddLineOverlay(const utl::Vector& origin, const utl::Vector& dest, int r, int g, int b, bool noDepthTest, float duration) = 0;
		virtual void AddTextOverlay(const utl::Vector& origin, float duration, const char* format, ...) = 0;
		virtual void AddTextOverlay(const utl::Vector& origin, int line_offset, float duration, const char* format, ...) = 0;
		virtual void AddScreenTextOverlay(float flXPos, float flYPos, float flDuration, int r, int g, int b, int a, const char* text) = 0;
		virtual void AddSweptBoxOverlay(const utl::Vector& start, const utl::Vector& end,
										const utl::Vector& mins, const utl::Vector&  max, const utl::QAngle& angles, int r, int g, int b, int a, float flDuration) = 0;
		virtual void           AddGridOverlay(const utl::Vector& origin) = 0;
		virtual void           AddCoordFrameOverlay(const utl::matrix3x4_t& frame, float flScale, int vColorTable[3][3] = 0) = 0;
		virtual int            ScreenPosition(const utl::Vector& point, utl::Vector& screen) = 0;
		virtual int            ScreenPosition(float flXPos, float flYPos, utl::Vector& screen) = 0;
		virtual OverlayText_t* GetFirst( ) = 0;
		virtual OverlayText_t* GetNext(OverlayText_t* current) = 0;
		virtual void           ClearDeadOverlays( ) = 0;
		virtual void           ClearAllOverlays( ) = 0;
		virtual void           AddTextOverlayRGB(const utl::Vector& origin, int line_offset, float duration, float r, float g, float b, float alpha, const char* format, ...) = 0;
		virtual void           AddTextOverlayRGB(const utl::Vector& origin, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) = 0;
		virtual void           AddLineOverlayAlpha(const utl::Vector& origin, const utl::Vector& dest, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;
		virtual void           AddBoxOverlay2(const utl::Vector& origin, const utl::Vector&  mins, const utl::Vector&  max,
											  const utl::QAngle& orientation, const uint8_t* faceColor, const uint8_t* edgeColor, float duration) = 0;
		virtual void PurgeTextOverlays( ) = 0;
		virtual void DrawPill(const utl::Vector& mins, const utl::Vector& max, float& diameter, int r, int g, int b, int a, float duration) = 0;
	};
}
