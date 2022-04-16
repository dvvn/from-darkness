module;

// shaders are build during compilation and header files are created
#include "effects_data/Build/blur_x.h"
#include "effects_data/Build/blur_y.h"
#include "effects_data/Build/chromatic_aberration.h"
#include "effects_data/Build/monochrome.h"

#include <nstd/ranges.h>
#include <nstd/runtime_assert.h>
#include <nstd/unordered_map.h>
#include <nstd/winapi/comptr_includes.h>

#include <imgui_internal.h>

#include <d3d9.h>

#include <array>
#include <string>
#include <algorithm>
#include <functional>

module cheat.gui:effects;
import nstd.winapi.comptr;
import nstd.one_instance;

using namespace cheat;
using namespace gui;

using BYTE = unsigned char;

#ifdef _DEBUG
#define HRESULT_VALIDATE(fn, ...) runtime_assert(fn == D3D_OK,##__VA_ARGS__)
#else
#define HRESULT_VALIDATE(fn, ...) fn
#endif

static auto& _D3d( ) noexcept
{
	return *nstd::get_instance<IDirect3DDevice9*>( );
}

#if _WIN32

using nstd::winapi::comptr;

[[nodiscard]]
static auto _Create_texture(UINT width, UINT height) noexcept
{
	comptr<IDirect3DTexture9> texture;
	HRESULT_VALIDATE(_D3d( ).CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, texture, nullptr));
	return texture;
}

[[nodiscard]]
static auto _Create_texture(UINT scale = 1) noexcept
{
	comptr<IDirect3DSurface9> back_buffer;
	HRESULT_VALIDATE(_D3d( ).GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, back_buffer));
	D3DSURFACE_DESC desc;
	HRESULT_VALIDATE(back_buffer->GetDesc(&desc));
	return _Create_texture(desc.Width / scale, desc.Height / scale);

	/*IDirect3DTexture9* texture;
	TEST_RESULT = csgo_interfaces::get()->d3d_device->CreateTexture(desc.Width, desc.Height, 1, desc.Usage, desc.Format, desc.Pool, std::addressof(texture), nullptr);
	return texture;*/
}

#else
[[nodiscard]] static GLuint _Create_texture(int width, int height) noexcept
{
	GLint lastTexture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(GL_TEXTURE_2D, lastTexture);
	return texture;
}
#endif

template <class Shader>
struct basic_shader_program : comptr<Shader>
{
#ifndef _WIN32
	~basic_shader_program( )
	{

		if (program)
			glDeleteProgram(program);

	}
#endif

	basic_shader_program( ) = default;

	basic_shader_program(const BYTE* shader_source_function)
	{
		HRESULT_VALIDATE(_D3d( ).CreatePixelShader(reinterpret_cast<const DWORD*>(shader_source_function), *this));
	}

	basic_shader_program(const basic_shader_program& other) = delete;
	basic_shader_program& operator=(const basic_shader_program& other) = delete;
	basic_shader_program(basic_shader_program&& other) noexcept = default;
	basic_shader_program& operator=(basic_shader_program&& other) noexcept = default;

	void use(float uniform, int location) const noexcept
	{
#ifdef _WIN32

		HRESULT_VALIDATE(_D3d( ).SetPixelShader(*this));
		std::array params = {uniform, 0.f, 0.f, 0.f};
		HRESULT_VALIDATE(_D3d( ).SetPixelShaderConstantF(location, params.data( ), 1));
#else
		glUseProgram(program);
		glUniform1f(location, uniform);
#endif
	}

#if 0

#ifdef _WIN32
	void init(const BYTE* pixelShaderSrc) noexcept
#else
	void init(const char* pixelShaderSrc, const char* vertexShaderSrc) noexcept
#endif
	{
		if (initialized)
			return;
		initialized = true;

#ifdef _WIN32
		device->CreatePixelShader(reinterpret_cast<const DWORD*>(pixelShaderSrc), pixelShader.GetAddressOf( ));
#else
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &pixelShaderSrc, nullptr);
		glCompileShader(fragmentShader);

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
		glCompileShader(vertexShader);

		program = glCreateProgram( );
		glAttachShader(program, fragmentShader);
		glAttachShader(program, vertexShader);
		glLinkProgram(program);

		glDeleteShader(fragmentShader);
		glDeleteShader(vertexShader);
#endif
	}

#endif

private:
	//#ifdef _WIN32
	//	comptr<Shader> pixelShader;
	//#else
	//    GLuint program = 0;
	//#endif
	//	//bool initialized = false;
};

