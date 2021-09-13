#pragma once

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

	struct callback_data
	{
		virtual ~callback_data( ) = default; //for dynamic_cast

		callback_data(objects::renderable* caller, ImGuiID id);
		callback_data(objects::renderable* caller);

		objects::renderable* caller;
		ImGuiID              id;
	};

	using callback_func_type = std::function<void(const callback_data&, const callback_state&)>;

	struct callback_info
	{
		callback_info(callback_func_type&& func, bool repeat);
		~callback_info( );

		callback_info(callback_info&&) noexcept;
		callback_info& operator=(callback_info&&) noexcept;

		using fn_type = std::unique_ptr<callback_func_type>;

		fn_type fn;
		bool    repeat; //todo: timer
		bool    skip = false;
	};

	class callback
	{
	public:
		callback( );
		~callback( );

		callback(callback&&) noexcept;
		callback& operator=(callback&&) noexcept;

		bool active( ) const;
		void reset( );
		void operator()(const callback_data& data);

		void add(callback_info&& info);

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};

	struct two_way_callback
	{
		callback way_true, way_false;

		enum ways:uint8_t
		{
			WAY_TRUE
		  , WAY_FALSE
		};

		void add(callback_info&& info, ways way);
		void operator()(bool value, const callback_data& data);
	};
}
