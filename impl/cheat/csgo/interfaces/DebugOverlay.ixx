module;

#include <cstdint>

export module cheat.csgo.interfaces.DebugOverlay;
export import cheat.math.qangle;
export import cheat.math.vector3;
export import cheat.math.matrix3x4;

export namespace cheat::csgo
{
	class OverlayText_t;

	class IVDebugOverlay
	{
	public:
		virtual      ~IVDebugOverlay( ) = default;
		virtual void AddEntityTextOverlay(int ent_index, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) = 0;
		virtual void AddBoxOverlay(const math::vector3& origin, const math::vector3& mins, const math::vector3& max, const math::qangle& orientation, int r, int g, int b, int a, float duration) = 0;
		virtual void AddSphereOverlay(const math::vector3& vOrigin, float flRadius, int nTheta, int nPhi, int r, int g, int b, int a, float flDuration) = 0;
		virtual void AddTriangleOverlay(const math::vector3& p1, const math::vector3& p2, const math::vector3& p3, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;
		virtual void AddLineOverlay(const math::vector3& origin, const math::vector3& dest, int r, int g, int b, bool noDepthTest, float duration) = 0;
		virtual void AddTextOverlay(const math::vector3& origin, float duration, const char* format, ...) = 0;
		virtual void AddTextOverlay(const math::vector3& origin, int line_offset, float duration, const char* format, ...) = 0;
		virtual void AddScreenTextOverlay(float flXPos, float flYPos, float flDuration, int r, int g, int b, int a, const char* text) = 0;
		virtual void AddSweptBoxOverlay(const math::vector3& start, const math::vector3& end, const math::vector3& mins, const math::vector3& max, const math::qangle& angles, int r, int g, int b, int a, float flDuration) = 0;
		virtual void           AddGridOverlay(const math::vector3& origin) = 0;
		virtual void           AddCoordFrameOverlay(const math::matrix3x4& frame, float flScale, int vColorTable[3][3] = 0) = 0;
		virtual int            ScreenPosition(const math::vector3& point, math::vector3& screen) = 0;
		virtual int            ScreenPosition(float flXPos, float flYPos, math::vector3& screen) = 0;
		virtual OverlayText_t* GetFirst( ) = 0;
		virtual OverlayText_t* GetNext(OverlayText_t* current) = 0;
		virtual void           ClearDeadOverlays( ) = 0;
		virtual void           ClearAllOverlays( ) = 0;
		virtual void           AddTextOverlayRGB(const math::vector3& origin, int line_offset, float duration, float r, float g, float b, float alpha, const char* format, ...) = 0;
		virtual void           AddTextOverlayRGB(const math::vector3& origin, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) = 0;
		virtual void           AddLineOverlayAlpha(const math::vector3& origin, const math::vector3& dest, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;
		virtual void           AddBoxOverlay2(const math::vector3& origin, const math::vector3& mins, const math::vector3& max,
											  const math::qangle& orientation, const uint8_t* faceColor, const uint8_t* edgeColor, float duration) = 0;
		virtual void PurgeTextOverlays( ) = 0;
		virtual void DrawPill(const math::vector3& mins, const math::vector3& max, float& diameter, int r, int g, int b, int a, float duration) = 0;
	};
}