using shader_program = basic_shader_program<IDirect3DPixelShader9>;

struct custom_texture : comptr<IDirect3DTexture9>
{
	using comptr::comptr;
	using comptr::operator=;

	void prepare_buffer(const RECT* surface_pos = nullptr)
	{
		comptr<IDirect3DSurface9> back_buffer;
		HRESULT_VALIDATE(_D3d( ).GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, back_buffer));
		comptr<IDirect3DSurface9> surface;
		HRESULT_VALIDATE(Get( )->GetSurfaceLevel(0, surface));

		HRESULT_VALIDATE(_D3d( ).StretchRect(back_buffer, surface_pos, surface, nullptr, D3DTEXF_NONE));
	}

	void set_as_target( ) const
	{
		comptr<IDirect3DSurface9> surface;
		HRESULT_VALIDATE(Get( )->GetSurfaceLevel(0, surface));
		//SetRenderTarget resets viewport!!!
		HRESULT_VALIDATE(_D3d( ).SetRenderTarget(0, surface));
	}
};

class basic_effect
{
public:
	virtual ~basic_effect( ) = default;

protected:
	virtual void begin(const ImDrawList*, const ImDrawCmd*) = 0;
	virtual void process(ImDrawList* drawList) = 0;
	virtual void end(const ImDrawList*, const ImDrawCmd*) = 0;

public:
	//call once per frame!
	virtual void update(ImDrawList* drawList) noexcept
	{
		this->update_data( );
		drawList->AddCallback({this, 1}, nullptr);
		this->process(drawList);
		drawList->AddCallback({this, 3}, nullptr);
		//drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
	}

	//call multiple per frame (with different clip rects)
	virtual void render(ImDrawList* drawList) = 0;

	virtual void update_data( ) = 0;
	virtual void reset_data( ) = 0;
};

struct effect_data
{
	custom_texture texture;
	shader_program shader;
};

struct debug_string : std::string
{
	template <typename T>
	debug_string(const T& str)
	{
		static size_t counter = 0;

		auto tmp = std::to_string(counter++);
		append(str).append("##").append(tmp);
	}

	operator const char* () const
	{
		return c_str( );
	}
};

#if 0
static void _Set_projection(int width, int height, float p0 = 1.f, float p1 = 1.f, float p2 = 1.f, float p3 = 1.f)
{
	using std::array;
	using matrix_t = array<array<float, 4>, 4>;
	static_assert(sizeof(matrix_t) == sizeof(D3DMATRIX));
	matrix_t matrix;

	matrix[0] = {p0, 0.0f, 0.0f, 0.0f};
	matrix[1] = {0.0f, p1, 0.0f, 0.0f};
	matrix[2] = {0.0f, 0.0f, p2, 0.0f};
	matrix[3] = {-1.0f / width, 1.0f / height, 0.0f, p3};

	TEST_RESULT = csgo_interfaces::get( )->d3d_device->SetVertexShaderConstantF(0, reinterpret_cast<const float*>(matrix.data( )), matrix.size( ));
}
#endif

static auto _ImRect_to_rect(const ImRect& rect)
{
	RECT out;

	out.left = static_cast<LONG>(rect.Min.x);
	out.top = static_cast<LONG>(rect.Min.y);
	out.right = static_cast<LONG>(rect.Max.x);
	out.bottom = static_cast<LONG>(rect.Max.y);

	return out;
}

static auto _ImRect_to_viewport(const ImRect& rect)
{
	D3DVIEWPORT9 out;

	out.X = static_cast<DWORD>(rect.Min.x);
	out.Y = static_cast<DWORD>(rect.Min.y);
	out.Width = static_cast<DWORD>(rect.GetWidth( ));
	out.Height = static_cast<DWORD>(rect.GetHeight( ));
	out.MinZ = 0;
	out.MaxZ = 1;

	return out;
}

class effect : public basic_effect
{
public:
	effect(effect_data&& data)
		: data_(std::move(data))
	{
	}

private:
	effect_data data_;
};

