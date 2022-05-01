#if 0

#include <nstd/winapi/comptr_includes.h>

#include <Windows.h>

#include <imgui.h>
#include <d3d9.h>
#include <tchar.h>

#include <future>

import cheat.hooks;
import cheat.console;
import nstd.winapi.comptr;

using nstd::winapi::comptr;

// Data
static comptr<IDirect3D9> g_pD3D;
static comptr<IDirect3DDevice9> g_pd3dDevice;
static D3DPRESENT_PARAMETERS g_d3dpp;

IDirect3DDevice9* d3dDevice9_ptr;

// Win32 message handler
static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	switch(msg)
	{
		case WM_SIZE:
			if(g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
			{
				g_d3dpp.BackBufferWidth = LOWORD(lParam);
				g_d3dpp.BackBufferHeight = HIWORD(lParam);
				ResetDevice( );
			}
			return FALSE;
		case WM_SYSCOMMAND:
			if((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
				return FALSE;
			break;
		case WM_DESTROY:
			::PostQuitMessage(0);
			return FALSE;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

// Main code
int main(int, char**)
{
	// Create application window
	//ImGui_ImplWin32_EnableDpiAwareness();
	const WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, _T("ImGui Example"), nullptr};
	::RegisterClassEx(&wc);
	const HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Dear ImGui DirectX9 Example"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

	// Initialize Direct3D
	if(!CreateDeviceD3D(hwnd))
	{
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return TRUE;
	}

	d3dDevice9_ptr = g_pd3dDevice;

	using namespace cheat;
	console::enable( );
	hooks::init_basic( );

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	if(!hooks::start( ).get( ))
		goto _RESET;

	// Setup Dear ImGui context
	//IMGUI_CHECKVERSION( );
	//ImGui::CreateContext( );
	//ImGuiIO& io = ImGui::GetIO( );
	//(void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark( );
	//ImGui::StyleColorsClassic();

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	// Our state
	//bool   show_demo_window = true;
	//bool   show_another_window = false;

	// Main loop	
	for(;;)
	{
		// Poll and handle messages (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		MSG msg;
		while(::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if(msg.message == WM_QUIT)
				goto _RESET;
		}

		g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

		static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		static D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f),
			(int)(clear_color.y * clear_color.w * 255.0f),
			(int)(clear_color.z * clear_color.w * 255.0f),
			(int)(clear_color.w * 255.0f));
		g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
		const HRESULT result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);

		// Handle loss of D3D9 device
		if(result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel( ) == D3DERR_DEVICENOTRESET)
			ResetDevice( );
	}

_RESET:
	hooks::stop( );

	/*ImGui_ImplDX9_Shutdown( );
	ImGui_ImplWin32_Shutdown( );
	ImGui::DestroyContext( );*/

	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);

	return FALSE;
}

#endif

#include <nstd/core.h>
#include <nstd/winapi/comptr.h>

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

#include <DirectXMath.h>
#include <d3d9.h>
//#include <d3dx9.h>
#include <tchar.h>
#include <windows.h>

#include <chrono>

import cheat.hooks;
import cheat.console;

using nstd::winapi::comptr;

static comptr<IDirect3D9> g_pD3D;
static comptr<IDirect3DDevice9> g_pd3dDevice;
static D3DPRESENT_PARAMETERS g_d3dpp;

IDirect3DDevice9* d3dDevice9_ptr;

// This structure is created for each set of geometry that Rocket compiles. It stores the vertex and index buffers and
// the texture associated with the geometry, if one was specified.
struct RocketD3D9CompiledGeometry
{
	LPDIRECT3DVERTEXBUFFER9 vertices;
	DWORD num_vertices;

	LPDIRECT3DINDEXBUFFER9 indices;
	DWORD num_primitives;

	LPDIRECT3DTEXTURE9 texture;
};

// The internal format of the vertex we use for rendering Rocket geometry. We could optimise space by having a second
// untextured vertex for use when rendering coloured borders and backgrounds.
struct RocketD3D9Vertex
{
	FLOAT x, y, z;
	DWORD colour;
	FLOAT u, v;
};

