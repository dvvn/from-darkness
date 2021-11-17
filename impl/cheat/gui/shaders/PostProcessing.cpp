#ifdef _WIN32
#include <d3d9.h>
#else
#include "imgui/GL/gl3w.h"
#endif

#ifdef _WIN32
// shaders are build during compilation and header files are created
#include "Build/blur_x.h"
#include "Build/blur_y.h"
#include "Build/chromatic_aberration.h"
#include "Build/monochrome.h"
#endif

#include "PostProcessing.h"

#include <nstd/runtime_assert_fwd.h>

#include <imgui_internal.h>

#include <vector>

static float _Back_buffer_width  = 0;
static float _Back_buffer_height = 0;

#ifdef _WIN32
static IDirect3DDevice9* device; // DO NOT RELEASE!

[[nodiscard]] static IDirect3DTexture9* _Create_texture(int width, int height) noexcept
{
	IDirect3DTexture9* texture;
	[[maybe_unused]] const auto result = device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, std::addressof(texture), nullptr);
	runtime_assert(result == D3D_OK);
	return texture;
}

static constexpr auto _Com_deleter = []<typename T>(T* ptr)
{
	ptr->Release( );
};

template <typename T, typename Base = std::unique_ptr<T, std::remove_const_t<decltype(_Com_deleter)>>>
struct comptr : Base
{
	using typename Base::pointer;
	using typename Base::deleter_type;

	comptr( )
		: Base( )
	{
	}

	comptr(pointer ptr, deleter_type dx = {})
		: Base(ptr, std::move(dx))
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

	/*pointer* get_addressof( ) const
	{
		return std::addressof(Base::get( ));
	}*/

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
	[[maybe_unused]] const auto result1 = device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, back_buffer.set( ));
	runtime_assert(result1 == D3D_OK);
	comptr<IDirect3DSurface9> surface;
	[[maybe_unused]] const auto result2 = texture->GetSurfaceLevel(0, surface.set( ));
	runtime_assert(result2 == D3D_OK);
	[[maybe_unused]] const auto result3 = device->StretchRect(back_buffer.get( ), nullptr, surface.get( ), nullptr, filtering);
	runtime_assert(result3 == D3D_OK);
}

