#include "PostProcessing.h"

#include <algorithm>
#include <functional>

#ifdef _WIN32
typedef unsigned char BYTE;

// shaders are build during compilation and header files are created
#include "Build/blur_x.h"
#include "Build/blur_y.h"
#include "Build/chromatic_aberration.h"
#include "Build/monochrome.h"
#endif

#include <nstd/runtime_assert_fwd.h>

#include <imgui_internal.h>

#include <d3d9.h>

#include <vector>
#include <array>
#include <string>
#include <ranges>

static float _Back_buffer_width  = 0;
static float _Back_buffer_height = 0;

#ifdef _WIN32
static IDirect3DDevice9* _D3d_device; // DO NOT RELEASE!

struct result_tester
{
	__forceinline result_tester(const HRESULT result)
	{
		runtime_assert(result == D3D_OK);
	}
};

#define TEST_RESULT \
	[[maybe_unuser]] const result_tester _CONCAT(test,__LINE__)

[[nodiscard]] static IDirect3DTexture9* _Create_texture(int width = _Back_buffer_width, int height = _Back_buffer_height) noexcept
{
	IDirect3DTexture9* texture;
	TEST_RESULT = _D3d_device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, std::addressof(texture), nullptr);
	return texture;
}

template <typename T>
struct comptr_deleter
{
	void operator()(T* ptr) const
	{
		ptr->Release( );
	}
};

template <typename T, typename Base = std::unique_ptr<T, comptr_deleter<T>>>
struct comptr : Base
{
	using typename Base::pointer;
	using typename Base::deleter_type;

	comptr( )
		: Base( )
	{
	}

	comptr(pointer ptr)
		: Base(ptr)
	{
	}

	comptr(comptr&& other) noexcept
		: Base(std::move(other))
	{
	}

	comptr& operator=(comptr&& other) noexcept
	{
		*static_cast<Base*>(this) = static_cast<Base&&>(other);
		return *this;
	}

	class setter
	{
	public:
		setter(const setter& other)            = delete;
		setter& operator=(const setter& other) = delete;

		~setter( )
		{
			if (!ptr_to_)
				return;

			*source_ = ptr_to_;
		}

		setter(comptr* source)
			: source_(source)
		{
		}

		operator std::add_pointer_t<pointer>( )
		{
			return std::addressof(ptr_to_);
		}

	private:
		comptr* source_;
		pointer ptr_to_ = nullptr;
	};

	setter set( ) { return this; }
};

template <class T>
comptr(T*) -> comptr<std::remove_const_t<T>>;

static void _Copy_back_buffer_to_texture(IDirect3DTexture9* texture, D3DTEXTUREFILTERTYPE filtering) noexcept
{
	comptr<IDirect3DSurface9> back_buffer;
	TEST_RESULT = _D3d_device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, back_buffer.set( ));
	comptr<IDirect3DSurface9> surface;
	TEST_RESULT = texture->GetSurfaceLevel(0, surface.set( ));
	TEST_RESULT = _D3d_device->StretchRect(back_buffer.get( ), nullptr, surface.get( ), nullptr, filtering);
}