class blur_effect : public basic_effect
{
public:
	blur_effect( )
	{
		set_default_values( );
	}

private:
	std::function<void( )> end_restore_;

protected:
	void post_begin( )
	{
		comptr<IDirect3DSurface9> target;
		HRESULT_VALIDATE(_D3d( ).GetRenderTarget(0, target));
		DWORD addessu;
		HRESULT_VALIDATE(_D3d( ).GetSamplerState(0, D3DSAMP_ADDRESSU, &addessu));
		HRESULT_VALIDATE(_D3d( ).SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
		DWORD addessv;
		HRESULT_VALIDATE(_D3d( ).GetSamplerState(0, D3DSAMP_ADDRESSV, &addessv));
		HRESULT_VALIDATE(_D3d( ).SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
		DWORD scissortest;
		HRESULT_VALIDATE(_D3d( ).GetRenderState(D3DRS_SCISSORTESTENABLE, &scissortest));
		HRESULT_VALIDATE(_D3d( ).SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE));
		comptr<IDirect3DPixelShader9> shader;
		HRESULT_VALIDATE(_D3d( ).GetPixelShader(shader));

		end_restore_ = [=]
		{
			HRESULT_VALIDATE(_D3d( ).SetSamplerState(0, D3DSAMP_ADDRESSU, addessu));
			HRESULT_VALIDATE(_D3d( ).SetSamplerState(0, D3DSAMP_ADDRESSV, addessv));
			HRESULT_VALIDATE(_D3d( ).SetRenderState(D3DRS_SCISSORTESTENABLE, scissortest));
			HRESULT_VALIDATE(_D3d( ).SetPixelShader(shader));
			HRESULT_VALIDATE(_D3d( ).SetRenderTarget(0, target));
		};
	}

	void begin(const ImDrawList* list, const ImDrawCmd* cmd) override
	{
		x.texture.prepare_buffer( );
		post_begin( );
	}

	void first_pass(const ImDrawList* drawList, const ImDrawCmd* cmd) const noexcept
	{
		const auto& rect = reinterpret_cast<const ImRect&>(drawList->_ClipRectStack.front( ));
		x.shader.use(1.0f / (rect.GetWidth( ) / down_sample_), 0);
		y.texture.set_as_target( );
	}

	void second_pass(const ImDrawList* drawList, const ImDrawCmd* cmd) const noexcept
	{
		const auto& rect = reinterpret_cast<const ImRect&>(drawList->_ClipRectStack.front( ));
		y.shader.use(1.0f / (rect.GetHeight( ) / down_sample_), 0);
		x.texture.set_as_target( );
	}

	void process(ImDrawList* drawList) override
	{
		const auto& rect = reinterpret_cast<const ImRect&>(drawList->_ClipRectStack.front( ));

		const auto min = ImVec2(0, 0);
		const auto max = ImVec2(rect.GetWidth( ), rect.GetHeight( ));

		auto clarity = clarity_;
		while (clarity-- != 0)
		{
			drawList->AddCallback({this, &blur_effect::first_pass}, nullptr);
			drawList->AddImage(x.texture, min, max);
			drawList->AddCallback({this, &blur_effect::second_pass}, nullptr);
			drawList->AddImage(y.texture, min, max);
		}
	}

	void end(const ImDrawList*, const ImDrawCmd*) override
	{
		end_restore_( );
	}

public:
	void render(ImDrawList* drawList) noexcept override
	{
		const auto& rect = reinterpret_cast<const ImRect&>(drawList->_ClipRectStack.front( ));

		const auto min = ImVec2(0, 0);
		const auto max = ImVec2(rect.GetWidth( ), rect.GetHeight( ));

		drawList->AddImage(x.texture, min, max, {0.f, 0.f}, {1.0f, 1.0f}, color_);
	}

	void update_data( ) override
	{
		if (!x.texture)
		{
			x.texture = _Create_texture( );
			y.texture = _Create_texture( );
		}
		if (!x.shader)
		{
			x.shader = {blur_x};
			y.shader = {blur_y};
		}
	}

	void reset_data( ) override
	{
		[[maybe_unused]] const auto dummy = std::make_tuple(std::move(x), std::move(y), std::move(end_restore_));
	}

private:
	ImU32 color_ = -1;
public:
	void set_color(const ImColor& color) { color_ = color; }
	void set_color(ImU32 color) { color_ = color; }

	void set_color(float alpha)
	{
		ImColor clr = color_;
		if (clr.Value.w == alpha)
			return;
		clr.Value.w = alpha;
		color_ = clr;
	}

	ImU32 get_color( ) const { return color_; }

private:
	uint8_t down_sample_ = 0;
public:
	void set_down_sample(uint8_t down_sample)
	{
		if (down_sample_ == down_sample)
			return;
		runtime_assert(down_sample != 0);
		down_sample_ = down_sample;
	}

	uint8_t get_down_sample( ) const { return down_sample_; }

private:
	uint8_t clarity_ = 0;
public:
	void set_clarity(uint8_t clarity) { clarity_ = clarity; }
	uint8_t get_clarity( ) const { return clarity_; }

private:
	void set_default_values( )
	{
		set_color(IM_COL32(255, 255, 255, 255));
		set_clarity(8);
		set_down_sample(4);
	}

protected:
	effect_data x;
	effect_data y;

private:
#ifdef _DEBUG

	struct
	{
		debug_string clarity = "clarity";
		debug_string down_sample = "down sample";
		debug_string reset_values = "reset values";
		debug_string reset_textures = "reset textures";
	} debug_strings_;

public:
	virtual void debug_update( )
	{
		//constexpr auto absolute_max = std::numeric_limits<uint8_t>::max( ) - 1;
		int clarity = clarity_;
		if (ImGui::SliderInt(debug_strings_.clarity.c_str( ), &clarity, 1, 128))
			set_clarity(static_cast<uint32_t>(clarity));

		int down_sample = down_sample_;
		if (ImGui::SliderInt(debug_strings_.down_sample, &down_sample, 1, 64))
			set_down_sample(static_cast<uint32_t>(down_sample));

		if (ImGui::Button(debug_strings_.reset_values))
			set_default_values( );

		if (ImGui::Button(debug_strings_.reset_textures))
			reset_data( );
	}

#endif
};

class blur_effect_scaled final : public blur_effect
{
public:
	using blur_effect::blur_effect;

private:
	ImRect rect_;
public:
	void set_rect(const ImRect& rect)
	{
		if (std::memcmp(&rect_, &rect, sizeof(ImRect)) == 0)
			return;
		x.texture.Reset( );
		rect_ = rect;
	}

