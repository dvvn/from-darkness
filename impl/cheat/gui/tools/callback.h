#pragma once

//#define CHEAT_GUI_CALLBACK_HAVE_INDEX

// ReSharper disable CppInconsistentNaming
struct ImRect;
using ImGuiID = unsigned int;
// ReSharper restore CppInconsistentNaming

namespace std
{
	template <class Fty>
	class function;
}

#include <memory>

namespace cheat::gui::objects
{
	class renderable;
}

namespace cheat::gui::tools
{
	struct callback_state
	{
		callback_state( );

		void tick( );

		size_t ticks = 0;
		double start;
		double duration = 0;
	};

	class callback_fn
	{
		callback_fn( );
	public:
		~callback_fn( );

		callback_fn(callback_fn&&) noexcept;
		callback_fn& operator=(callback_fn&&) noexcept;

		using func_type = std::function<void(const callback_state& state)>;
		using func_type2 = std::function<void( )>;

		callback_fn(func_type&& fn);
		callback_fn(func_type2&& fn);

		void operator()(const callback_state& state) const;

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};

#ifdef CHEAT_GUI_CALLBACK_HAVE_INDEX
	using callback_id = uintptr_t;
#endif

	struct callback_info
	{
		callback_info(callback_fn&& func
#ifdef CHEAT_GUI_CALLBACK_HAVE_INDEX
					, callback_id id
#endif
					, bool repeat);
		~callback_info( );

		callback_info(callback_info&&) noexcept;
		callback_info& operator=(callback_info&&) noexcept;

		callback_fn fn;
#ifdef CHEAT_GUI_CALLBACK_HAVE_INDEX
		callback_id id = -1;
#endif

		bool repeat; //todo: timer
		bool skip = false;
	};

	template <class T>
		requires(std::constructible_from<callback_fn, T>)
	callback_info make_callback_info(T&& func
#ifdef CHEAT_GUI_CALLBACK_HAVE_INDEX
								   , callback_id id
#endif
								   , bool repeat = false)
	{
		auto fn = callback_fn(std::forward<T>(func));
		return
		{
			std::move(fn)
#ifdef CHEAT_GUI_CALLBACK_HAVE_INDEX
		  , id
#endif
		  , repeat
		};
	}

	class callback
	{
	public:
		callback( );
		~callback( );

		callback(callback&&) noexcept;
		callback& operator=(callback&&) noexcept;

		bool active( ) const;
		void reset( );
		void operator()( );

		void add(callback_info&& info);
#ifdef CHEAT_GUI_CALLBACK_HAVE_INDEX
		bool erase(callback_id ids);
#endif

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};

	class two_way_callback
	{
	public:
		enum ways :uint8_t
		{
			WAY_TRUE
		  , WAY_FALSE
		};

		callback& operator[](ways way);

		void operator()(bool value);

	private:
		callback way_true, way_false;
	};
}
