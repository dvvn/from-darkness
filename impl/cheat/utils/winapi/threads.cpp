#include "threads.h"

#include "cheat/utils/bitflag.h"

using namespace cheat;
using namespace utl;
using namespace winapi;
using namespace winapi::detail;

void thread_entry::Open_assert_( ) const
{
	BOOST_ASSERT_MSG(is_opened( ), "Thread isnt open!");
}

thread_entry::thread_entry(THREADENTRY32&& entry): THREADENTRY32(move(entry))
{
	//-
}

bool thread_entry::open(thread_access access)
{
	close( );
	open_handle__ = decltype(open_handle__)(OpenThread(static_cast<std::underlying_type_t<thread_access>>(access), FALSE, this->th32ThreadID));
	return is_opened( );
}

bool thread_entry::close( )
{
	if (is_opened( ))
	{
		open_handle__.reset( );
		return true;
	}
	return false;
}

HANDLE thread_entry::transfer_handle( )
{
	Open_assert_( );
	return open_handle__.release( );
}

HANDLE thread_entry::handle( ) const
{
	return open_handle__.get( );
}

bool thread_entry::is_opened( ) const
{
	return open_handle__ != nullptr;
}

bool thread_entry::is_paused( ) const
{
	Open_assert_( );
	return WaitForSingleObject(open_handle__.get( ), 0) == WAIT_ABANDONED;
}

// ReSharper disable once CppInconsistentNaming
class THREADENTRY32_UPDATER: public THREADENTRY32
{
	template <std::invocable<LPVOID, THREADENTRY32*> Fn>
	decltype(auto) Update_(const handle& snapshot, Fn&& fn)
	{
		this->dwSize = sizeof(THREADENTRY32);
		return invoke(fn, snapshot.get( ), this);
	}

public:
	bool first(const handle& snapshot)
	{
		return Update_(snapshot, Thread32First);
	}

	bool next(const handle& snapshot)
	{
		return Update_(snapshot, Thread32Next);
	}
};

void winapi::enumerate_threads(threads_adder* push_fn, DWORD process_id)
{
	if (process_id == 0)
		process_id = GetCurrentProcessId( );

	const auto snapshot = handle(CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, process_id));
	if (snapshot == nullptr)
	{
		BOOST_ASSERT("Unable to create snapshot!");
		return;
	}

	auto updater = THREADENTRY32_UPDATER( );
	//auto threads = threads_storage( );

	constexpr auto min_size = FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(DWORD);
	const auto&    my_pid = process_id;
	const auto     my_thread = GetCurrentThreadId( );

	for (auto active = updater.first(snapshot); active != false; active = updater.next(snapshot))
	{
		if (updater.dwSize < min_size)
			continue;
		if (updater.th32OwnerProcessID != my_pid)
			continue;
		if (updater.th32ThreadID == my_thread)
			continue;

		auto copy = updater; //this is for future, we not move anything at real
		invoke(*push_fn, move(copy));
		//threads.push_back(boost::move(updater));
	}

	//threads.shrink_to_fit( );
	//return threads;
}

void unfreeze_thread::operator()(HANDLE h) const
{
	ResumeThread(h);
	CloseHandle(h);
}

frozen_thread::frozen_thread(HANDLE handle): frozen_thread_restorer(handle)
{
	SuspendThread(handle);
}

frozen_threads_storage::frozen_threads_storage(bool fill)
{
	if (fill)
		this->fill( );
}

void frozen_threads_storage::fill( )
{
	if (!this->empty( ))
		return;
	enumerate_threads(this);
}

void frozen_threads_storage::operator()(thread_entry&& t)
{
	using access = thread_access;

	if (constexpr auto flags = make_bitflag(access::suspend_resume, access::query_information, access::get_context, access::set_context).get( );
		!t.open(flags) || t.is_paused( ))
		return;

	this->push_back(t.transfer_handle( ));
}