static void _Set_render_target(IDirect3DTexture9* rtTexture) noexcept
{
	comptr<IDirect3DSurface9> surface;
	TEST_RESULT = rtTexture->GetSurfaceLevel(0, surface.set( ));
	TEST_RESULT = _D3d_device->SetRenderTarget(0, surface.get( ));
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
	~basic_shader_program()
	{

		if (program)
			glDeleteProgram(program);

	}
#endif
	basic_shader_program( ) = default;

	basic_shader_program(const BYTE* shader_source_function)
	{
		TEST_RESULT = _D3d_device->CreatePixelShader(reinterpret_cast<const DWORD*>(shader_source_function), this->set( ));
	}

	basic_shader_program(const basic_shader_program& other)                = delete;
	basic_shader_program& operator=(const basic_shader_program& other)     = delete;
	basic_shader_program(basic_shader_program&& other) noexcept            = default;
	basic_shader_program& operator=(basic_shader_program&& other) noexcept = default;

	void use(float uniform, int location) const noexcept
	{
#ifdef _WIN32
		TEST_RESULT       = _D3d_device->SetPixelShader(this->get( ));
		std::array params = {uniform, 0.f, 0.f, 0.f};
		TEST_RESULT       = _D3d_device->SetPixelShaderConstantF(location, params._Unchecked_begin( ), 1);
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
		device->CreatePixelShader(reinterpret_cast<const DWORD*>(pixelShaderSrc), pixelShader.GetAddressOf());
#else
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &pixelShaderSrc, nullptr);
		glCompileShader(fragmentShader);

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
		glCompileShader(vertexShader);

		program = glCreateProgram();
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

	// ReSharper disable once CppMemberFunctionMayBeConst
	void store_back_buffer(D3DTEXTUREFILTERTYPE filtering)
	{
		_Copy_back_buffer_to_texture(this->get( ), filtering);
	}

	void set_as_target( ) const
	{
		_Set_render_target(this->get( ));
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
	virtual bool updated( ) const =0;

	/*class updated_value
	{
	public:
		enum value_type:uint8_t
		{
			NO
		  , YES
		  , UNKNOWN
		};

		updated_value(value_type value)
			: value_(value)
		{
		}

		updated_value(bool value)
			: value_(value ? YES : NO)
		{
		}

		updated_value( )
			: updated_value(UNKNOWN)
		{
		}

		explicit operator bool( ) const
		{
			return value_ == YES;
		}

		value_type get( ) const
		{
			return value_;
		}

		bool empty( ) const
		{
			return value_ == UNKNOWN;
		}

	private:
		value_type value_;
	};*/

	//call once per frame!
	virtual void update(ImDrawList* drawList) noexcept
	{
		this->update_textures( );
		this->update_shaders( );

		drawList->AddCallback({this, 1}, nullptr);
		this->process(drawList);
		drawList->AddCallback({this, 3}, nullptr);
		drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
	}

	/*bool update(ImDrawList* drawList, updated_value updated) noexcept
	{
	_AGAIN:
		switch (updated.get( ))
		{
			case updated_value::NO:
				break;
			case updated_value::YES:
				return false;
			case updated_value::UNKNOWN:
				updated = this->updated( );
				goto _AGAIN;
			default:
				runtime_assert("Unknown state detected");
				return false;
		}

		return this->update(drawList);
	}*/

	//call multiple per frame (with different clip rects)
	virtual void render(ImDrawList* drawList)
	{
		update(drawList);
	}

	virtual void update_textures( ) = 0;
	virtual void update_shaders( ) = 0;
	virtual void reset_textures( ) = 0;
	virtual void reset_shaders( ) = 0;
};

struct effect_data
{
	custom_texture texture;
	shader_program shader;
};

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

	TEST_RESULT = _D3d_device->SetVertexShaderConstantF(0, reinterpret_cast<const float*>(matrix._Unchecked_begin( )), matrix.size( ));
}

#ifdef _DEBUG
static std::string _Make_debug_string(const std::string_view& str, const void* _this)
{
	const auto num = std::to_string(reinterpret_cast<uintptr_t>(_this));
	std::string out;
	out.reserve(str.size( ) + 2 + num.size( ));
	out += str;
	out += "##";
	out += num;
	return out;
}
#endif

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

class blur_effect final : public basic_effect
{
public:
	blur_effect( )
	{
		set_default_values( );
	}

	bool updated( ) const override
	{
		if (!x.shader)
			return false;
		if (!x.texture)
			return false;
		if (!y.shader)
			return false;
		if (!y.texture)
			return false;

		return true;
	}

protected:
	void begin(const ImDrawList*, const ImDrawCmd*) override
	{
		TEST_RESULT = _D3d_device->GetRenderTarget(0, std::addressof(backup_));
		x.texture.store_back_buffer(D3DTEXF_LINEAR);

		TEST_RESULT = _D3d_device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		TEST_RESULT = _D3d_device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		TEST_RESULT = _D3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

		_Set_projection(_Back_buffer_width /*/ down_sample_*/, _Back_buffer_height /*/ down_sample_*/);
	}

private:
	void first_pass(const ImDrawList*, const ImDrawCmd*) const noexcept
	{
		x.shader.use(1.0f / (_Back_buffer_width / down_sample_), 0);
		y.texture.set_as_target( );
	}

