// ReSharper disable CppInconsistentNaming
#pragma once

#include <concepts>
#include <cstring>
#include <cstddef>
#include <exception>

struct ImDrawCmd;
struct ImDrawList;

namespace ImGui
{
	template <class C>
	auto _Get_virtual_func(const size_t idx, C* instance) noexcept
	{
		auto num = reinterpret_cast<uintptr_t>(instance);
		auto vtable0 = *reinterpret_cast<void**>(num);
		auto vtable = static_cast<void**>(vtable0);
		return vtable[idx];
	}

	class ImDrawCallback_custom
	{
		class class_member_unwrapped
		{
			struct dummy_struct { };

			union
			{
				void(__thiscall dummy_struct::* fn_)(const ImDrawList*, const ImDrawCmd*);
				void* fn_ptr_;
			};
			dummy_struct* inst_;

		public:
			class_member_unwrapped( )
				:fn_ptr_(nullptr), inst_(nullptr)
			{
			}

			template <class C>
			class_member_unwrapped(void* fn, C* inst)
				: fn_ptr_(fn), inst_(reinterpret_cast<dummy_struct*>(inst))
			{
			}

			template <class C, std::invocable<C*, ImDrawList*, ImDrawCmd*> Fn>
			class_member_unwrapped(Fn fn, C* inst)
				: class_member_unwrapped(reinterpret_cast<void*&>(fn), inst)
			{
			}

			template <class C>
			class_member_unwrapped(const size_t index, C* inst)
				: class_member_unwrapped(_Get_virtual_func(index, inst), inst)
			{
				static_assert(std::is_abstract_v<C>);
			}


			void operator()(const ImDrawList* parent_list, const ImDrawCmd* cmd) const
			{
				std::invoke(fn_, inst_, parent_list, cmd);
			}

			bool operator ==(nullptr_t) const noexcept
			{
				return fn_ptr_ == nullptr || inst_ == nullptr;
			}

			bool operator ==(const class_member_unwrapped& other) const noexcept
			{
				return fn_ptr_ == other.fn_ptr_ && inst_ == other.inst_;
			}
		};

		enum class index :uint8_t
		{
			UNSET
			, STATIC
			, MEMBER
			, RESET
		};

	public:
		using static_func = void(*)(const ImDrawList* parent_list, const ImDrawCmd* cmd);

		ImDrawCallback_custom(static_func fn)
			:idx_(index::STATIC), static_(fn)
		{
		}

		template <typename T, class C>
		ImDrawCallback_custom(T fn_src, C* inst)
			: idx_(index::MEMBER), member_(fn_src, inst)
		{
		}

		ImDrawCallback_custom([[maybe_unused]] const int i)
			:idx_(index::RESET)
		{
#ifdef _DEBUG
			if (i != -1)
				throw;
#endif
		}

		ImDrawCallback_custom( )
			:idx_(index::UNSET)
		{
		}

		void operator()(const ImDrawList* parent_list, const ImDrawCmd* cmd) const
		{
			switch (idx_)
			{
			case index::STATIC:
				return static_(parent_list, cmd);
			case index::MEMBER:
				return member_(parent_list, cmd);
			default:
				std::terminate( );
			}
		}

		bool operator==(nullptr_t) const noexcept
		{
			switch (idx_)
			{
			case index::UNSET:
				return true;
			case index::STATIC:
				return static_ == nullptr;
			case index::MEMBER:
				return member_ == nullptr;
			case index::RESET:
				return false;
			default:
				std::terminate( );
			}
		}

		bool operator==(const ImDrawCallback_custom& other) const noexcept
		{
			if (idx_ != other.idx_)
				return false;

			switch (idx_)
			{
			case index::STATIC:
				return static_ == other.static_;
			case index::MEMBER:
				return member_ == other.member_;
			case index::RESET:
			case index::UNSET:
				return true;
			default:
				std::terminate( );
			}
		}

		explicit operator bool( ) const
		{
			return idx_ == index::STATIC || idx_ == index::MEMBER;
		}

	private:
		union
		{
			static_func static_;
			class_member_unwrapped member_;
		};

		index idx_;
	};

	inline int _Custom_callback_memcmp(const ImGui::ImDrawCallback_custom* l, const ImGui::ImDrawCallback_custom* r, const size_t size) noexcept
	{
#ifdef _DEBUG
		if (size != sizeof(ImGui::ImDrawCallback_custom))
			std::terminate( );
#endif
		return *l == *r ? 0 : 1;
	}
}

#define IMGUI_CUSTOM_CALLBACK_MEMCMP(_CONST_,_CONST1_)\
inline int memcmp(_CONST_ ImGui::ImDrawCallback_custom* l, _CONST1_ ImGui::ImDrawCallback_custom* r, const size_t size) noexcept\
{\
	return ImGui::_Custom_callback_memcmp(l, r, size);\
}

IMGUI_CUSTOM_CALLBACK_MEMCMP(const, const);
IMGUI_CUSTOM_CALLBACK_MEMCMP(const, );
IMGUI_CUSTOM_CALLBACK_MEMCMP(, const);
IMGUI_CUSTOM_CALLBACK_MEMCMP(, );

#undef IMGUI_CUSTOM_CALLBACK_MEMCMP

//------------------

#define	ImDrawIdx uint32_t
#define ImDrawCallback ImGui::ImDrawCallback_custom
#define	IMGUI_USE_WCHAR32
#define IMGUI_DEFINE_MATH_OPERATORS
#define	IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_DISABLE_OBSOLETE_KEYIO
#define IMGUI_IMPL_WIN32_DISABLE_GAMEPAD

#if __has_include(<stb_truetype.h>) && __has_include(<stb_rect_pack.h>)
#define	IMGUI_STB_TRUETYPE_FILENAME <stb_truetype.h>
#define	IMGUI_STB_RECT_PACK_FILENAME <stb_rect_pack.h>
#endif

#if __has_include("stb_sprintf.h")
#define IMGUI_USE_STB_SPRINTF
#endif

#include <intrin.h>
#define IMGUI_ENABLE_SSE

#ifdef _DEBUG
#define IMGUI_DISABLE_DEFAULT_ALLOCATORS 1
#else
//#define IMGUI_DISABLE_DEMO_WINDOWS
//#define IMGUI_DISABLE_METRICS_WINDOW
#endif

struct IDirect3DTexture9;
#define ImTextureID IDirect3DTexture9*

#define IMGUI_USE_BGRA_PACKED_COLOR
#define IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT struct ImDrawVert { ImVec2 pos; float z; ImU32 col; ImVec2 uv; }
