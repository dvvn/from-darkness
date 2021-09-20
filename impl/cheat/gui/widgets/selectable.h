#pragma once

#include "selectable base.h"
#include "text.h"

#include <memory>
#include <chrono>

#include <imgui.h>

// ReSharper disable CppInconsistentNaming
struct ImGuiStyle;
struct ImVec4;
using ImU32 = unsigned int;
enum ImGuiCol_;
// ReSharper restore CppInconsistentNaming

namespace std
{
	template <typename T>
	class function;
	template <typename T>
	class optional;
}

namespace cheat::gui::tools
{
	class animator;
}

namespace std
{
	ImVec4 lerp(const ImVec4& a,const ImVec4&b,double diff);
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

	template <class T, class Clock = std::chrono::high_resolution_clock>
	class animation_property : public animation_base
	{
	public:
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

		void set_start(const T& start) { start_val_ = start; }
		void set_start(T&& start) { start_val_ = std::move(start); }
		void set_end(const T& end) { end_val_ = end; }
		void set_end(T&& end) { end_val_ = std::move(end); }

		void set_duration(typename Clock::duration duration)
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
				restart_on_update_ = 1;
				return;
			}
			current_val_       = start_val_;
			finished_          = false;
			restart_on_update_ = false;
			start_time_        = Clock::now( );
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

			T target;

			const auto diff = Clock::now( ) - start_time_;
			if (diff < duration_)
			{
				const auto normalize = []<class A,class B,class C>(A val, B min, C max)
				{
					const auto range = static_cast<double>(max - min);
					return (val - min) / range;
				};

				const auto normalized = normalize(diff.count( ), 0, (duration_.count( )));

				this->update_impl(/*start_val_,*/ current_val_, end_val_, normalized);
				target = current_val_;
			}
			else
			{
				finished_ = true;
				target    = current_val_ = end_val_;
			}

			*target_val_ = std::move(target);
		}

	protected:
		/**
		 * \brief
		 * \param current: value between start and end
		 * \param to: target value
		 * \param diff_normalized: time difference converted to 0..1 range
		 */
		virtual void update_impl(T& current, const T& to, double diff_normalized) = 0;

	private:
		typename Clock::time_point start_time_;
		typename Clock::duration   duration_;

		T* target_val_ = 0;
		T  start_val_, current_val_, end_val_;

		bool finished_          = true;
		bool restart_on_update_ = 1;
	};
	
	template <typename T>
	class animation_property_linear : public animation_property<T>
	{
	protected:
		void update_impl(T& current, const T& to, double diff_normalized) override
		{
			current = std::lerp(current, to, diff_normalized);
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
		bool         render(ImGuiWindow* window, ImRect& bb, ImGuiID id, bool outer_spacing);
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
