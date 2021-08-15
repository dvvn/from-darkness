#pragma once
#include "handle.h"

namespace cheat::utl::winapi
{
	struct thread_access
	{
		enum value_type : DWORD
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

		CHEAT_ENUM_STRUCT_FILL_BITFLAG(thread_access)
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

	void _Enumerate_threads(threads_adder* push_fn, DWORD process_id = 0);

	//--

	namespace detail
	{
		class unfreeze_thread
		{
		public:
			using pointer = HANDLE;

			void operator()(HANDLE h) const;
		};

		using frozen_thread_restorer = std::unique_ptr<HANDLE, unfreeze_thread>;
		class frozen_thread: public frozen_thread_restorer
		{
		public:
			frozen_thread(HANDLE handle);
		};
	}

	using frozen_threads_storage_container = std::vector<detail::frozen_thread>;
	class frozen_threads_storage: protected frozen_threads_storage_container, public threads_adder
	{
	public:
		frozen_threads_storage(frozen_threads_storage&& other) noexcept;
		frozen_threads_storage& operator =(frozen_threads_storage&& other) noexcept;

		frozen_threads_storage(const frozen_threads_storage& other)             = delete;
		frozen_threads_storage& operator =(const frozen_threads_storage& other) = delete;

		frozen_threads_storage(bool fill);

		void fill( );

		using frozen_threads_storage_container::clear;

	protected:
		void operator()(thread_entry&& t) override;

		//using frozen_threads_storage_container::empty;
		//using frozen_threads_storage_container::size;
	};
}
