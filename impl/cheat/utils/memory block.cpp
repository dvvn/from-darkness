#include "memory block.h"

using namespace cheat;
using namespace utl;

memory_block::memory_block(const address& begin, size_t mem_size) : memory_block_container(begin.raw<uint8_t>( ), mem_size)
{
}

memory_block::memory_block(const address& begin, const address& end) : memory_block_container(begin.raw<uint8_t>( ), end.raw<uint8_t>( ))
{
}

memory_block::memory_block(const address& addr) : memory_block(addr, sizeof(address))
{
}

memory_block::memory_block(const memory_block_container& span) : memory_block_container(span)
{
}

address memory_block::addr( ) const
{
	return data( );
}

address memory_block::last_addr( ) const
{
	return _Unchecked_end( );
}

memory_block memory_block::subblock(size_t offset, size_t count) const
{
	return this->subspan(offset, count);
}

memory_block memory_block::shift_to(pointer ptr) const
{
	return this->subspan(std::distance(data( ), ptr));
}

memory_block memory_block::shift_to_start(const memory_block& block) const
{
	return this->shift_to(block._Unchecked_begin( ));
}

memory_block memory_block::shift_to_end(const memory_block& block) const
{
	return this->shift_to(block._Unchecked_end( ));
}

#pragma region flags_check

struct mem_flags_t
{
	using value_type = decltype(MEMORY_BASIC_INFORMATION::Protect);
	CHEAT_ENUM_STRUCT_FILL_BITFLAG(mem_flags_t)
};

// ReSharper disable once CppInconsistentNaming
class MEMORY_BASIC_INFORMATION_UPDATER: protected MEMORY_BASIC_INFORMATION
{
	static constexpr SIZE_T class_size = sizeof(MEMORY_BASIC_INFORMATION);

	template <typename Fn, typename ...Args>
	bool Virtual_query_(Fn&& native_function, Args&&...args)
	{
		return class_size == invoke((native_function), forward<Args>(args)..., static_cast<PMEMORY_BASIC_INFORMATION>(this), class_size);
	}

	//protected:
	//auto& get_flags( ) { return reinterpret_cast<mem_flags_t&>(Protect); }
public:
	auto& get_flags( ) const
	{
		return reinterpret_cast<const mem_flags_t&>(Protect);
	}

	SIZE_T region_size( ) const
	{
		return this->RegionSize;
	}

	bool update(LPCVOID address)
	{
		return Virtual_query_(VirtualQuery, address);
	}

	bool update(HANDLE process, LPCVOID address)
	{
		return Virtual_query_(VirtualQueryEx, process, address);
	}
};

template <typename Fn>
	requires(std::is_invocable_r_v<bool, Fn, mem_flags_t, mem_flags_t>)
class flags_checker: public MEMORY_BASIC_INFORMATION_UPDATER
{
public:
	flags_checker(mem_flags_t::value_type_raw flags) : MEMORY_BASIC_INFORMATION_UPDATER( ),
													   flags_checked__(flags)
	{
	}

private:
	static constexpr Fn check_fn;
	mem_flags_t flags_checked__;

public:
	optional<bool> check_flags(SIZE_T block_size) const
	{
		//memory isnt commit!
		if (this->State != MEM_COMMIT)
			return false;

		//flags check isnt passed!
		if (invoke(check_fn, this->get_flags( ), flags_checked__) == false)
			return false;

		//found good result
		if (this->RegionSize >= block_size)
			return true;

		//check next block
		return { };
	}
};

struct have_flags_fn
{
	bool operator()(const mem_flags_t& region_flags, const mem_flags_t& target_flags) const
	{
		return region_flags.has(target_flags) == true;
	}
};

struct dont_have_flags_fn
{
	bool operator()(const mem_flags_t& region_flags, const mem_flags_t& target_flags) const
	{
		return region_flags.has(target_flags) == false;
	}
};

static constexpr auto PAGE_READ_FLAGS = mem_flags_t(PAGE_READONLY, PAGE_READWRITE, PAGE_EXECUTE_READ, PAGE_EXECUTE_READWRITE).value_raw( );
static constexpr auto PAGE_WRITE_FLAGS = mem_flags_t(PAGE_READWRITE, PAGE_WRITECOPY, PAGE_EXECUTE_READWRITE, PAGE_EXECUTE_WRITECOPY, PAGE_WRITECOMBINE).value_raw( );
static constexpr auto PAGE_EXECUTE_FLAGS = mem_flags_t(PAGE_EXECUTE, PAGE_EXECUTE_READ, PAGE_EXECUTE_READWRITE, PAGE_EXECUTE_WRITECOPY).value_raw( );

template <typename Fn>
static bool _Memory_block_flags_checker(mem_flags_t::value_type_raw flags, memory_block block)
{
	flags_checker<Fn> checker(flags);
	while (true)
	{
		if (!checker.update(block._Unchecked_begin( )))
			return false;

		auto result = checker.check_flags(block.size( ));
		if (result.has_value( ))
			return *result;

		block = block.subblock(checker.region_size( ));
	}
}

#pragma endregion

bool memory_block::have_flags(DWORD flags) const
{
	return _Memory_block_flags_checker<have_flags_fn>(flags, *this);
}

bool memory_block::dont_have_flags(DWORD flags) const
{
	return _Memory_block_flags_checker<dont_have_flags_fn>(flags, *this);
}

bool memory_block::readable( ) const
{
	return this->dont_have_flags(PAGE_NOACCESS);
}

bool memory_block::readable_ex( ) const
{
	return this->have_flags(PAGE_READ_FLAGS);
}

bool memory_block::writable( ) const
{
	return this->have_flags(PAGE_WRITE_FLAGS);
}

bool memory_block::executable( ) const
{
	return this->have_flags(PAGE_EXECUTE_FLAGS);
}