	void update_data( ) override
	{
		if (!x.texture)
		{
			const auto w = static_cast<UINT>(rect_.GetWidth( ));
			const auto h = static_cast<UINT>(rect_.GetHeight( ));
			x.texture = _Create_texture(w, h);
			y.texture = _Create_texture(w, h);
		}
		blur_effect::update_data( );
	}

protected:
	void begin(const ImDrawList* list, const ImDrawCmd* cmd) override
	{
		const auto rect = static_cast<RECT*>(cmd->UserCallbackData);
		x.texture.prepare_buffer(rect);
		delete rect;

		post_begin( );
	}

public:
	void update(ImDrawList* drawList) noexcept override
	{
		auto& cmdbuffer = drawList->CmdBuffer;

		blur_effect::update(drawList);

		auto& begin_cmd = *std::ranges::find_if(cmdbuffer, [this](const ImDrawCmd& cmd)
		{
			return cmd.UserCallback == ImDrawCallback{static_cast<basic_effect*>(this), 1};
		});
		begin_cmd.UserCallbackData = new RECT(_ImRect_to_rect(rect_));
	}

	void render(ImDrawList* drawList) noexcept override
	{
		//do something with uv_min and uv_max to fix streching
		drawList->AddImage(x.texture, rect_.Min, rect_.Max, {0.f, 0.f}, {1.0f, 1.0f}, get_color( ));
	}
};

#if 0
class BlurEffect
{
public:
	static void draw(ImDrawList* drawList, float alpha) noexcept
	{
		instance( )._draw(drawList, alpha);
	}

	static void draws(ImDrawList* drawList, const ImVec2& min, const ImVec2& max, float alpha) noexcept
	{
		instance( )._draws(drawList, min, max, alpha);
	}
#ifdef _WIN32
	static void clearTextures( ) noexcept
	{
		if (instance( ).blurTexture1)
		{
			instance( ).blurTexture1->Release( );
			instance( ).blurTexture1 = nullptr;
		}
		if (instance( ).blurTexture2)
		{
			instance( ).blurTexture2->Release( );
			instance( ).blurTexture2 = nullptr;
		}
	}
#else
	static void clearTextures( ) noexcept
	{
		if (instance( ).blurTexture1) {
			glDeleteTextures(1, &instance( ).blurTexture1);
			instance( ).blurTexture1 = 0;
		}
		if (instance( ).blurTexture2) {
			glDeleteTextures(1, &instance( ).blurTexture2);
			instance( ).blurTexture2 = 0;
		}
	}
#endif
private:
#ifdef _WIN32
	IDirect3DSurface9* rtBackup = nullptr;
	IDirect3DTexture9* blurTexture1 = nullptr;
	IDirect3DTexture9* blurTexture2 = nullptr;
#else
	GLint textureBackup = 0;
	GLint fboBackup = 0;
	GLint programBackup = 0;