constexpr DWORD vertex_fvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;

// Set to byte packing, or the compiler will expand our struct, which means it won't read correctly from file
#pragma pack(push, 1)
struct TGAHeader
{
	char  idLength;
	char  colourMapType;
	char  dataType;
	short int colourMapOrigin;
	short int colourMapLength;
	char  colourMapDepth;
	short int xOrigin;
	short int yOrigin;
	short int width;
	short int height;
	char  bitsPerPixel;
	char  imageDescriptor;
};
// Restore packing
#pragma pack (pop)

using namespace Rml;

struct RenderIfc : RenderInterface
{
	virtual void RenderGeometry(Vertex* vertices, int num_vertices, int* indices, int num_indices, TextureHandle texture, const Vector2f& translation)
	{
		// @TODO We've chosen to not support non-compiled geometry in the DirectX renderer. If you wanted to render non-compiled
		// geometry, for example for very small sections of geometry, you could use DrawIndexedPrimitiveUP or write to a
		// dynamic vertex buffer which is flushed when either the texture changes or compiled geometry is drawn.

		if(g_pd3dDevice == NULL)
		{
			return;
		}

		/// @TODO, HACK, just use the compiled geometry framework for now, this is inefficient but better than absolutely nothing
		/// for the time being
		CompiledGeometryHandle geom = this->CompileGeometry(vertices, num_vertices, indices, num_indices, texture);
		this->RenderCompiledGeometry(geom, translation);
		this->ReleaseCompiledGeometry(geom);
	}

	virtual CompiledGeometryHandle CompileGeometry(Vertex* vertices, int num_vertices, int* indices, int num_indices, TextureHandle texture)
	{
		// Construct a new RocketD3D9CompiledGeometry structure, which will be returned as the handle, and the buffers to
		// store the geometry.
		RocketD3D9CompiledGeometry* geometry = new RocketD3D9CompiledGeometry( );
		g_pd3dDevice->CreateVertexBuffer(num_vertices * sizeof(RocketD3D9Vertex), D3DUSAGE_WRITEONLY, vertex_fvf, D3DPOOL_DEFAULT, &geometry->vertices, NULL);
		g_pd3dDevice->CreateIndexBuffer(num_indices * sizeof(unsigned int), D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &geometry->indices, NULL);

		// Fill the vertex buffer.
		RocketD3D9Vertex* d3d9_vertices;
		geometry->vertices->Lock(0, 0, (void**)&d3d9_vertices, 0);
		for(int i = 0; i < num_vertices; ++i)
		{
			d3d9_vertices[i].x = vertices[i].position.x;
			d3d9_vertices[i].y = vertices[i].position.y;
			d3d9_vertices[i].z = 0;

			d3d9_vertices[i].colour = D3DCOLOR_RGBA(vertices[i].colour.red, vertices[i].colour.green, vertices[i].colour.blue, vertices[i].colour.alpha);

			d3d9_vertices[i].u = vertices[i].tex_coord[0];
			d3d9_vertices[i].v = vertices[i].tex_coord[1];
		}
		geometry->vertices->Unlock( );

		// Fill the index buffer.
		unsigned int* d3d9_indices;
		geometry->indices->Lock(0, 0, (void**)&d3d9_indices, 0);
		memcpy(d3d9_indices, indices, sizeof(unsigned int) * num_indices);
		geometry->indices->Unlock( );

		geometry->num_vertices = (DWORD)num_vertices;
		geometry->num_primitives = (DWORD)num_indices / 3;

		geometry->texture = texture == NULL ? NULL : (LPDIRECT3DTEXTURE9)texture;

		return (CompiledGeometryHandle)geometry;
	}

	virtual void RenderCompiledGeometry(CompiledGeometryHandle geometry, const Vector2f& translation)
	{
		// Build and set the transform matrix.
		//D3DMATRIX world_transform;
		//D3DXMatrixTranslation(&world_transform, translation.x, translation.y, 0);

		auto world_transform = DirectX::XMMatrixTranslation(translation.x, translation.y, 0);
		g_pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&world_transform);

