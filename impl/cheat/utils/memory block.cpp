#include "memory block.h"

using namespace cheat;
using namespace utl;

memory_block::memory_block(const address& begin, size_t mem_size) : memory_block_container(begin.ptr<uint8_t>( ), mem_size)
{
}

memory_block::memory_block(const address& begin, const address& end) : memory_block_container(begin.ptr<uint8_t>( ), end.ptr<uint8_t>( ))
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

using flags_type = memory_block::flags_type;

// ReSharper disable once CppInconsistentNaming
class MEMORY_BASIC_INFORMATION_UPDATER: protected MEMORY_BASIC_INFORMATION
{
	static constexpr SIZE_T class_size = sizeof(MEMORY_BASIC_INFORMATION);

	template <typename Fn, typename ...Args>
	bool virtual_query(Fn&& native_function, Args&&...args)
	{
		return class_size == std::invoke((native_function), std::forward<Args>(args)..., static_cast<PMEMORY_BASIC_INFORMATION>(this), class_size);
	}

	//protected:
	//auto& get_flags( ) { return reinterpret_cast<flags_type&>(Protect); }
public:
	MEMORY_BASIC_INFORMATION_UPDATER( ): MEMORY_BASIC_INFORMATION( )
	{
	}

	auto& get_flags( ) const
	{
		return reinterpret_cast<const flags_type&>(Protect);
	}

	SIZE_T region_size( ) const
	{
		return this->RegionSize;
	}

	bool update(LPCVOID address)
	{
		return virtual_query(VirtualQuery, address);
	}

	bool update(HANDLE process, LPCVOID address)
	{
		return virtual_query(VirtualQueryEx, process, address);
	}
};

template <std::default_initializable Fn>
	requires(std::is_invocable_r_v<bool, Fn, flags_type, flags_type>)
class flags_checker: public MEMORY_BASIC_INFORMATION_UPDATER
{
	flags_checker(flags_type flags): MEMORY_BASIC_INFORMATION_UPDATER( ),
									 flags_checked_(flags)
	{
	}

public:
	flags_checker(Fn&& checker_fn, flags_type flags) : flags_checker(flags)
	{
		checker_fn_ = std::move(checker_fn);
	}

	flags_checker(const Fn& checker_fn, flags_type flags) : flags_checker(flags)
	{
		checker_fn_ = (checker_fn);
	}

private:
	Fn         checker_fn_;
	flags_type flags_checked_;

public:
	std::optional<bool> check_flags(SIZE_T block_size) const
	{
		//memory isnt commit!
		if (this->State != MEM_COMMIT)
			return false;

		//flags check isnt passed!
		if (std::invoke(checker_fn_, this->get_flags( ), flags_checked_) == false)
			return false;

		//found good result
		if (this->RegionSize >= block_size)
			return true;

		//check next block
		return { };
	}
};

template <typename Fn>
flags_checker(Fn&&) -> flags_checker<std::remove_cvref_t<Fn>>;

static constexpr auto _Page_read_flags    = flags_type(PAGE_READONLY, PAGE_READWRITE, PAGE_EXECUTE_READ, PAGE_EXECUTE_READWRITE);
static constexpr auto _Page_write_flags   = flags_type(PAGE_READWRITE, PAGE_WRITECOPY, PAGE_EXECUTE_READWRITE, PAGE_EXECUTE_WRITECOPY, PAGE_WRITECOMBINE);
static constexpr auto _Page_execute_flags = flags_type(PAGE_EXECUTE, PAGE_EXECUTE_READ, PAGE_EXECUTE_READWRITE, PAGE_EXECUTE_WRITECOPY);

template <typename Fn>
static bool _Memory_block_flags_checker(flags_type flags, memory_block block, Fn&& checker_fn = { })
{
	flags_checker<Fn> checker(std::forward<Fn>(checker_fn), flags);
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

struct have_flags_fn
{
	bool operator()(flags_type region_flags, flags_type target_flags) const
	{
		return region_flags.has(target_flags) == true;
	}
};
struct dont_have_flags_fn
{
	bool operator()(flags_type region_flags, flags_type target_flags) const
	{
		return region_flags.has(target_flags) == false;
	}
};

#pragma endregion

bool memory_block::have_flags(flags_type flags) const
{
	return _Memory_block_flags_checker<have_flags_fn>(flags, *this);
}

bool memory_block::dont_have_flags(flags_type flags) const
{
	return _Memory_block_flags_checker<dont_have_flags_fn>(flags, *this);
}

bool memory_block::readable( ) const
{
	return this->dont_have_flags(PAGE_NOACCESS);
}

bool memory_block::readable_ex( ) const
{
	return this->have_flags(_Page_read_flags);
}

bool memory_block::writable( ) const
{
	return this->have_flags(_Page_write_flags);
}

bool memory_block::executable( ) const
{
	return this->have_flags(_Page_execute_flags);
}

bool memory_block::code_padding( ) const
{
	const auto first = this->front( );
	if (first != 0x00 && first != 0x90 && first != 0xCC)
		return false;

	for (const auto val: this->subblock(1))
	{
		if (val != first)
			return false;
	}
	return true;
}