	GLuint blurTexture1 = 0;
	GLuint blurTexture2 = 0;
	GLuint frameBuffer = 0;
#endif

	ShaderProgram blurShaderX;
	ShaderProgram blurShaderY;
	static constexpr auto blurDownsample = 4;

	BlurEffect( ) = default;
	BlurEffect(const BlurEffect&) = delete;

	~BlurEffect( )
	{
#ifdef _WIN32
		if (rtBackup)
			rtBackup->Release( );
		if (blurTexture1)
			blurTexture1->Release( );
		if (blurTexture2)
			blurTexture2->Release( );
#endif
	}

	static BlurEffect& instance( ) noexcept
	{
		static BlurEffect blurEffect;
		return blurEffect;
	}

	static void begin(const ImDrawList*, const ImDrawCmd*) noexcept { instance( )._begin( ); }
	static void firstPass(const ImDrawList*, const ImDrawCmd*) noexcept { instance( )._firstPass( ); }
	static void secondPass(const ImDrawList*, const ImDrawCmd*) noexcept { instance( )._secondPass( ); }
	static void end(const ImDrawList*, const ImDrawCmd*) noexcept { instance( )._end( ); }

	void createTextures( ) noexcept
	{
		if (!blurTexture1)
			blurTexture1 = _Create_texture(_Back_buffer_width / blurDownsample, _Back_buffer_height / blurDownsample);
		if (!blurTexture2)
			blurTexture2 = _Create_texture(_Back_buffer_width / blurDownsample, _Back_buffer_height / blurDownsample);
	}

	void createShaders( ) noexcept
	{
#ifdef _WIN32
		blurShaderX.init(blur_x);
		blurShaderY.init(blur_y);
#else
		blurShaderX.init(
#include "Resources/Shaders/blur_x.glsl"
			,
#include "Resources/Shaders/passthrough.glsl"
		);

		blurShaderY.init(
#include "Resources/Shaders/blur_y.glsl"
			,
#include "Resources/Shaders/passthrough.glsl"
		);
#endif
	}

	void _begin( ) noexcept
	{
#ifdef _WIN32
		device->GetRenderTarget(0, &rtBackup);

		copyBackbufferToTexture(blurTexture1, D3DTEXF_LINEAR);

		device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

		const D3DMATRIX projection{
			{
				{
					1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f / (_Back_buffer_width / blurDownsample)
				  , 1.0f / (_Back_buffer_height / blurDownsample), 0.0f, 1.0f
				}
			}
		};
		device->SetVertexShaderConstantF(0, &projection.m[0][0], 4);
#else
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBackup);
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fboBackup);
		glGetIntegerv(GL_CURRENT_PROGRAM, &programBackup);

		if (!frameBuffer)
			glGenFramebuffers(1, &frameBuffer);

		glViewport(0, 0, _Back_buffer_width / blurDownsample, _Back_buffer_height / blurDownsample);
		glDisable(GL_SCISSOR_TEST);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTexture1, 0);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, blurTexture2, 0);
		glReadBuffer(GL_BACK);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, _Back_buffer_width, _Back_buffer_height, 0, 0, _Back_buffer_width / blurDownsample, _Back_buffer_height / blurDownsample, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
#endif
	}

	void _firstPass( ) noexcept
	{
		blurShaderX.use(1.0f / (_Back_buffer_width / blurDownsample), 0);
#ifdef _WIN32
		_Set_render_target(blurTexture2);
#else
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glUniform1i(1, 0);
#endif
	}

	void _secondPass( ) noexcept
	{
		blurShaderY.use(1.0f / (_Back_buffer_height / blurDownsample), 0);
#ifdef _WIN32
		_Set_render_target(blurTexture1);
#else
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glUniform1i(1, 0);
#endif
	}

	void _end( ) noexcept
	{
#ifdef _WIN32
		device->SetRenderTarget(0, rtBackup);
		rtBackup->Release( );

		device->SetPixelShader(nullptr);
		device->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
#else
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBackup);
		glUseProgram(programBackup);
		glBindTexture(GL_TEXTURE_2D, textureBackup);