		RocketD3D9CompiledGeometry* d3d9_geometry = (RocketD3D9CompiledGeometry*)geometry;

		// Set the vertex format for the Rocket vertices, and bind the vertex and index buffers.
		g_pd3dDevice->SetFVF(vertex_fvf);
		g_pd3dDevice->SetStreamSource(0, d3d9_geometry->vertices, 0, sizeof(RocketD3D9Vertex));
		g_pd3dDevice->SetIndices(d3d9_geometry->indices);

		// Set the texture, if this geometry has one.
		if(d3d9_geometry->texture != NULL)
			g_pd3dDevice->SetTexture(0, d3d9_geometry->texture);
		else
			g_pd3dDevice->SetTexture(0, NULL);

		// Draw the primitives.
		g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, d3d9_geometry->num_vertices, 0, d3d9_geometry->num_primitives);
	}

	virtual void ReleaseCompiledGeometry(CompiledGeometryHandle geometry)
	{
		RocketD3D9CompiledGeometry* d3d9_geometry = (RocketD3D9CompiledGeometry*)geometry;

		d3d9_geometry->vertices->Release( );
		d3d9_geometry->indices->Release( );

		delete d3d9_geometry;
	}

	virtual void EnableScissorRegion(bool enable)
	{
		g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, enable);
	}

	virtual void SetScissorRegion(int x, int y, int width, int height)
	{
		RECT scissor_rect;
		scissor_rect.left = x;
		scissor_rect.right = x + width;
		scissor_rect.top = y;
		scissor_rect.bottom = y + height;

		g_pd3dDevice->SetScissorRect(&scissor_rect);
	}

	virtual bool LoadTexture(TextureHandle& texture_handle, Vector2i& texture_dimensions, const String& source)
	{
		FileInterface* file_interface = GetFileInterface( );
		FileHandle file_handle = file_interface->Open(source);
		if(file_handle == NULL)
			return false;

		file_interface->Seek(file_handle, 0, SEEK_END);
		size_t buffer_size = file_interface->Tell(file_handle);
		file_interface->Seek(file_handle, 0, SEEK_SET);

		char* buffer = new char[buffer_size];
		file_interface->Read(buffer, buffer_size, file_handle);
		file_interface->Close(file_handle);

		TGAHeader header;
		memcpy(&header, buffer, sizeof(TGAHeader));

		int color_mode = header.bitsPerPixel / 8;
		int image_size = header.width * header.height * 4; // We always make 32bit textures 

		if(header.dataType != 2)
		{
			Log::Message(Log::LT_ERROR, "Only 24/32bit uncompressed TGAs are supported.");
			return false;
		}

		// Ensure we have at least 3 colors
		if(color_mode < 3)
		{
			Log::Message(Log::LT_ERROR, "Only 24 and 32bit textures are supported");
			return false;
		}

		const char* image_src = buffer + sizeof(TGAHeader);
		unsigned char* image_dest = new unsigned char[image_size];

		// Targa is BGR, swap to RGB and flip Y axis
		for(long y = 0; y < header.height; y++)
		{
			long read_index = y * header.width * color_mode;
			long write_index = ((header.imageDescriptor & 32) != 0) ? read_index : (header.height - y - 1) * header.width * color_mode;
			for(long x = 0; x < header.width; x++)
			{
				image_dest[write_index] = image_src[read_index + 2];
				image_dest[write_index + 1] = image_src[read_index + 1];
				image_dest[write_index + 2] = image_src[read_index];
				if(color_mode == 4)
					image_dest[write_index + 3] = image_src[read_index + 3];
				else
					image_dest[write_index + 3] = 255;

				write_index += 4;
				read_index += color_mode;
			}
		}

		texture_dimensions.x = header.width;
		texture_dimensions.y = header.height;

		bool success = GenerateTexture(texture_handle, image_dest, texture_dimensions);

		delete[ ] image_dest;
		delete[ ] buffer;

		return success;
	}

	virtual bool GenerateTexture(TextureHandle& texture_handle, const byte* source, const Vector2i& source_dimensions)
	{
		// Create a Direct3DTexture9, which will be set as the texture handle. Note that we only create one surface for
		// this texture; because we're rendering in a 2D context, mip-maps are not required.
		LPDIRECT3DTEXTURE9 d3d9_texture;
		if(g_pd3dDevice->CreateTexture(source_dimensions.x, source_dimensions.y, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &d3d9_texture, NULL) != D3D_OK)
			return false;

		// Lock the top surface and write the pixel data onto it.
		D3DLOCKED_RECT locked_rect;
		d3d9_texture->LockRect(0, &locked_rect, NULL, 0);
		for(int y = 0; y < source_dimensions.y; ++y)
		{
			for(int x = 0; x < source_dimensions.x; ++x)
			{
				const byte* source_pixel = source + (source_dimensions.x * 4 * y) + (x * 4);
				byte* destination_pixel = ((byte*)locked_rect.pBits) + locked_rect.Pitch * y + x * 4;

				destination_pixel[0] = source_pixel[2];
				destination_pixel[1] = source_pixel[1];
				destination_pixel[2] = source_pixel[0];
				destination_pixel[3] = source_pixel[3];
			}
		}
		d3d9_texture->UnlockRect(0);

		// Set the handle on the Rocket texture structure.
		texture_handle = (TextureHandle)d3d9_texture;
		return true;
	}

	virtual void ReleaseTexture(TextureHandle texture_handle)
	{
		((LPDIRECT3DTEXTURE9)texture_handle)->Release( );
	}

	void SetTransform(const Matrix4f* transform) override
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

};

