#pragma once

struct ImVec2;
struct ImDrawList;
struct ImGuiWindow;

#ifdef _WIN32
struct IDirect3DDevice9;
#endif

namespace PostProcessing
{
#ifdef _WIN32
	void setDevice(IDirect3DDevice9* device) noexcept;
	void clearBlurTextures( ) noexcept;
	void onDeviceReset( ) noexcept;
#endif
	void newFrame( ) noexcept;
	void endFrame( ) noexcept;
	void performFullscreenBlur(ImDrawList* drawList, float alpha) noexcept;
	void performBlur(ImDrawList* drawList, const ImVec2& min, const ImVec2& max, float alpha) noexcept;
	void performFullscreenChromaticAberration(ImDrawList* drawList, float amount) noexcept;
	void performFullscreenMonochrome(ImDrawList* drawList, float amount) noexcept;


	void end_frame()noexcept;

	//move outside
	bool custom_textures_applicable(const ImGuiWindow *wnd);
}
