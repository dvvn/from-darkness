#pragma once
#include <imgui_internal.h>

#include <array>

namespace cheat::gui::tools
{
	class imgui_id
	{
	public:
		static_assert(sizeof(ImGuiID) % sizeof(uintptr_t) == 0);

		template <typename T>
		imgui_id(const T& obj, const ImGuiWindow* wnd = ImGui::GetCurrentWindowRead( ))
		{
			constexpr auto arr_size = sizeof(uintptr_t) / sizeof(ImGuiID);

			const auto make_id = [&]<size_t ...I>(std::index_sequence<I...>)
			{
				using arr = std::array<ImGuiID, arr_size>;

				const auto ptr = [&]
				{
					if constexpr (std::is_pointer_v<T>)
						return obj;
					else
						return std::addressof(obj);
				}( );
				const auto id_temp = reinterpret_cast<uintptr_t>(ptr);

				auto& id_arr    = reinterpret_cast<const arr&>(id_temp);
				const auto seed = wnd->IDStack.back( );
				return (id_arr[I] ^ ...) ^ seed;
			};

			const ImGuiID id = make_id(std::make_index_sequence<arr_size>( ));

			ImGui::KeepAliveID(id);
			id_ = id;
		}

		operator ImGuiID() const { return id_; }

	private:
		ImGuiID id_;
	};
}