#endif
	}

	void _draw(ImDrawList* drawList, float alpha) noexcept
	{
		createTextures( );
		createShaders( );

		if (!blurTexture1 || !blurTexture2)
			return;
		const auto min = ImVec2(0, 0);
		const auto max = ImVec2(_Back_buffer_width, _Back_buffer_height);

		drawList->AddCallback(&begin, nullptr);
		for (auto i = 0; i < 8; ++i)
		{
			drawList->AddCallback(&firstPass, nullptr);
			drawList->AddImage(blurTexture1, min, max);
			drawList->AddCallback(&secondPass, nullptr);
			drawList->AddImage(blurTexture2, min, max);
		}
		drawList->AddCallback(&end, nullptr);
		drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

#ifdef _WIN32

		drawList->AddImage(blurTexture1, min, max, {0.0f, 0.0f}, {1.0f, 1.0f}, IM_COL32(255, 255, 255, 255 * alpha));
#else
		drawList->AddImage(reinterpret_cast<ImTextureID>(blurTexture1), {0.0f, 0.0f}, {_Back_buffer_width * 1.0f, _Back_buffer_height * 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f}, IM_COL32(255, 255, 255, 255 * alpha));
#endif
	}

	void _draws(ImDrawList* drawList, const ImVec2& min, const ImVec2& max, float alpha) noexcept
	{
		createTextures( );
		createShaders( );

		if (!blurTexture1 || !blurTexture2)
			return;

		drawList->AddCallback(&begin, nullptr);
		for (int i = 1; i <= 8; ++i)
		{
			drawList->AddCallback(&firstPass, nullptr);
			drawList->AddImage(blurTexture1, min, max);
			drawList->AddCallback(&secondPass, nullptr);
			drawList->AddImage(blurTexture2, min, max);
		}
		drawList->AddCallback(&end, nullptr);
		drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

#ifdef _WIN32
		drawList->AddImage(blurTexture1, min, max, ImVec2(0, 0), ImVec2(_Back_buffer_width * 1, _Back_buffer_height * 1)
						   , IM_COL32(255, 255, 255, 255 * alpha));
#else
		drawList->AddImage(reinterpret_cast<ImTextureID>(blurTexture1), min, max, ImVec2(0, 0), ImVec2(_Back_buffer_width * 1, _Back_buffer_height * 1), IM_COL32(255, 255, 255, 255 * alpha));
#endif
	}
};
#endif

#if 0
class ChromaticAberration
{
public:
	static void draw(ImDrawList* drawList, float amount) noexcept
	{
		instance( ).amount = amount;
		instance( )._draw(drawList);
	}

	static void clearTexture( ) noexcept
	{
		instance( )._clearTexture( );
	}

private:
#ifdef _WIN32
	IDirect3DTexture9* texture = nullptr;
#else
	GLint textureBackup = 0;
	GLint programBackup = 0;

	GLuint texture = 0;
	GLuint frameBuffer = 0;
#endif

	ShaderProgram shader;
	float amount = 0.0f;

	ChromaticAberration( ) = default;
	ChromaticAberration(const ChromaticAberration&) = delete;

	~ChromaticAberration( )
	{
#ifdef _WIN32
		if (texture)
			texture->Release( );
#endif
	}

	static ChromaticAberration& instance( ) noexcept
	{
		static ChromaticAberration chromaticAberration;
		return chromaticAberration;
	}

	void _clearTexture( ) noexcept
	{
#ifdef _WIN32
		if (texture)
		{
			texture->Release( );
			texture = nullptr;
		}
#else
		if (texture) {
			glDeleteTextures(1, &texture);
			texture = 0;
		}
#endif
	}

	static void begin(const ImDrawList*, const ImDrawCmd* cmd) noexcept { instance( )._begin( ); }
	static void end(const ImDrawList*, const ImDrawCmd* cmd) noexcept { instance( )._end( ); }

	void _Create_texture( ) noexcept
	{
		if (!texture)
			texture = ::_Create_texture(_Back_buffer_width, _Back_buffer_height);
	}

	void createShaders( ) noexcept
	{
#ifdef _WIN32
		shader.init(chromatic_aberration);
#else
		shader.init(
#include "Resources/Shaders/chromatic_aberration.glsl"
			,
#include "Resources/Shaders/passthrough.glsl"
		);
#endif
	}

	void _begin( ) noexcept
	{
#ifdef _WIN32
		copyBackbufferToTexture(texture, D3DTEXF_NONE);

		device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

		const D3DMATRIX projection{
			{
				{
					1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f / (_Back_buffer_width), 1.0f / (_Back_buffer_height), 0.0f, 1.0f
				}
			}
		};
		device->SetVertexShaderConstantF(0, &projection.m[0][0], 4);
#else
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBackup);
		glGetIntegerv(GL_CURRENT_PROGRAM, &programBackup);

