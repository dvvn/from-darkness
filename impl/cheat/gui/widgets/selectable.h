#pragma once

#include "selectable base.h"
#include "text.h"

#include <memory>
#include <chrono>

#include <imgui_internal.h>

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

	namespace detail
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
	}

	template <class T, class Clock = std::chrono::high_resolution_clock>
	class animation_property : public animation_base
	{
	public:
		using clock_duration = typename Clock::duration;

		using unachieved_value = std::optional<T>;

		animation_property()
		{
			static_assert(std::copyable<T>);
			static_assert(std::chrono::is_clock_v<Clock>);
		}

		~animation_property() override
		{
			if (target_val_)
				*target_val_ = std::move(start_val_);
		}

		void set_target(T& obj)
		{
			target_val_ = std::addressof(obj);
		}

		void set_start(const T& start)
		{
			start_val_ = start;
		}

		void set_end(const T& end)
		{
			end_val_ = end;
		}

		void update_end(const T& new_end)
		{
			if (finished_ || !detail::equal_helper(start_val_, new_end))
			{
				start_val_    = std::move(end_val_);
				end_val_      = new_end;
				temp_val_old_ = temp_val_;
			}
			else
			{
				const auto time_elapsed   = last_time_ - start_time_;
				const auto time_remaining = duration_ - time_elapsed;

				//some time has passed, now we only need to wait for this segment
				start_time_ = last_time_ - time_remaining;
				this->inverse( );
			}

			this->start(true, true);
		}

		void set_duration(clock_duration duration)
		{
			duration_ = duration;
		}

		//----

		void start(bool wait_for_finish, bool delayed)
		{
			if (!finished_ && wait_for_finish)
				return;

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

			clock_duration frame_time, time_elapsed;
			// ReSharper disable once CppInconsistentNaming
			const auto _Get_current_time = [&]
			{
				if (!update_started_)
				{
					frame_time   = clock_duration::zero( );
					time_elapsed = clock_duration::zero( );
					return start_time_;
				}
				else
				{
					auto now     = Clock::now( );
					frame_time   = now - last_time_;
					time_elapsed = now - start_time_;
					return now;
				}
			};
			last_time_ = _Get_current_time( );

			// ReSharper disable once CppInconsistentNaming
			const auto _Get_new_target_value = [&]()-> const T&
			{
				if (time_elapsed >= duration_)
				{
					finished_ = true;
					return end_val_;
				}

				if (update_started_)
				{
					this->update_impl(temp_val_old_, start_val_, temp_val_, end_val_, duration_, frame_time, time_elapsed);
#if 0
					if (temp_val_old_.has_value( ))
					{
						// ReSharper disable once CppInconsistentNaming
						const auto _Any_of = [&]<class ...Ts>(Ts&& ...args)
						{
							return (detail::equal_helper(*temp_val_old_, args) || ...);
						};

						if (_Any_of(start_val_, temp_val_, end_val_))
							temp_val_old_.reset( );
					}
#endif
				}
				return temp_val_;
			};
			*target_val_ = _Get_new_target_value( );

			update_started_ = true;
		}

	protected:
		virtual void update_impl(const unachieved_value& current_unachieved, const T& from, T& current, const T& to,
								 clock_duration duration, clock_duration frame_time, clock_duration time_elapsed) = 0;

	private:
		typename Clock::time_point start_time_, last_time_;
		clock_duration duration_;
		T* target_val_ = nullptr;
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
		using clock_duration = typename base::clock_duration;
		using unachieved_value = typename base::unachieved_value;

		void update_impl(const unachieved_value& current_old, const T& from, T& current, const T& to,
						 clock_duration duration, clock_duration frame_time, clock_duration time_elapsed) override
		{
			const auto diff = static_cast<float>(time_elapsed.count( )) / static_cast<float>(duration.count( ));

			current = std::lerp(current_old.value_or(from), to, diff);
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
}
