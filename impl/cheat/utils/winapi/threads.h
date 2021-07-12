#pragma once

#include "handle.h"

namespace cheat::utl::winapi
{
	enum class thread_access : DWORD
	{
		terminate = THREAD_TERMINATE,
		suspend_resume = THREAD_SUSPEND_RESUME,
		get_context = THREAD_GET_CONTEXT,
		set_context = THREAD_SET_CONTEXT,
		query_information = THREAD_QUERY_INFORMATION,
		set_information = THREAD_SET_INFORMATION,
		set_thread_token = THREAD_SET_THREAD_TOKEN,
		impersonate = THREAD_IMPERSONATE,
		direct_impersonation = THREAD_DIRECT_IMPERSONATION,
		set_limited_information = THREAD_SET_LIMITED_INFORMATION,
		query_limited_information = THREAD_QUERY_LIMITED_INFORMATION,
		resume = THREAD_RESUME,
		all_access = THREAD_ALL_ACCESS,
	};

	class thread_entry: public THREADENTRY32
	{
		void Open_assert_( ) const;

	public:
		thread_entry(THREADENTRY32&& entry);

		bool open(thread_access access);
		bool close( );

		[[nodiscard]] HANDLE transfer_handle( );
		HANDLE               handle( ) const;
		
		bool is_opened( ) const;
		bool is_paused( ) const;

		

	private:
		winapi::handle open_handle__ = nullptr;
	};

	//using threads_storage = std::vector<thread_entry>;
	//using threads_adder = std::function<void (thread_entry&&)>;
	class threads_adder
	{
	public:
		virtual      ~threads_adder( ) = default;
		virtual void operator()(thread_entry&& entry) =0;
	};

	void enumerate_threads(threads_adder* push_fn, DWORD process_id = 0);

	//--

	namespace detail
	{
		class unfreeze_thread
		{
		public:
			using pointer = HANDLE;

			void operator()(HANDLE h) const;
		};

		using frozen_thread_restorer = unique_ptr<HANDLE, unfreeze_thread>;
		class frozen_thread: public frozen_thread_restorer
		{
		public:
			frozen_thread(HANDLE handle);
		};
	}

	using frozen_threads_storage_container = vector<detail::frozen_thread>;
	class frozen_threads_storage :protected frozen_threads_storage_container, public threads_adder
	{
	BOOST_MOVABLE_BUT_NOT_COPYABLE(frozen_threads_storage)
	public:
		frozen_threads_storage(frozen_threads_storage&&) noexcept = default;
		frozen_threads_storage& operator =(frozen_threads_storage&&) noexcept= default;

		frozen_threads_storage(bool fill=0);

		void fill( );

		using frozen_threads_storage_container::clear;

	protected:
		void operator()(thread_entry&& t) override;

		//using frozen_threads_storage_container::empty;
		//using frozen_threads_storage_container::size;
	};
}