		if (!frameBuffer) {
			glGenFramebuffers(1, &frameBuffer);
		}

		glDisable(GL_SCISSOR_TEST);

		GLint fboBackup = 0;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fboBackup);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
		glReadBuffer(GL_BACK);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, _Back_buffer_width, _Back_buffer_height, 0, 0, _Back_buffer_width, _Back_buffer_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBackup);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
#endif
		shader.use(amount, 0);
#ifndef _WIN32
		glUniform1i(1, 0);
#endif
	}

	void _end( ) noexcept
	{
#ifdef _WIN32
		device->SetPixelShader(nullptr);
#else
		glUseProgram(programBackup);
		glBindTexture(GL_TEXTURE_2D, textureBackup);
		glEnable(GL_SCISSOR_TEST);
#endif
	}

	void _draw(ImDrawList* drawList) noexcept
	{
		_Create_texture( );
		createShaders( );
		if (!texture)
			return;

		drawList->AddCallback(&begin, nullptr);
		drawList->AddImage(texture, {-1.0f, -1.0f}, {1.0f, 1.0f});
		drawList->AddCallback(&end, nullptr);
		drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
	}
};

class MonochromeEffect
{
public:
	static void draw(ImDrawList* drawList, float amount) noexcept
	{
		instance( ).amount = amount;
		instance( )._draw(drawList);
	}

	static void clearTexture( ) noexcept
	{
		instance( )._clearTexture( );
	}

private:
#ifdef _WIN32
	IDirect3DTexture9* texture = nullptr;
#else
	GLint textureBackup = 0;
	GLint programBackup = 0;

	GLuint texture = 0;
	GLuint frameBuffer = 0;
#endif

	ShaderProgram shader;
	float amount = 0.0f;

	MonochromeEffect( ) = default;
	MonochromeEffect(const MonochromeEffect&) = delete;

	~MonochromeEffect( )
	{
#ifdef _WIN32
		if (texture)
			texture->Release( );
#endif
	}

	static MonochromeEffect& instance( ) noexcept
	{
		static MonochromeEffect monochromeEffect;
		return monochromeEffect;
	}

	void _clearTexture( ) noexcept
	{
#ifdef _WIN32
		if (texture)
		{
			texture->Release( );
			texture = nullptr;
		}
#else
		if (texture) {
			glDeleteTextures(1, &texture);
			texture = 0;
		}
#endif
	}

	static void begin(const ImDrawList*, const ImDrawCmd* cmd) noexcept { instance( )._begin( ); }
	static void end(const ImDrawList*, const ImDrawCmd* cmd) noexcept { instance( )._end( ); }

	void _Create_texture( ) noexcept
	{
		if (!texture)
			texture = ::_Create_texture(_Back_buffer_width, _Back_buffer_height);
	}

	void createShaders( ) noexcept
	{
#ifdef _WIN32
		shader.init(monochrome);
#else
		shader.init(
#include "Resources/Shaders/monochrome.glsl"
			,
#include "Resources/Shaders/passthrough.glsl"
		);
#endif
	}

	void _begin( ) noexcept
	{
#ifdef _WIN32
		copyBackbufferToTexture(texture, D3DTEXF_NONE);

		device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

		const D3DMATRIX projection{
			{
				{
					1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f / (_Back_buffer_width), 1.0f / (_Back_buffer_height), 0.0f, 1.0f
				}
			}
		};
		device->SetVertexShaderConstantF(0, &projection.m[0][0], 4);
#else
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBackup);
		glGetIntegerv(GL_CURRENT_PROGRAM, &programBackup);

		if (!frameBuffer) {
			glGenFramebuffers(1, &frameBuffer);
		}

		glDisable(GL_SCISSOR_TEST);

		GLint fboBackup = 0;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fboBackup);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
		glReadBuffer(GL_BACK);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(0, 0, _Back_buffer_width, _Back_buffer_height, 0, 0, _Back_buffer_width, _Back_buffer_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboBackup);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
#endif
		shader.use(amount, 0);
#ifndef _WIN32
		glUniform1i(1, 0);