class SystemIfc :public SystemInterface
{
	using clock = std::chrono::high_resolution_clock;

	clock::time_point start_time_ = clock::now( );
public:

	double GetElapsedTime( ) override
	{
		using namespace std::chrono;
		return duration_cast<duration<double>>(clock::now( ) - start_time_).count( );
	}
};

static bool CreateDeviceD3D(HWND hWnd) noexcept
{
	g_pD3D.Attach(Direct3DCreate9(D3D_SDK_VERSION));
	if(!g_pD3D)
		return false;

	// Create the D3DDevice
	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // Present with vsync
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	return SUCCEEDED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, g_pd3dDevice));
}

static void PresetD3D(HWND nativeWindow)
{
	RECT clientRect;
	if(!GetClientRect(nativeWindow, &clientRect))
	{
		// if we can't lookup the client rect, abort, something is seriously wrong
		std::terminate( );
	}

	// Set up an orthographic projection.
	auto projection = DirectX::XMMatrixOrthographicOffCenterLH(0, (FLOAT)clientRect.right, (FLOAT)clientRect.bottom, 0, -1, 1);

	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&projection);

	//-----

	// Switch to clockwise culling instead of counter-clockwise culling; Rocket generates counter-clockwise geometry,
	// so you can either reverse the culling mode when Rocket is rendering, or reverse the indices in the render
	// interface.
	g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	// Enable alpha-blending for Rocket.
	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// Set up the texture stage states for the diffuse texture.
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

	g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	// Disable lighting for Rocket.
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
}

static void ResetDevice( ) noexcept
{
	//ImGui_ImplDX9_InvalidateDeviceObjects( );
	const HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
	//IM_ASSERT(hr != D3DERR_INVALIDCALL);
	//ImGui_ImplDX9_CreateDeviceObjects( );
}