	void second_pass(const ImDrawList*, const ImDrawCmd*) const noexcept
	{
		y.shader.use(1.0f / (_Back_buffer_height / down_sample_), 0);
		x.texture.set_as_target( );
	}

protected:
	void process(ImDrawList* drawList) override
	{
		const auto min = ImVec2(0, 0);
		const auto max = ImVec2(_Back_buffer_width, _Back_buffer_height);

		auto clarity = clarity_;
		while (clarity-- != 0)
		{
			drawList->AddCallback({this, &blur_effect::first_pass}, nullptr);
			drawList->AddImage(x.texture.get( ), min, max);
			drawList->AddCallback({this, &blur_effect::second_pass}, nullptr);
			drawList->AddImage(y.texture.get( ), min, max);
		}
	}

	void end(const ImDrawList*, const ImDrawCmd*) override
	{
		TEST_RESULT = _D3d_device->SetRenderTarget(0, backup_);
		backup_->Release( );
		TEST_RESULT = _D3d_device->SetPixelShader(nullptr);
		TEST_RESULT = _D3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
	}

public:
	void update(ImDrawList* drawList) noexcept override
	{
#ifdef _DEBUG
		this->finish_debug_update( );
#endif
		basic_effect::update(drawList);
	}

	void render(ImDrawList* drawList) noexcept override
	{
		drawList->AddImage(x.texture.get( ), {0, 0}, {_Back_buffer_width, _Back_buffer_height}, {0.0f, 0.0f}, {1.0f, 1.0f}, color_);
	}

	void update_textures( ) override
	{
		if (!x.texture)
			x.texture = _Create_texture( );
		if (!y.texture)
			y.texture = _Create_texture( );
	}

	void update_shaders( ) override
	{
		if (!x.shader)
			x.shader = {blur_x};
		if (!y.shader)
			y.shader = {blur_y};
	}

	void reset_textures( ) override
	{
		x.texture.reset( );
		y.texture.reset( );
	}

	void reset_shaders( ) override
	{
		x.shader.reset( );
		y.shader.reset( );
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
		color_      = clr;
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
		reset_textures( );
	}

	uint8_t get_down_sample( ) const { return down_sample_; }

private:
	uint8_t clarity_ = 0;
public:
	void set_clarity(uint8_t clarity)
	{
		clarity_ = clarity;
	}

	uint8_t get_clarity( ) const { return clarity_; }

private:
	void set_default_values( )
	{
		set_color(IM_COL32(255, 255, 255, 255));
		set_clarity(8);
		set_down_sample(4);
	}

	effect_data x;
	effect_data y;
	IDirect3DSurface9* backup_ = nullptr;

#ifdef _DEBUG

	struct
	{
		std::string clarity        = _Make_debug_string("clarity", this);
		std::string down_sample    = _Make_debug_string("down sample", this);
		std::string reset_values   = _Make_debug_string("reset values", this);
		std::string reset_textures = _Make_debug_string("reset textures", this);
	} debug_strings_;

	std::function<void( )> debug_update_;

	void finish_debug_update( )
	{
		if (!debug_update_)
			return;

		debug_update_( );
		debug_update_ = nullptr;
	}

public:
	void debug_update( )
	{
		//constexpr auto absolute_max = std::numeric_limits<uint8_t>::max( ) - 1;
		int clarity = clarity_;
		if (ImGui::SliderInt(debug_strings_.clarity.c_str( ), &clarity, 1, 128))
			debug_update_ = std::bind_front(&blur_effect::set_clarity, this, clarity);;

		int down_sample = down_sample_;
		if (ImGui::SliderInt(debug_strings_.down_sample.c_str( ), &down_sample, 1, 64))
			debug_update_ = std::bind_front(&blur_effect::set_down_sample, this, down_sample);

		if (ImGui::Button(debug_strings_.reset_values.c_str( )))
			debug_update_ = std::bind_front(&blur_effect::set_default_values, this);

		if (ImGui::Button(debug_strings_.reset_textures.c_str( )))
			debug_update_ = std::bind_front(&blur_effect::reset_textures, this);
	}

#endif
};

#if 0
class BlurEffect
{
public:
	static void draw(ImDrawList* drawList, float alpha) noexcept
	{
		instance()._draw(drawList, alpha);
	}

