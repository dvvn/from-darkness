#pragma once

#include "address.h"
//#include "signature.h"

namespace cheat::utl
{
	template <typename>
	constexpr bool is_optional_v = false;

	template <typename T>
	constexpr bool is_optional_v<std::optional<T>> = true;
	template <typename T>
	constexpr bool is_optional_v<const std::optional<T>&> = true;
	template <typename T>
	constexpr bool is_optional_v<std::optional<T>&> = true;
	template <typename T>
	constexpr bool is_optional_v<std::optional<T>&&> = true;

	using memory_block_container = std::span<uint8_t>;
	class memory_block final: memory_block_container
	{
	public:
		memory_block( ) = default;

		memory_block(const address& begin, size_t mem_size);
		memory_block(const address& begin, const address& end);
		memory_block(const address& addr);

		explicit memory_block(const memory_block_container& span);

		using known_bytes_range = std::span<uint8_t>;
		using unknown_bytes_range = std::span<std::optional<uint8_t>>;

	private:
		std::optional<memory_block> find_block_impl(const known_bytes_range& rng) const;
		std::optional<memory_block> find_block_impl(const unknown_bytes_range& rng) const;

	public:
		template <typename T>
		std::optional<memory_block> find_block(const T& block) const
		{
			if constexpr (!ranges::range<T>)
			{
				static_assert(!std::is_pointer_v<T> && std::is_trivially_destructible_v<T>, __FUNCSIG__": Unsupported block data type!");
				const auto rng = known_bytes_range((uint8_t*)std::addressof(block), sizeof(T));
				return this->find_block_impl(rng);
			}
			else
			{
				static_assert(ranges::random_access_range<T>, __FUNCSIG__": Unsupported range type!");

				auto first = std::_Get_unwrapped(block.begin( ));
				using raw_t = ranges::range_value_t<T>;

				if constexpr (is_optional_v<raw_t>)
				{
					static_assert(sizeof(typename raw_t::value_type) == sizeof(std::byte), __FUNCSIG__": Unsupported range element type!");
					const auto rng = unknown_bytes_range((std::optional<uint8_t>*)first, block.size( ));
					return this->find_block_impl(rng);
				}
				else
				{
					static_assert(sizeof(raw_t) == sizeof(std::byte), __FUNCSIG__": Unsupported range element type!");
					const auto rng = known_bytes_range((uint8_t*)first, block.size( ));
					return this->find_block_impl(rng);
				}
			}
		}

		template <typename T>
		std::vector<memory_block> find_all_blocks(const T& block) const
		{
			//todo: do it with ranges

			auto data = std::vector<memory_block>( );
			auto from = *this;
			for (;;)
			{
				std::optional<memory_block> found_block = from.find_block(block);
				if (!found_block.has_value( ))
					break;

				from = from.shift_to_end(*found_block);
				data.push_back(std::move(*found_block));
			}

			return data;
		}

		///use find_all_blocks directly
		//std::vector<memory_block> find_xrefs(const address& addr) const;

		using memory_block_container::size;
		using memory_block_container::empty;
		using memory_block_container::operator[];

		using memory_block_container::begin;
		using memory_block_container::end;

		using memory_block_container::front;
		using memory_block_container::back;

		using memory_block_container::_Unchecked_begin;
		using memory_block_container::_Unchecked_end;

		address addr( ) const;
		address last_addr( ) const;

		memory_block subblock(size_t offset, size_t count = std::dynamic_extent) const;
		memory_block shift_to(pointer ptr) const;
		memory_block shift_to_start(const memory_block& block) const;
		memory_block shift_to_end(const memory_block& block) const;

		struct flags_type final
		{
			using value_type = decltype(MEMORY_BASIC_INFORMATION::Protect);
			NSTD_ENUM_STRUCT_BITFLAG(flags_type);
		};

		bool have_flags(flags_type flags) const;
		bool dont_have_flags(flags_type flags) const;

		bool readable( ) const;
		bool readable_ex( ) const;
		bool writable( ) const;
		bool executable( ) const;
		bool code_padding( ) const;
	};
}