static void _Set_render_target(IDirect3DTexture9* rtTexture) noexcept
{
	comptr<IDirect3DSurface9> surface;
	[[maybe_unused]] const auto result = rtTexture->GetSurfaceLevel(0, surface.set( ));
	runtime_assert(result == D3D_OK);
	device->SetRenderTarget(0, surface.get( ));
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
		[[maybe_unused]] const auto result = device->CreatePixelShader(reinterpret_cast<const DWORD*>(shader_source_function), this->set( ));
		runtime_assert(result == D3D_OK);
	}

	basic_shader_program(const basic_shader_program& other)                = delete;
	basic_shader_program& operator=(const basic_shader_program& other)     = delete;
	basic_shader_program(basic_shader_program&& other) noexcept            = default;
	basic_shader_program& operator=(basic_shader_program&& other) noexcept = default;

	void use(float uniform, int location) const noexcept
	{
#ifdef _WIN32
		device->SetPixelShader(this->get( ));
		const float params[4] = {uniform};
		device->SetPixelShaderConstantF(location, params, 1);
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
using custom_texture = comptr<IDirect3DTexture9>;

class basic_shader_updater
{
public:
	virtual ~basic_shader_updater( ) = default;

protected:
	virtual void begin(const ImDrawList*, const ImDrawCmd*) = 0;
	virtual void process_texture(ImDrawList* drawList) = 0;
	virtual void end(const ImDrawList*, const ImDrawCmd*) = 0;

public:
	virtual void draw(ImDrawList* drawList) noexcept
	{
		drawList->AddCallback({this, 1}, nullptr);
		this->process_texture(drawList);
		drawList->AddCallback({this, 3}, nullptr);
		drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
	}
};

struct shader_data
{
	custom_texture texture;
	shader_program shader;
};

class shader_updater : public basic_shader_updater
{
public:
	shader_updater(shader_data&& data)
		: data_(std::move(data))
	{
	}

private:
	shader_data data_;
};

class blur_shader_updater : public basic_shader_updater
{
public:
	blur_shader_updater( ) = default;

	/*blur_shader_updater(float alpha, int down_sample = 4)
	{
		update_color(alpha);
		update_textures(down_sample);
		update_shaders( );
	}*/

	bool operator==(nullptr_t) const
	{
		auto& [x, y] = data_;
		return !x.shader && !x.texture && !y.shader && !y.texture;
	}

protected:
	void begin(const ImDrawList*, const ImDrawCmd*) override
	{
		device->GetRenderTarget(0, std::addressof(backup_));

		_Copy_back_buffer_to_texture(data_.x.texture.get( ), D3DTEXF_LINEAR);

		device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

		const D3DMATRIX projection{
			{
				{
					1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f / (_Back_buffer_width / down_sample_), 1.0f / (_Back_buffer_height / down_sample_), 0.0f
				  , 1.0f
				}
			}
		};
		device->SetVertexShaderConstantF(0, &projection.m[0][0], 4);
	}

private:
	void first_pass(const ImDrawList*, const ImDrawCmd*) const noexcept
	{
		auto& [x,y] = data_;

		x.shader.use(1.0f / (_Back_buffer_width / down_sample_), 0);
		_Set_render_target(y.texture.get( ));
	}

	void second_pass(const ImDrawList*, const ImDrawCmd*) const noexcept
	{
		auto& [x,y] = data_;

		y.shader.use(1.0f / (_Back_buffer_height / down_sample_), 0);
		_Set_render_target(x.texture.get( ));
	}

protected:
	void process_texture(ImDrawList* drawList) override
	{
		const auto min = ImVec2(0, 0);
		const auto max = ImVec2(_Back_buffer_width, _Back_buffer_height);

		auto& [x,y] = data_;

		for (auto i = 0; i < 8; ++i)
		{
			drawList->AddCallback({this, &blur_shader_updater::first_pass}, nullptr);
			drawList->AddImage(x.texture.get( ), min, max);
			drawList->AddCallback({this, &blur_shader_updater::second_pass}, nullptr);
			drawList->AddImage(y.texture.get( ), min, max);
		}
	}

	void end(const ImDrawList*, const ImDrawCmd*) override
	{
		device->SetRenderTarget(0, backup_);
		backup_->Release( );

		device->SetPixelShader(nullptr);
		device->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
	}

public:
	void draw(ImDrawList* drawList) noexcept override
	{
		basic_shader_updater::draw(drawList);
		drawList->AddImage(data_.x.texture.get( ), {0, 0}, {_Back_buffer_width, _Back_buffer_height}, {0.0f, 0.0f}, {1.0f, 1.0f}, color_);
	}

	void update_color(float alpha)
	{
		if (alpha_ == alpha)
			return;

		color_ = IM_COL32(255, 255, 255, 255 * alpha);
		alpha_ = alpha;
	}

private:
	ImU32 color_ = 0;
	float alpha_ = -1;

public:
	void update_textures(int down_sample)
	{
		auto& [x, y] = data_;

		const auto create = [&]( )-> custom_texture
		{
			return _Create_texture(_Back_buffer_width / down_sample_, _Back_buffer_height / down_sample_);
		};

		if (down_sample_ != down_sample)
		{
			down_sample_ = down_sample;
			x.texture    = create( );
			y.texture    = create( );
		}
		else
		{
			if (!x.texture)
				x.texture = create( );
			if (!y.texture)
				y.texture = create( );
		}
	}

	void update_shaders( )
	{
		auto& [x, y] = data_;

		if (!x.shader)
			x.shader = {blur_x};
		if (!y.shader)
			y.shader = {blur_y};
	}

	void reset_textures( )
	{
		auto& [x, y] = data_;
		x.texture.reset( );
		y.texture.reset( );
	}

	void reset_shaders( )
	{
		auto& [x, y] = data_;
		x.shader.reset( );
		y.shader.reset( );
	}

private:
	int down_sample_ = -1;

	struct
	{
		shader_data x;
		shader_data y;
	} data_;

	IDirect3DSurface9* backup_ = nullptr;
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

template <typename T, class Storage = std::vector<std::pair<ImDrawList*, T>>>
class textures_storage : public Storage
{
public:
	std::tuple<T&, bool> operator[](ImDrawList* list)
	{
		for (auto& [ptr, entry]: *this)
		{
			if (list == ptr)
				return {entry, true};
		}

		return {add_on_new_frame_.emplace_back(list, T( )).second, false};
	}

	void on_new_frame( )
	{
		if (add_on_new_frame_.empty( ))
			return;

		Storage::reserve(Storage::size( ) + add_on_new_frame_.size( ));
		for (auto& entry: add_on_new_frame_)
			Storage::push_back(std::move(entry));

		Storage tmp;
		std::swap(add_on_new_frame_, tmp);
	}

private:
	Storage add_on_new_frame_;
};

static textures_storage<blur_shader_updater> _Blur;

#ifdef _WIN32
void PostProcessing::setDevice(IDirect3DDevice9* device) noexcept
{
	::device = device;
}

void PostProcessing::clearBlurTextures( ) noexcept
{
	//BlurEffect::clearTextures( );
}

void PostProcessing::onDeviceReset( ) noexcept
{
	//BlurEffect::clearTextures( );
	//ChromaticAberration::clearTexture( );

	_Blur.clear( );
}
#endif

void PostProcessing::newFrame( ) noexcept
{
	const auto [width, height] = ImGui::GetIO( ).DisplaySize;
	//if (_Back_buffer_width != width)
	_Back_buffer_width = width;
	//if (_Back_buffer_height != height)
	_Back_buffer_height = height;

	_Blur.on_new_frame( );
	/*if (_Blur == nullptr)
		_Blur = {1, 4};
	else
		_Blur.update_textures( );*/
}

void PostProcessing::performFullscreenBlur(ImDrawList* drawList, float alpha) noexcept
{
	auto [blur, stored] = _Blur[drawList];

	if (blur == nullptr)
		blur.update_shaders( );
	else
		blur.reset_textures( );

	if (!stored)
		return;

	blur.update_color(alpha);
	blur.update_textures(4);
	blur.draw(drawList);
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