	static void draws(ImDrawList* drawList, const ImVec2& min, const ImVec2& max, float alpha) noexcept
	{
		instance()._draws(drawList, min, max, alpha);
	}
#ifdef _WIN32
	static void clearTextures() noexcept
	{
		if (instance().blurTexture1)
		{
			instance().blurTexture1->Release();
			instance().blurTexture1 = nullptr;
		}
		if (instance().blurTexture2)
		{
			instance().blurTexture2->Release();
			instance().blurTexture2 = nullptr;
		}
	}
#else
	static void clearTextures() noexcept
	{
		if (instance().blurTexture1) {
			glDeleteTextures(1, &instance().blurTexture1);
			instance().blurTexture1 = 0;
		}
		if (instance().blurTexture2) {
			glDeleteTextures(1, &instance().blurTexture2);
			instance().blurTexture2 = 0;
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

	BlurEffect() = default;
	BlurEffect(const BlurEffect&) = delete;

	~BlurEffect()
	{
#ifdef _WIN32
		if (rtBackup)
			rtBackup->Release();
		if (blurTexture1)
			blurTexture1->Release();
		if (blurTexture2)
			blurTexture2->Release();
#endif
	}

	static BlurEffect& instance() noexcept
	{
		static BlurEffect blurEffect;
		return blurEffect;
	}

	static void begin(const ImDrawList*, const ImDrawCmd*) noexcept { instance()._begin(); }
	static void firstPass(const ImDrawList*, const ImDrawCmd*) noexcept { instance()._firstPass(); }
	static void secondPass(const ImDrawList*, const ImDrawCmd*) noexcept { instance()._secondPass(); }
	static void end(const ImDrawList*, const ImDrawCmd*) noexcept { instance()._end(); }

	void createTextures() noexcept
	{
		if (!blurTexture1)
			blurTexture1 = _Create_texture(_Back_buffer_width / blurDownsample, _Back_buffer_height / blurDownsample);
		if (!blurTexture2)
			blurTexture2 = _Create_texture(_Back_buffer_width / blurDownsample, _Back_buffer_height / blurDownsample);
	}

	void createShaders() noexcept
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

	void _begin() noexcept
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

	void _firstPass() noexcept
	{
		blurShaderX.use(1.0f / (_Back_buffer_width / blurDownsample), 0);
#ifdef _WIN32
		_Set_render_target(blurTexture2);
#else
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glUniform1i(1, 0);
#endif
	}

	void _secondPass() noexcept
	{
		blurShaderY.use(1.0f / (_Back_buffer_height / blurDownsample), 0);
#ifdef _WIN32
		_Set_render_target(blurTexture1);
#else
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glUniform1i(1, 0);
#endif
	}

	void _end() noexcept
	{
#ifdef _WIN32
		device->SetRenderTarget(0, rtBackup);
		rtBackup->Release();

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
		createTextures();
		createShaders();

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

		drawList->AddImage(blurTexture1, min, max, { 0.0f, 0.0f }, { 1.0f, 1.0f }, IM_COL32(255, 255, 255, 255 * alpha));
#else
		drawList->AddImage(reinterpret_cast<ImTextureID>(blurTexture1), { 0.0f, 0.0f }, { _Back_buffer_width * 1.0f, _Back_buffer_height * 1.0f }, { 0.0f, 1.0f }, { 1.0f, 0.0f }, IM_COL32(255, 255, 255, 255 * alpha));
#endif
	}

	void _draws(ImDrawList* drawList, const ImVec2& min, const ImVec2& max, float alpha) noexcept
	{
		createTextures();
		createShaders();

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
		instance().amount = amount;
		instance()._draw(drawList);
	}

	static void clearTexture() noexcept
	{
		instance()._clearTexture();
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

	ChromaticAberration() = default;
	ChromaticAberration(const ChromaticAberration&) = delete;

	~ChromaticAberration()
	{
#ifdef _WIN32
		if (texture)
			texture->Release();
#endif
	}

	static ChromaticAberration& instance() noexcept
	{
		static ChromaticAberration chromaticAberration;
		return chromaticAberration;
	}

	void _clearTexture() noexcept
	{
#ifdef _WIN32
		if (texture)
		{
			texture->Release();
			texture = nullptr;
		}
#else
		if (texture) {
			glDeleteTextures(1, &texture);
			texture = 0;
		}
#endif
	}

	static void begin(const ImDrawList*, const ImDrawCmd* cmd) noexcept { instance()._begin(); }
	static void end(const ImDrawList*, const ImDrawCmd* cmd) noexcept { instance()._end(); }

	void _Create_texture() noexcept
	{
		if (!texture)
			texture = ::_Create_texture(_Back_buffer_width, _Back_buffer_height);
	}

	void createShaders() noexcept
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

	void _begin() noexcept
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

	void _end() noexcept
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
		_Create_texture();
		createShaders();
		if (!texture)
			return;

		drawList->AddCallback(&begin, nullptr);
		drawList->AddImage(texture, { -1.0f, -1.0f }, { 1.0f, 1.0f });
		drawList->AddCallback(&end, nullptr);
		drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
	}
};

class MonochromeEffect
{
public:
	static void draw(ImDrawList* drawList, float amount) noexcept
	{
		instance().amount = amount;
		instance()._draw(drawList);
	}

	static void clearTexture() noexcept
	{
		instance()._clearTexture();
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

	MonochromeEffect() = default;
	MonochromeEffect(const MonochromeEffect&) = delete;

	~MonochromeEffect()
	{
#ifdef _WIN32
		if (texture)
			texture->Release();
#endif
	}

	static MonochromeEffect& instance() noexcept
	{
		static MonochromeEffect monochromeEffect;
		return monochromeEffect;
	}

	void _clearTexture() noexcept
	{
#ifdef _WIN32
		if (texture)
		{
			texture->Release();
			texture = nullptr;
		}
#else
		if (texture) {
			glDeleteTextures(1, &texture);
			texture = 0;
		}
#endif
	}

	static void begin(const ImDrawList*, const ImDrawCmd* cmd) noexcept { instance()._begin(); }
	static void end(const ImDrawList*, const ImDrawCmd* cmd) noexcept { instance()._end(); }

	void _Create_texture() noexcept
	{
		if (!texture)
			texture = ::_Create_texture(_Back_buffer_width, _Back_buffer_height);
	}

	void createShaders() noexcept
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

	void _begin() noexcept
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

	void _end() noexcept
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
		_Create_texture();
		createShaders();
		if (!texture)
			return;

		drawList->AddCallback(&begin, nullptr);
		drawList->AddImage(texture, { -1.0f, -1.0f }, { 1.0f, 1.0f });
		drawList->AddCallback(&end, nullptr);
		drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
	}
};
#endif

template <std::derived_from<basic_effect> T, class Storage = std::vector<std::pair<ImDrawList*, T>>>
class drawlist_storage : public Storage
{
public:
	std::tuple<T&, bool> operator[](ImDrawList* list)
	{
		for (auto& [ptr, entry]: *this)
		{
			if (list == ptr)
				return {entry, true};
		}

		/*for (auto& [ptr, entry]: add_on_new_frame_)
		{
			if (list == ptr)
				return {entry, false};
		}*/

		return {write_for_new_frame(list), false};
	}

	T* find(ImDrawList* list)
	{
		for (auto& [ptr, entry]: *this)
		{
			if (list == ptr)
				return std::addressof(entry);
		}

		return nullptr;
	}

private :
	void write_old_data( )
	{
		if (add_on_new_frame_.empty( ))
			return;

		Storage::reserve(Storage::size( ) + add_on_new_frame_.size( ));
		for (auto& entry: add_on_new_frame_)
			Storage::push_back(std::move(entry));

		Storage tmp;
		std::swap(add_on_new_frame_, tmp);
	}

	void prepare_textures( )
	{
		/*for (auto& [key,value]: *this)
		{
			value.reset_textures( );
		}*/
	}

public:
	void on_new_frame( )
	{
		write_old_data( );
		prepare_textures( );
	}

	void on_reset( )
	{
		for (auto& [key,value]: *this)
		{
			value.reset_textures( );
			value.reset_shaders( );
		}
	}

private:
	T& write_for_new_frame(ImDrawList* list)
	{
		return add_on_new_frame_.emplace_back(list, T( )).second;
	}

	Storage add_on_new_frame_;
};

static drawlist_storage<blur_effect> _Blur;

#ifdef _WIN32
void PostProcessing::setDevice(IDirect3DDevice9* device) noexcept
{
	_D3d_device = device;
}

void PostProcessing::clearBlurTextures( ) noexcept
{
	//BlurEffect::clearTextures( );
}

void PostProcessing::onDeviceReset( ) noexcept
{
	//BlurEffect::clearTextures( );
	//ChromaticAberration::clearTexture( );

	_Blur.on_reset( );
}
#endif

static bool _Invisible;

void PostProcessing::newFrame( ) noexcept
{
	const auto& size = ImGui::GetIO( ).DisplaySize;

	_Invisible = size.x == 0 || size.y == 0;
	if (_Invisible)
		return;

	_Back_buffer_width  = size.x;
	_Back_buffer_height = size.y;

	_Blur.on_new_frame( );
}

//unstable, but save tons fps on multiwindow gui
static ImDrawList* _Find_generated_texture_data(const ImGuiWindow* current, const ImRect& test_rect)
{
	const auto& windows = GImGui->Windows;
	if (windows.size( ) <= 2) //fallback and current
		return nullptr;

	const auto start = windows.begin( ) + 1;

	const auto current_ptr = std::find(start, windows.end( ), current);
	if (current_ptr == start)
		return nullptr;

	auto valid_windows = std::views::filter(std::ranges::subrange(start, current_ptr), PostProcessing::custom_textures_applicable);
	if (valid_windows.empty( ))
		return nullptr;

	// ReSharper disable once CppInconsistentNaming
	const auto _override = [&](const ImGuiWindow* w)
	{
		const auto& r = (w->OuterRectClipped); //InnerClipRect for better perfomance but ugliest look on small intersections
		const auto& l = test_rect;

		return r.Overlaps(l);
	};

	if (std::ranges::any_of(valid_windows, _override))
		return nullptr;

	return valid_windows.back( )->DrawList;
}

void PostProcessing::performFullscreenBlur(ImDrawList* drawList, float alpha) noexcept
{
	if (_Invisible)
		return;

	auto [blur, stored] = _Blur[drawList];
	if (!stored)
		return;

	auto& rect           = reinterpret_cast<const ImRect&>(drawList->_ClipRectStack.back( ));
	const auto wnd       = ImGui::GetCurrentWindowRead( );
	const auto generated = _Find_generated_texture_data(wnd, rect);

	if (!generated)
	{
	_RENDER:
		blur.set_color(alpha);
		blur.update(drawList);
		blur.render(drawList);
	}
	else
	{
		const auto blur1 = _Blur.find(generated);
		if (!blur1)
			goto _RENDER;
		if (!blur1->updated( ))
		{
			//blur1->update(generated);
			goto _RENDER;
		}
		if (blur1->get_color( ) != blur.get_color( ))
			goto _RENDER;
		if (blur1->get_clarity( ) != blur.get_clarity( ))
			goto _RENDER;
		if (blur1->get_down_sample( ) != blur.get_down_sample( ))
			goto _RENDER;

		blur1->render(drawList);
	}

#ifdef _DEBUG
	if (!(wnd->Flags & (ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)))
		blur.debug_update( );
#endif
}

void PostProcessing::performBlur(ImDrawList* drawList, const ImVec2& min, const ImVec2& max, float alpha) noexcept
{
	//BlurEffect::draws(drawList, min, max, alpha);
}

void PostProcessing::performFullscreenChromaticAberration(ImDrawList* drawList, float amount) noexcept
{
	//ChromaticAberration::draw(drawList, amount);
}

void PostProcessing::performFullscreenMonochrome(ImDrawList* drawList, float amount) noexcept
{
	//MonochromeEffect::draw(drawList, amount);
}

bool PostProcessing::custom_textures_applicable(const ImGuiWindow* wnd)
{
	if (!(wnd->WasActive) || wnd->SkipItems || wnd->Hidden)
		return false;

	if (wnd->Flags & ImGuiWindowFlags_NoBackground)
		return false;

	const auto& style = ImGui::GetStyle( );

	const auto bad_color = [&](ImGuiCol_ col)
	{
		auto& clr = style.Colors[col];
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
