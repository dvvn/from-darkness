#include "threads.h"

using namespace cheat;
using namespace utl;
using namespace winapi;
using namespace winapi::detail;

void thread_entry::Open_assert_( ) const
{
	runtime_assert(is_opened( ), "Thread isnt open!");
}

thread_entry::thread_entry(THREADENTRY32&& entry): THREADENTRY32(std::move(entry))
{
	//-
}

bool thread_entry::open(thread_access access)
{
	close( );
	open_handle__ = decltype(open_handle__)(OpenThread(access.value_raw( ), FALSE, this->th32ThreadID));
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
		return std::invoke(fn, snapshot.get( ), this);
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

void winapi::_Enumerate_threads(threads_adder* push_fn, DWORD process_id)
{
	if (process_id == 0)
		process_id = GetCurrentProcessId( );

	const auto snapshot = handle(CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, process_id));
	if (snapshot == nullptr)
	{
		runtime_assert("Unable to create snapshot!");
		return;
	}

	auto updater = THREADENTRY32_UPDATER( );
	//auto threads = threads_storage( );

	constexpr auto min_size = FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(decltype(THREADENTRY32::th32OwnerProcessID));
	const auto& my_pid = process_id;
	const auto my_thread = GetCurrentThreadId( );

	for (auto active = updater.first(snapshot); active != false; active = updater.next(snapshot))
	{
		if (updater.dwSize < min_size)
			continue;
		if (updater.th32OwnerProcessID != my_pid)
			continue;
		if (updater.th32ThreadID == my_thread)
			continue;

		auto copy = updater; //this is for future, we not move anything at real
		std::invoke(*push_fn, std::move(copy));
		//threads.push_back(boost::move(updater));
	}

	//threads.shrink_to_fit( );
	//return threads;
}

frozen_threads_storage::frozen_threads_storage(frozen_threads_storage&& other) noexcept
{
	*this = move(other);
}

frozen_threads_storage& frozen_threads_storage::operator=(frozen_threads_storage&& other) noexcept
{
	*static_cast<frozen_threads_storage_container*>(this) = static_cast<frozen_threads_storage_container&&>(other);
	return *this;
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
	_Enumerate_threads(this);
}

void frozen_threads_storage::operator()(thread_entry&& t)
{
	using f = thread_access;

	if (constexpr auto flags = thread_access(f::suspend_resume, f::query_information, f::get_context, f::set_context);
		!t.open(flags) || t.is_paused( ))
	{
		return;
	}

	this->push_back(t.transfer_handle( ));
}
