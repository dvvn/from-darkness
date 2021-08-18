#pragma once

namespace cheat::utl
{
	class memory_protect
	{
	public:
		struct value_type
		{
			LPVOID addr;
			SIZE_T size;
			DWORD  flags;
		};

		memory_protect(const memory_protect&)            = delete;
		memory_protect& operator=(const memory_protect&) = delete;

		memory_protect(memory_protect&& other) noexcept
		{
			*this = std::move(other);
		}

		memory_protect& operator=(memory_protect&& other) noexcept
		{
			std::swap(this->info_, other.info_);
			return *this;
		}

		memory_protect( ) = default;

		memory_protect(const LPVOID addr, SIZE_T size, DWORD new_flags)
		{
			DWORD old_flags;
			if (!VirtualProtect(addr, size, new_flags, std::addressof(old_flags)))
				throw std::runtime_error("Unable to protect memory!");

			info_.emplace(addr, size, old_flags);
		}

		memory_protect(address addr, SIZE_T size, DWORD new_flags): memory_protect(addr.ptr<void>(), size, new_flags)
		{
		}

		memory_protect(const memory_block& mem, DWORD new_flags): memory_protect(mem.addr( ), mem.size( ), new_flags)
		{
		}

		~memory_protect( )
		{
			this->restore_impl<true>( );
		}

	private:
		template <bool FromDestructor>
		auto restore_impl( )
		{
			if (info_.has_value( ))
			{
				auto& [addr,size,flags] = *info_;

				if (DWORD unused; VirtualProtect(addr, size, flags, std::addressof(unused)))
				{
					if constexpr (!FromDestructor)
					{
						info_.reset( );
						return true;
					}
				}
			}
			if constexpr (!FromDestructor)
				return false;
		}

	public:
		bool restore( )
		{
			return this->restore_impl<false>( );
		}

		bool has_value( ) const
		{
			return info_.has_value( );
		}

	private:
		std::optional<value_type> info_;
	};
}