#endif
	}

	void _end( ) noexcept
	{
#ifdef _WIN32
		device->SetPixelShader(nullptr);
#else
		glUseProgram(programBackup);
		glBindTexture(GL_TEXTURE_2D, textureBackup);
		glEnable(GL_SCISSOR_TEST);
#endif
	}

	void _draw(ImDrawList* drawList) noexcept
	{
		_Create_texture( );
		createShaders( );
		if (!texture)
			return;

		drawList->AddCallback(&begin, nullptr);
		drawList->AddImage(texture, {-1.0f, -1.0f}, {1.0f, 1.0f});
		drawList->AddCallback(&end, nullptr);
		drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
	}
};
#endif

template <class ValueType = std::pair<void*, blur_effect_scaled>>
class blur_scaled_data : public std::vector<ValueType>
{
	using std::vector<ValueType>::operator[];

public:
	using key_type = typename ValueType::first_type;
	using mapped_type = typename ValueType::second_type;

	auto operator[](const key_type& key_wanted)
	{
		auto found = find(key_wanted);
		if (found)
			return std::forward_as_tuple(*found, false);

		auto& tmp = temp_data_.emplace_back( );
		tmp.first = key_wanted;
		return std::forward_as_tuple(tmp.second, true);
	}

	void write_temp_data( )
	{
		if (temp_data_.empty( ))
			return;
		for (auto& d : temp_data_)
			this->push_back(std::move(d));
		temp_data_.clear( );
	}

	blur_effect_scaled* find(const key_type& key_wanted)
	{
		for (auto& [key, value] : *this)
		{
			if (key == key_wanted)
				return std::addressof(value);
		}
		return nullptr;
	}

	void reset_all_data( )
	{
		for (auto& [key, value] : *this)
			value.reset_data( );
	}

private:
	std::list<ValueType> temp_data_;
};

//#define USE_SCALED_BLUR

#ifdef USE_SCALED_BLUR
//gpu-friendly, but stretch background (fixable in theory) and work look because of small textures
static blur_scaled_data _Blur_scaled;
#else
//burn gpu, but looks perfect
static blur_effect _Blur;
#endif

static size_t _Blurred_targetes = 0;

void effects::perform_blur(ImDrawList* drawList, float alpha) noexcept
{
	++_Blurred_targetes;

#ifdef USE_SCALED_BLUR
	const auto& rect = reinterpret_cast<ImRect&>(drawList->_ClipRectStack.back( ));
	auto [blur, added] = _Blur_scaled[drawList];
	blur.set_rect(rect);
#else
	auto& blur = _Blur;
#endif

	blur.set_color(alpha);
	blur.update(drawList);
	blur.render(drawList);
}

void effects::new_frame( ) noexcept
{
	if (_Blurred_targetes == 0)
		return;
	_Blurred_targetes = 0;

#ifdef USE_SCALED_BLUR
	_Blur_scaled.write_temp_data( );
	for (const auto wnd : GImGui->Windows | std::views::reverse)
	{
		if (!is_applicable(wnd))
			continue;
		auto ptr = _Blur_scaled.find(wnd->DrawList);
		if (!ptr)
			continue;

		if (!ImGui::CollapsingHeader(wnd->Name))
			continue;
		ptr->debug_update( );
	}
#else
	_Blur.debug_update( );
#endif
}

void effects::invalidate_objects( ) noexcept
{
#ifdef USE_SCALED_BLUR
	_Blur_scaled.write_temp_data( );
	_Blur_scaled.reset_all_data( );
#else
	_Blur.reset_data( );
#endif
}

bool effects::is_applicable(const ImGuiWindow* wnd)
{
	if (!wnd->WasActive || wnd->SkipItems || wnd->Hidden)
		return false;

	if (wnd->Flags & ImGuiWindowFlags_NoBackground)
		return false;

	const auto bad_color = [style = std::addressof(ImGui::GetStyle( ))](ImGuiCol_ col)
	{
		const auto& clr = style->Colors[col];
		return /*clr.w == 0 ||*/ clr.w == 1;
	};

	if (wnd->Flags & ImGuiWindowFlags_MenuBar)
	{
		if (wnd->Size.y == wnd->MenuBarHeight( ))
			return false;
		if (bad_color(ImGuiCol_MenuBarBg) && bad_color(ImGuiCol_WindowBg))
			return false;
	}
	else if (wnd->Flags & (ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_ChildMenu))
	{
		if (bad_color(ImGuiCol_PopupBg))
			return false;
	}
	else
	{
		if (wnd->ParentWindow)
			return false;
		if (bad_color(ImGuiCol_WindowBg))
			return false;
	}

	return true;
}
