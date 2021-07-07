#pragma once

namespace cheat::utl::mem
{
	template <std::copyable T>
	class memory_backup
	{
	public:
		memory_backup(const memory_backup&) = delete;
		memory_backup& operator=(const memory_backup&)= delete;

		memory_backup(memory_backup&& other) noexcept
		{
			*this = move(other);
		}

		memory_backup& operator=(memory_backup&& other) noexcept
		{
			//owner__.swap(other.owner__);
			//value__.swap(other.value__);
			owner__ = move(other.owner__);
			value__ = move(other.value__);
			return *this;
		}

		memory_backup( ) = default;

		memory_backup(T& from) : owner__(from)
		{
			value__.emplace(from);
		}

		template <typename T2>
		memory_backup(T& from, T2&& owerride)
			requires(std::is_constructible_v<T, decltype(owerride)>) : memory_backup(from)
		{
			from = T(forward<T2>(owerride));
		}

		~memory_backup( )
		{
			if (value__.has_value( ))
			{
				*owner__ = move(*value__);
			}
		}

		void reset( )
		{
			value__.reset( );
		}

		_NODISCARD T release( )
		{
			T ret = move(*value__);
			this->reset( );
			return ret;
		}

		/**
		 * \brief added to prevent create varianle-holder. create backup and call this function from fucntion paramter
		 */
		template <typename T1>
		_NODISCARD T1 val(T1&& val)
		{
			(void)this;
			return val;
		}

		bool operator!( ) const
		{
			return !value__.has_value();
		}

	private:
		optional<T&> owner__;
		optional<T>  value__;
	};

	//#define MEM_BACKUP(memory,...)\
	//   [[maybe_unused]] const _CONCAT(mem_backup_, __LINE__)  = utl::mem::memory_backup(memory,##__VA_ARGS__); (const void)_CONCAT(mem_backup_, __LINE__)
}
