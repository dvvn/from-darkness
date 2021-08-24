#pragma once

namespace nstd
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

		memory_protect(memory_protect&& other) noexcept;
		memory_protect& operator=(memory_protect&& other) noexcept;

		memory_protect( )= default;

		memory_protect(LPVOID addr, SIZE_T size, DWORD new_flags);
		memory_protect(address addr, SIZE_T size, DWORD new_flags);
		memory_protect(const memory_block& mem, DWORD new_flags);

		~memory_protect( );

		bool restore( );
		bool has_value( ) const;

	private:
		std::optional<value_type> info_;
	};
}