// Win32 message handler
static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
	switch(msg)
	{
		case WM_SIZE:
			if(g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
			{
				g_d3dpp.BackBufferWidth = LOWORD(lParam);
				g_d3dpp.BackBufferHeight = HIWORD(lParam);
				ResetDevice( );

				int width = LOWORD(lParam);;
				int height = HIWORD(lParam);;

				auto projection = DirectX::XMMatrixOrthographicOffCenterLH(0, (float)width, (float)height, 0, -1, 1);
				g_pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&projection);

				auto ctx = Rml::GetContext(0);
				if(ctx)
					ctx->SetDimensions(Vector2i(width, height));
				//Rml::GetContext(0)->ProcessProjectionChange( );
				//((Context*)m_rocket_context)->ProcessViewChange();


			}
			return FALSE;
		case WM_SYSCOMMAND:
			if((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
				return FALSE;
			break;
		case WM_DESTROY:
			::PostQuitMessage(0);
			return FALSE;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

Rml::String operator""_sample(const char* str, const size_t len)
{
	constexpr std::string_view path = NSTD_STRINGIZE_RAW(NSTD_CONCAT(RMLUI_DIR, \Samples\assets\));
	Rml::String buff;
	buff.reserve(path.size( ) + len);
	buff.append(path);
	buff.append(str, str + len);
	return buff;
}

struct ApplicationData
{
	bool show_text = true;
	Rml::String animal = "dog";
} my_data;

int main(int, char**)
{
	// Create application window
	//ImGui_ImplWin32_EnableDpiAwareness();
	const WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, _T("ImGui Example"), nullptr};
	::RegisterClassEx(&wc);
	const HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Dear ImGui DirectX9 Example"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

	// Initialize Direct3D
	if(!CreateDeviceD3D(hwnd))
	{
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return TRUE;
	}

	d3dDevice9_ptr = g_pd3dDevice;

	using namespace cheat;
	//console::enable( );
	//hooks::init_basic( );

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);
	PresetD3D(hwnd);

	//if(!hooks::start( ).get( ))
	//	return 0;

	//----------------

	RenderIfc render;
	SystemIfc system;

	SetRenderInterface(&render);
	SetSystemInterface(&system);

	// Now we can initialize RmlUi.
	Initialise( );

	int window_width = 1024;
	int window_height = 768;

	// Create a context to display documents within.
	Rml::Context* context = Rml::CreateContext("main", Rml::Vector2i(window_width, window_height));
	
	Rml::Debugger::Initialise(context);
	Rml::Debugger::SetVisible(true);
	
	// Tell RmlUi to load the given fonts.
	//Rml::LoadFontFace("LatoLatin-Regular.ttf"_sample);
	// Fonts can be registered as fallback fonts, as in this case to display emojis.
	//Rml::LoadFontFace("NotoEmoji-Regular.ttf"_sample, true);

	// Now we are ready to load our document.
	Rml::ElementDocument* document = context->LoadDocument("demo.rml"_sample);
	document->Show( );
	
#if 0
	// Replace and style some text in the loaded document.
	Rml::Element* element = document->GetElementById("world");
	element->SetInnerRML(reinterpret_cast<const char*>(u8"🌍"));
	element->SetProperty("font-size", "1.5em");

	// Set up data bindings to synchronize application data.
	if(Rml::DataModelConstructor constructor = context->CreateDataModel("animals"))
	{
		constructor.Bind("show_text", &my_data.show_text);
		constructor.Bind("animal", &my_data.animal);
}
#endif

	bool exit_application = false;
	while(!exit_application)
	{
		MSG msg;
		while(::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if(msg.message == WM_QUIT)
				return 0;
		}

		//g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		//g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		//g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

		// We assume here that we have some way of updating and retrieving inputs internally.
		//if(my_input->KeyPressed(KEY_ESC))
		//	exit_application = true;

		// Submit input events such as MouseMove and key events (not shown) to the context.
		//if(my_input->MouseMoved( ))
		//	context->ProcessMouseMove(mouse_pos.x, mouse_pos.y, 0);

		// Update the context to reflect any changes resulting from input events, animations,
		// modified and added elements, or changed data in data bindings.
		context->Update( );

		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,0), 1.0f, 0);
		const HRESULT result0 = g_pd3dDevice->BeginScene( );

		// Render the user interface. All geometry and other rendering commands are now
		// submitted through the render interface.
		context->Render( );

		const HRESULT result1 = g_pd3dDevice->EndScene( );
		const HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
		// Handle loss of D3D9 device
		if(result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel( ) == D3DERR_DEVICENOTRESET)
			ResetDevice( );
	}

	Rml::Shutdown( );

	return 0;

}