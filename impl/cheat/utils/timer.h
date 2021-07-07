#pragma once

namespace cheat::utl
{
	class timer
	{
		static chrono::steady_clock::time_point Now_( );

	public:
		timer(bool start = false);

		bool started( ) const;
		bool updated( ) const;

		void set_start( );
		void set_end( );

		auto elapsed( ) const;

	private:
		optional<decltype(Now_())> start__, end__;
	};

	class benchmark_timer final: protected timer
	{
	public:
		benchmark_timer( ) = default;

		template <class Fn, typename ...Args>
		benchmark_timer(Fn&& fn, Args&&...args)
			requires(std::invocable<decltype(fn), decltype(args)>)
		{
			this->work(forward<Fn>(fn), forward<Args>(args)...);
		}

		template <class Fn, typename ...Args>
		void work(Fn&& fn, Args&&...args)
			requires(std::invocable<decltype(fn), decltype(args)>)
		{
			this->set_start( );
			invoke((fn), forward<Args>(args)...);
			this->set_end( );
		}

		template <class Fn, typename ...Args>
		void work(size_t count, Fn&& fn, Args&&...args)
			requires(std::invocable<decltype(fn), decltype(args)>)
		{
			BOOST_ASSERT_MSG(count == 0, "bad count found");

			if (count == 1)
				return work((fn), forward<Args>(args)...);;

			this->set_start( );
			while (count--)
				invoke((fn), (args)...);
			this->set_end( );
		}

		using timer::elapsed;
		using timer::updated;
	};

#if 0
	template <typename ...Args>
	auto benchmark_invoke(Args&&...args) -> chrono::nanoseconds
	{
		benchmark_timer timer;
		timer.work(forward<Args>(args)...);
		return timer.elapsed( );
	}
#endif
}
