// ReSharper disable CppInconsistentNaming
#pragma once

#define IMGUI_DISABLE_DEFAULT_ALLOCATORS 1
#define	IMGUI_USE_WCHAR32
#define IMGUI_DEFINE_MATH_OPERATORS
#define	IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#define IMGUI_IMPL_WIN32_DISABLE_GAMEPAD
#if __has_include(<stb_truetype.h>) && __has_include(<stb_rect_pack.h>)
#define	IMGUI_STB_TRUETYPE_FILENAME <stb_truetype.h>
#define	IMGUI_STB_RECT_PACK_FILENAME <stb_rect_pack.h>
#endif
#if __has_include("stb_sprintf.h")
#define IMGUI_USE_STB_SPRINTF
#endif
#define	ImDrawIdx size_t

#include <concepts>

struct ImDrawCmd;
struct ImDrawList;

namespace ImGui
{
	class ImDrawCallback_custom
	{
	public:
		template <std::invocable<ImDrawList*, ImDrawCmd*> Fn>
		ImDrawCallback_custom(Fn fn)
		{
			idx     = 1;
			static_ = fn;
		}

		template <typename C, std::invocable<C, ImDrawList*, ImDrawCmd*> Fn>
		ImDrawCallback_custom(C inst, Fn fn)
		{
			_Set_class_member(inst, fn);
		}

		template <typename C>
			requires(std::is_class_v<C>)
		ImDrawCallback_custom(C inst)
			: ImDrawCallback_custom(inst, &std::remove_cvref_t<C>::operator())
		{
		}

		template <typename C>
			requires(std::is_abstract_v<C>)
		ImDrawCallback_custom(C* inst, size_t index)
		{
			auto vtable0 = *(void**)inst;
			auto vtable  = (void**)vtable0;

			_Set_class_member(inst, vtable[index]);
		}

		ImDrawCallback_custom([[maybe_unused]] int i)
		{
#ifdef _DEBUG
			if (i != -1)
				throw;
#endif
			idx = index::RESET;
			_Fill_dummy( );
		}

		ImDrawCallback_custom( )
		{
			idx = index::UNSET;
			_Fill_dummy( );
		}

		void operator()(const ImDrawList* parent_list, const ImDrawCmd* cmd) const
		{
			switch (idx)
			{
				case index::STATIC:
					return static_(parent_list, cmd);
				case index::MEMBER:
					return member_(parent_list, cmd);
				default:
					throw;
			}
		}

		/*operator bool( ) const
		{
			return idx == 1 || idx == 2;
		}*/

		bool operator==(decltype(nullptr)) const
		{
			return static_ == 0;
		}

		bool operator==(const ImDrawCallback_custom& other) const
		{
			if (idx != other.idx)
				return 0;

			switch (idx)
			{
				case index::STATIC:
					return static_ == other.static_;
				case index::MEMBER:
					return __builtin_memcmp(dummy_, other.dummy_, _Dummy_size) == 0;
				case index::RESET:
				case index::UNSET:
					return true;
				default:
					throw;
			}
		}

		explicit operator bool( ) const
		{
			switch (idx)
			{
				case index::STATIC:
				case index::MEMBER:
					return true;
				default:
					return false;
			}
		}

	private:
		void _Fill_dummy( )
		{
			for (auto& d: dummy_)
				d = 0;
		}

		template <typename C, typename Fn>
		void _Set_class_member(C inst, Fn fn)
		{
			idx                    = index::MEMBER;
			member_.instance       = inst;
			(void*&)member_.member = (void*&)fn;
		}

		struct class_member_unwrapped
		{
			using member_t = void(__fastcall*)(void*, void*, const ImDrawList*, const ImDrawCmd*);

			void* instance;
			member_t member;

			void operator()(const ImDrawList* parent_list, const ImDrawCmd* cmd) const
			{
				member(instance, nullptr, parent_list, cmd);
			}
		};

		using static_function = void(*)(const ImDrawList* parent_list, const ImDrawCmd* cmd);

		static constexpr auto _Dummy_size = sizeof(class_member_unwrapped);

		union
		{
			static_function static_;
			class_member_unwrapped member_;
			uint8_t dummy_[_Dummy_size];
		};

		enum class index :uint8_t
		{
			UNSET
		  , STATIC
		  , MEMBER
		  , RESET
		};

		index idx;
	};
}

#define ImDrawCallback ImGui::ImDrawCallback_custom
