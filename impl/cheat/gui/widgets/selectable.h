#pragma once

#include "selectable base.h"
#include "text.h"
#include "cheat/gui/tools/button_info.h"

#include <imgui_internal.h>

#include <nstd/confirmable_value.h>

#include <memory>
#include <chrono>
// ReSharper disable CppInconsistentNaming
struct ImGuiStyle;
struct ImVec4;
using ImU32 = unsigned int;
enum ImGuiCol_;
// ReSharper restore CppInconsistentNaming

//template <std::floating_point T>
//ImVec4 operator*(const ImVec4& a, T fl)
//{
//	const auto fl2 = static_cast<float>(fl);
//	return {a.x * fl2, a.y * fl2, a.z * fl2, a.w * fl2};
//}

inline bool operator==(const ImVec4& a, const ImVec4& b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

inline bool operator!=(const ImVec4& a, const ImVec4& b)
{
	return !(a == b);
}

namespace std
{
	template <typename T>
	class function;
	template <typename T>
	class optional;

	inline ImVec4 lerp(const ImVec4& a, const ImVec4& b, float val)
	{
		return {lerp(a.x, b.x, val), lerp(a.y, b.y, val), lerp(a.z, b.z, val), lerp(a.w, b.w, val)};
	}
}

namespace cheat::gui::tools
{
	class animator;
}

namespace cheat::gui::widgets
{
	class animation_base
	{
	protected:
		virtual ~animation_base() = default;

	public:
		virtual void update() = 0;
	};

	/*namespace detail
	{
		template <typename T>
		bool equal_helper(const T& a, const T& b)
		{
			if constexpr (std::equality_comparable<T>)
				return a == b;
			else if constexpr (std::is_trivially_destructible_v<T>)
				return std::memcmp(std::addressof(a), std::addressof(b), sizeof(T)) == 0;
			else
			{
				static_assert(std::_Always_false<T>, __FUNCTION__": unable to equality compare elements");
				return false;
			}
		}
	}*/

	template <typename T>
	class animation_property_target
	{
	public:
		virtual ~animation_property_target() = default;
		virtual void set_target(T& obj) =0;

		virtual T& get_value() =0;
	};

	template <typename T>
	class animation_property_target_external final : public animation_property_target<T>
	{
	public:
		void set_target(T& obj) override
		{
			target_val_ = std::addressof(obj);
		}

		T& get_value() override
		{
			return *target_val_;
		}

	private:
		T* target_val_ = nullptr;
	};

	template <typename T>
	class animation_property_target_internal final : public animation_property_target<T>
	{
	public:
		animation_property_target_internal() = default;

		void set_target(T& obj) override
		{
			target_val_ = (obj);
		}

		T& get_value() override
		{
			return target_val_;
		}

	private:
		T target_val_;
	};

	template <class T
			, class Clock = std::chrono::high_resolution_clock>
	class animation_property : public animation_base
	{
	public:
		using duration = typename Clock::duration;
		using time_point = typename Clock::time_point;

		using unachieved_value = std::optional<T>;

		using target_type = animation_property_target<T>;
		using target_holder = std::unique_ptr<target_type>;

		template <template<class V>typename Prop>
		using target_holder_fwd = Prop<T>;

		~animation_property() override
		{
			//if (target_val_)
			//	*target_val_ = std::move(start_val_);
		}

		animation_property()
		{
			static_assert(std::copyable<T>);
			static_assert(std::chrono::is_clock_v<Clock>);
			start_val_ = temp_val_ = end_val_ = T( );
		}

		animation_property(const animation_property& other)                = delete;
		animation_property(animation_property&& other) noexcept            = default;
		animation_property& operator=(const animation_property& other)     = delete;
		animation_property& operator=(animation_property&& other) noexcept = default;

		//----

		void set_target(target_holder&& holder)
		{
			target_val_ = std::move(holder);
		}

		template <template<class>typename Prop>
			requires(std::derived_from<Prop<T>, target_type>)
		void set_target()
		{
			set_target(std::make_unique<Prop<T>>( ));
		}

		T& get_target_value()
		{
			return target_val_->get_value( );
		}

		//----

		void set_start(const T& start)
		{
			start_val_ = start;
		}

		void set_end(const T& end)
		{
			end_val_ = end;
		}

		const T& get_start_val() const
		{
			return start_val_;
		}

		const T& get_end_val() const
		{
			return end_val_;
		}

		void update_end(const T& new_end, bool force = false)
		{
			if (!force && new_end == end_val_)
				return;

			if (finished_ || (start_val_ != new_end))
			{
				start_val_    = std::move(end_val_);
				end_val_      = new_end;
				temp_val_old_ = temp_val_;
			}
			else
			{
				const auto elapsed_time   = last_time_ - start_time_;
				const auto time_remaining = duration_ - elapsed_time;

				//some time has passed, now we only need to wait for this segment
				start_time_ = last_time_ - time_remaining;
				this->inverse( );
			}

			this->start(true, true);
		}

		void set_duration(duration duration)
		{
			duration_ = duration;
		}

		//----

		void start(bool wait_for_finish, bool delayed)
		{
			if (!finished_ && wait_for_finish)
				return;
			/*if (target_val_ == nullptr)
				set_target<animation_property_target_internal>( );*/
			restart(delayed);
		}

		void restart(bool delayed)
		{
			if (delayed)
			{
				restart_on_update_ = true;
				return;
			}

			temp_val_          = start_val_;
			finished_          = false;
			restart_on_update_ = false;
			update_started_    = false;
			temp_val_old_.reset( );
			start_time_ = Clock::now( );
		}

		void inverse()
		{
			std::swap(start_val_, end_val_);
		}

		//---

		bool running() const
		{
			return finished_;
		}

		//---

		void update() final
		{
			if (restart_on_update_)
				this->restart(false);
			else if (finished_)
				return;

			duration frame_time, elapsed_time;
			if (update_started_)
			{
				auto now     = Clock::now( );
				frame_time   = now - last_time_;
				elapsed_time = now - start_time_;
				last_time_   = now;
			}
			else
			{
				update_started_ = true;
				frame_time      = elapsed_time = duration::zero( );
				last_time_      = start_time_;
				return;
			}

			if (elapsed_time >= duration_)
			{
				finished_                 = true;
				target_val_->get_value( ) = end_val_;
			}
			else
			{
				this->update_impl(temp_val_old_, start_val_, temp_val_, end_val_, frame_time, elapsed_time, duration_);
				target_val_->get_value( ) = temp_val_;
			}
		}

	protected:
		virtual void update_impl(const unachieved_value& current_unachieved, const T& from, T& current, const T& to,
								 duration frame_time, duration elapsed_time, duration duration) = 0;

	private:
		time_point start_time_, last_time_;
		duration duration_;
		target_holder target_val_;
		T start_val_, temp_val_, end_val_;
		unachieved_value temp_val_old_;
		bool update_started_    = false;
		bool finished_          = true;
		bool restart_on_update_ = true;
	};

	template <typename T/*, class Clock*/>
	class animation_property_linear : public animation_property<T/*, Clock*/>
	{
	protected:
		using base = animation_property<T>;
		using duration = typename base::duration;
		using unachieved_value = typename base::unachieved_value;

		void update_impl(const unachieved_value& current_old, const T& from, T& current, const T& to,
						 duration frame_time, duration elapsed_time, duration duration) override
		{
			const auto diff = static_cast<float>(elapsed_time.count( )) / static_cast<float>(duration.count( ));
			current         = std::lerp(current_old.value_or(from), to, diff);
		}
	};

	//----

	class selectable_bg : public selectable_base
	{
	public:
		selectable_bg();
		~selectable_bg() override;

		selectable_bg(selectable_bg&&) noexcept;
		selectable_bg& operator=(selectable_bg&&) noexcept;

		void set_background_color_modifier(std::unique_ptr<animation_property<ImVec4>>&& mod);

	protected:
		bool render(ImGuiWindow* window, ImRect& bb, ImGuiID id, bool outer_spacing);
		virtual void render_background(ImGuiWindow* window, ImRect& bb);

	private:
		struct data_type;
		std::unique_ptr<data_type> data_;
	};

	class selectable : public selectable_bg
					 , public text
	{
	public:
		selectable();
		~selectable() override;

		selectable(selectable&&) noexcept;
		selectable& operator=(selectable&&) noexcept;

		void render() override;

	private:
		struct data_type;
		std::unique_ptr<data_type> data_;
	};

	//---------

	//todo: move to tools namesapce
	template <typename T>
	ImGuiID make_imgui_id(const T& obj, ImGuiWindow* wnd)
	{
		static_assert(sizeof(ImGuiID) % sizeof(void*) == 0);

		const auto ptr     = std::addressof(obj);
		const auto id_temp = reinterpret_cast<uintptr_t>(ptr);

		constexpr auto arr_size = sizeof(uintptr_t) / sizeof(ImGuiID);
		using arr = std::array<ImGuiID, arr_size>;

		auto& id_arr    = reinterpret_cast<const arr&>(id_temp);
		const auto seed = wnd->IDStack.back( );

		const auto make_id = [&]<size_t ...I>(std::index_sequence<I...>)
		{
			return (id_arr[I] ^ ...) ^ seed;
		};

		const ImGuiID id = make_id(std::make_index_sequence<arr_size>( ));

		ImGui::KeepAliveID(id);
		return id;
	}

	//move to tools namespace
	class animation_color_helper
	{
		animation_property<ImVec4>* animation_;

	public:
		animation_color_helper(animation_property<ImVec4>* const animation)
			: animation_(animation)
		{
		}

		ImU32 operator()(const ImVec4& clr) const
		{
			ImVec4 result;
			if (!animation_)
			{
				result = clr;
			}
			else
			{
				animation_->update_end(clr);
				animation_->update( );
				result = animation_->get_target_value( );
			}
			const auto alpha = ImGui::GetStyle( ).Alpha;
			result.w *= alpha;
			return ImGui::ColorConvertFloat4ToU32(result);
		}
	};

	//move to tools namespace
	ImU32 get_selectable_color(tools::button_state state, bool idle_visible
							 , const ImVec4& idle_clr, const ImVec4& hovered_clr, const ImVec4& held_clr, const ImVec4& pressed_clr
							 , animation_property<ImVec4>* animation);
	//move to tools namespace
	ImU32 get_boolean_color(bool value
						  , const ImVec4& clr
						  , animation_property<ImVec4>* animation);

	tools::button_state selectable2(const tools::cached_text& label, bool selected
								  , animation_property<ImVec4>* bg_animation = 0);
}
