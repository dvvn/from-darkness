#pragma once

#include "address.h"
//#include "signature.h"

namespace cheat::utl
{
#if 0

	namespace detail
	{
		template <typename T>
		constexpr auto memory_sequence_impl() -> bool
		{
			if constexpr (ranges::range<T>)
				return ranges::random_access_iterator<ranges::iterator_t<T>> && !std::is_class_v<ranges::range_value_t<T>>;
			else
				return /*utl::simple_type*/std::is_trivially_destructible_v<T>;
		}

		template <typename T>
		//allow stuff like vector<int> but not vector<class> or map<>
		concept memory_sequence = memory_sequence_impl<T>();
		template <typename T>
		concept memory_divided = !memory_sequence_impl<T>();

		template <memory_sequence Seq, size_t S = sizeof(Seq)>
		requires(sizeof(Seq) <= S)
			class memory_blocks_comparer
		{
			static auto Compare_bytes(const void* a, const void* b, size_t array_elements_count) -> bool
			{
				return std::memcmp(a, b, S * array_elements_count) == 0;
			}

			template <typename T2>
			static auto Compare(const T2* a, const T2* b, size_t array_elements_count) -> bool
			{
				if constexpr (!ranges::same_as<Seq, T2>)
					return Compare((const Seq*)a, (const Seq*)b, array_elements_count);
				else if constexpr (!ranges::equality_comparable<Seq> || sizeof(Seq) != S)
					return Compare_bytes(a, b, array_elements_count);
				else
				{
					if (array_elements_count != 1)
						return Compare_bytes(a, b, array_elements_count);

					return *a == *b;
				}
			}

		public:
			template <typename T2>
			auto operator()(const T2* a, const T2* b, size_t array_elements_count) const -> bool
			{
				return Compare(a, b, array_elements_count);
			}
		};

		template <typename T, size_t S>
		constexpr auto select_memory_blocks_comparer()
			requires(!ranges::range<T>)
		{
			if constexpr (ranges::equality_comparable<T>)
			{
				if constexpr (sizeof(T) == S)
					return memory_blocks_comparer<T>();
				else
					return select_memory_blocks_comparer<void, S>();
			}
			else
			{
				if constexpr (S == sizeof(uint8_t))
					return memory_blocks_comparer<uint8_t>();
				else if constexpr (S == sizeof(uint16_t))
					return memory_blocks_comparer<uint16_t>();
				else if constexpr (S == sizeof(uint32_t))
					return memory_blocks_comparer<uint32_t>();
				else if constexpr (S == sizeof(uint64_t))
					return memory_blocks_comparer<uint64_t>();
				else
					return memory_blocks_comparer<uint8_t, S>();
			}
		}

		template <typename T>
		constexpr auto get_memory_block_raw_element_type()
		{
			if constexpr (memory_divided<T>)
				return std::type_identity<typename ranges::range_value_t<T>::value_type>();
			else if constexpr (ranges::/*random_access_*/range<T>) //we already know this is random_access data
				return std::type_identity<ranges::range_value_t<T>>();
			else
				return std::type_identity<T>();
		}

		template <memory_sequence Seq>
		auto find_raw_memory_block_start(const Seq& block)
		{
			if constexpr (!ranges::/*random_access_*/range<Seq>)
				return boost::addressof(block);
			else
			{
				using block_ptr_t = const ranges::range_value_t<Seq>*;
				return static_cast<block_ptr_t>(block.data());
			}
		}
	}

#endif

	namespace detail
	{
		template <typename T>
		concept have_has_value = requires(const T& checked)
		{
			{ checked.has_value( ) }->std::same_as<bool>;
		};
	}

	using memory_block_container = span<uint8_t>;
	class memory_block final: memory_block_container
	{
	public:
		memory_block( ) = default;

		memory_block(const address& begin, size_t mem_size);
		memory_block(const address& begin, const address& end);
		memory_block(const address& addr);

		static constexpr auto empty_block = memory_block_container(static_cast<uint8_t*>(nullptr), 0);

	private:
		memory_block(const memory_block_container& span);

#if 0

		template <typename T>
		class Compare_helper
		{
		public:
			using element_type = typename decltype(detail::get_memory_block_raw_element_type<T>())::type;

			static constexpr uintptr_t element_size_bytes = sizeof(element_type);
			static constexpr auto comparer_fn = detail::select_memory_blocks_comparer<element_type, element_size_bytes>();

			Compare_helper(const memory_block& block_from, const T& tested_block)
			{
				bytes_avaiable = block_from.size();
				start = block_from.addr().value();
				real_end = start + bytes_avaiable;

				if constexpr (ranges::/*random_access_*/range<T>)
					array_elements_count = tested_block.size();
				else
					array_elements_count = 1;

				block_size_bytes = element_size_bytes * array_elements_count;
				BOOST_ASSERT_MSG(real_end - start >= block_size_bytes, "Wrong block size");
				end = real_end - block_size_bytes;
				BOOST_ASSERT_MSG(start <= end, "Incorrect limit or step!");

#ifdef _DEBUG
				element_size_bytes_dbg = element_size_bytes;
				comparer_fn_dbg = comparer_fn;
#endif
			}

			uintptr_t start, end;
			uintptr_t real_end;
			uintptr_t bytes_avaiable;
			uintptr_t array_elements_count;
			uintptr_t block_size_bytes;

#ifdef _DEBUG
			//for better debug view
			// ReSharper disable CppInconsistentNaming
		private:
			std::type_identity<element_type> element_type_dbg;
			uintptr_t element_size_bytes_dbg;
			std::remove_const_t<decltype(comparer_fn)> comparer_fn_dbg;

		public:
			// ReSharper restore CppInconsistentNaming
#endif

			template <detail::memory_sequence T2>
			auto operator()(const T2* a, const T2* b) const -> bool
			{
				if constexpr (detail::memory_divided<T>)
					return comparer_fn(a, b, 1u);
				else
					return comparer_fn(a, b, array_elements_count);
			}

			template <bool InUse>
			[[deprecated("Works very slow. Use function below.")]]
			auto is_pointer_readable(const void* test) const -> bool
			{
				if constexpr (!InUse)
					return true;
				else
				{
					if (!test)
						return false;
					if (!memory_block(test, this->element_size_bytes).readable())
						return false;
					return true;
				}
			}

			template <bool InUse>
			auto is_data_readable() const -> bool
			{
				if constexpr (!InUse)
					return true;
				else
				{
					return memory_block(this->start, this->bytes_avaiable).readable();
				}
			}
		};

		/*template <bool _In_use, detail::_Memory_sequence T, typename _Fn>
		static bool _Is_pointer_readable(const T* test, const _Fn& comparer)
		{
			if constexpr (!_In_use)
				return true;
			else
			{
				if (!test)
					return false;
				if (!memory_block(test, comparer.element_size_bytes).readable( ))
					return false;
				return true;
			}
		}*/

		template <bool CheckReadable, detail::memory_sequence Seq>
		auto Check_block_raw_(const Seq& block, size_t step = 1) const -> memory_block
		{
			const auto comparer = Compare_helper(*this, block);

			if (!comparer.template is_data_readable<CheckReadable>())
				return empty_block;

			auto block_start = detail::find_raw_memory_block_start(block);
			for (auto addr = comparer.start; addr >= comparer.start && addr <= comparer.end; addr += step)
			{
				auto test = (decltype(block_start))(addr);

				/*if (!comparer.template _Is_pointer_readable<_Check_readable>(test))
					continue;*/
				if (!comparer(block_start, test))
					continue;

				return memory_block(test, comparer.block_size_bytes);
			}

			return empty_block;
		}

		template <bool CheckReadable, detail::memory_sequence Seq>
		auto Check_block_wrapped_(const span<std::optional<Seq>>& block, size_t step = 1) const -> memory_block
		{
			const auto comparer = Compare_helper(*this, block);
			static_assert(comparer.element_size_bytes == sizeof(Seq), "Comparer fucked up");

			if (!comparer.template is_data_readable<CheckReadable>())
				return empty_block;

			for (auto addr = comparer.start; addr >= comparer.start && addr <= comparer.end; addr += step)
			{
				auto addr_internal = addr;
				for (const std::optional<Seq>& val : block)
				{
					if (val.has_value())
					{
						const Seq* test = reinterpret_cast<const Seq*>(addr_internal);

						/*if (!comparer.template _Is_pointer_readable<_Check_readable>(test))
							goto _NOT_FOUND;*/

						const Seq* block_start = detail::find_raw_memory_block_start(*val);
						if (!comparer(block_start, test))
							goto _NOT_FOUND;
					}
					addr_internal += comparer.element_size_bytes;
				}

			_FOUND:
				return memory_block(address(addr), comparer.block_size_bytes);

			_NOT_FOUND:
				(void)0;
			}

			return empty_block;
		}

#endif

		template <size_t Min_size, typename T>
		static constexpr bool Converting_possible_(size_t range_size_bytes)
		{
			if constexpr (constexpr auto type_convert_to_size = sizeof(T); Min_size > type_convert_to_size)
				return false;
			else if constexpr (Min_size == type_convert_to_size)
				return true;
			else
				return (range_size_bytes % type_convert_to_size) == 0;
		}

		template <typename T>
		memory_block Check_block_raw_impl_(const span<T>& rng_checked) const
		{
			const auto this_size_bytes = this->size_bytes( );
			const auto rng_size_bytes = rng_checked.size_bytes( );

			const auto start = this->addr( ).value( );
			const auto real_end = start + this_size_bytes;
			const auto end = real_end - rng_size_bytes /*- sizeof(T)*/;

			BOOST_ASSERT_MSG(real_end - start >= rng_size_bytes, "Wrong block size");
			BOOST_ASSERT_MSG(start <= end, "Incorrect limit or step!");

			for (auto addr = start; addr <= end; ++addr)
			{
				auto addr_begin = reinterpret_cast<T*>(addr);

				auto rng_begin = rng_checked.data( );
				auto rng_end = rng_begin + rng_checked.size( );

				do
				{
					if (*rng_begin != *addr_begin)
						goto NOT_FOUND;

					++rng_begin;
					if (rng_begin == rng_end)
						goto FOUND;
					++addr_begin;
				}
				while (true);

			FOUND:
				return memory_block((addr), rng_size_bytes);
			NOT_FOUND:
				(void)0;
			}

			BOOST_ASSERT("Raw memory block not found!");
			return empty_block;
		}

		template <typename To, typename T>
		static span<To> Rewrap_range_(T* rng_unchecked_begin, size_t rng_size_bytes)
		{
			BOOST_ASSERT_MSG(rng_size_bytes % sizeof(To) == 0, "Unable to rewrap range! Wrong data type");
			auto begin = reinterpret_cast<To*>(rng_unchecked_begin);
			auto end = begin + rng_size_bytes / sizeof(To);
			return span<To>(begin, end);
		}

		template <bool InUse>
		bool Not_readable_assert_( ) const
		{
			if constexpr (!InUse)
				return true;
			else
			{
				if (this->readable( ))
					return true;

				BOOST_ASSERT("This memory block isn't readable!");
				return false;
			}
		}

		template <bool CheckReadable, ranges::random_access_range Rng>
		memory_block Check_block_raw_(const Rng& rng_checked) const
		{
			using rng_val = ranges::range_value_t<Rng>;
			BOOST_STATIC_ASSERT_MSG(std::is_integral_v<rng_val>, "only integral values supported");

			if (!Not_readable_assert_<CheckReadable>( ))
				return empty_block;

			constexpr auto rng_val_bytes = sizeof(rng_val);
			const size_t rng_size_bytes = rng_checked.size( ) * rng_val_bytes;

			rng_val* rng_begin = const_cast<rng_val*>(rng_checked.data( ));

			if (Converting_possible_<rng_val_bytes, uint64_t>(rng_size_bytes))
				return Check_block_raw_impl_(Rewrap_range_<uint64_t>(rng_begin, rng_size_bytes));
			if (Converting_possible_<rng_val_bytes, uint32_t>(rng_size_bytes))
				return Check_block_raw_impl_(Rewrap_range_<uint32_t>(rng_begin, rng_size_bytes));
			if (Converting_possible_<rng_val_bytes, uint16_t>(rng_size_bytes))
				return Check_block_raw_impl_(Rewrap_range_<uint16_t>(rng_begin, rng_size_bytes));
			if (Converting_possible_<rng_val_bytes, uint8_t>(rng_size_bytes))
				return Check_block_raw_impl_(Rewrap_range_<uint8_t>(rng_begin, rng_size_bytes));

			BOOST_ASSERT("Unable to check raw memory block!");
			return empty_block;
		}

		template <typename Rng>
		void Assert_bad_wrapped_mode_(const Rng& rng) const
		{
#ifndef BOOST_ASSERT_IS_VOID
			size_t known = 0;
			size_t unknown = 0;
			for (auto& opt: rng)
			{
				if (opt.has_value( ))
					++known;
				else
					++unknown;
			}

			BOOST_ASSERT_MSG(known!=rng.size(), "Check_block_wrapped_: all bytes are known!");
			BOOST_ASSERT_MSG(unknown!=rng.size(), "Check_block_wrapped_: all bytes are unknown!");
#endif
		}

		template <bool CheckReadable, ranges::random_access_range Rng>
		memory_block Check_block_wrapped_(const Rng& rng_checked) const
		{
			using rng_val = ranges::range_value_t<Rng>;
			BOOST_STATIC_ASSERT_MSG(std::is_class_v<rng_val>, "only classes supported!");
			BOOST_STATIC_ASSERT_MSG(detail::have_has_value<rng_val>, "only optional-like classes supported!");
			using opt_val = typename rng_val::value_type;
			BOOST_STATIC_ASSERT_MSG(std::is_integral_v<opt_val>, "unsupported value_type!");

			Assert_bad_wrapped_mode_(rng_checked);
			if (!Not_readable_assert_<CheckReadable>( ))
				return empty_block;

			constexpr auto opt_val_bytes = sizeof(opt_val);
			const size_t rng_size_bytes = rng_checked.size( ) * opt_val_bytes;
			const auto this_size_bytes = this->size_bytes( );

			const auto start = this->addr( ).value( );
			const auto real_end = start + this_size_bytes;
			const auto end = real_end - rng_size_bytes /*- sizeof(opt_val)*/;

			BOOST_ASSERT_MSG(real_end - start >= rng_size_bytes, "Wrong block size (2)");
			BOOST_ASSERT_MSG(start <= end, "Incorrect limit or step (2)!");

			for (auto addr = start; addr <= end; ++addr)
			{
				auto addr_begin = reinterpret_cast<opt_val*>(addr);

				auto rng_begin = rng_checked.data( );
				auto rng_end = rng_begin + rng_checked.size( );

				do
				{
					const auto& opt = *rng_begin;
					if (opt.has_value( ))
					{
						if (*opt != *addr_begin)
							goto NOT_FOUND;
					}

					++rng_begin;
					if (rng_begin == rng_end)
						goto FOUND;
					++addr_begin;
				}
				while (true);

			FOUND:
				return memory_block(addr, rng_size_bytes);
			NOT_FOUND:
				(void)0;
			}

			BOOST_ASSERT("Wrapped memory block not found!");
			return empty_block;
		}

	public:
		template <bool CheckReadable = /*_DEBUG*/false, typename T>
		memory_block find_block(const T& block) const
		{
			if constexpr (!ranges::range<T>)
			{
				BOOST_STATIC_ASSERT_MSG(!std::is_pointer_v<T> && std::is_trivially_destructible_v<T>, "Unsupported block data type!");
				auto& fake_array = reinterpret_cast<const std::array<uint8_t, sizeof(T)>&>(block);
				return Check_block_raw_<CheckReadable>(fake_array);
			}
			else
			{
				BOOST_STATIC_ASSERT_MSG(ranges::random_access_range<T>, "Unsupported range type!");

				using rng_val = ranges::range_value_t<T>;
				if constexpr (detail::have_has_value<rng_val>)
					return Check_block_wrapped_<CheckReadable>(block);
				else
					return Check_block_raw_<CheckReadable>(block);
			}
		}

		template <bool CheckReadable = /*_DEBUG*/false, typename T>
		vector<memory_block> find_all_blocks(const T& block) const
		{
			//todo: do it with ranges

			auto data = vector<memory_block>( );

			auto from = *this;
			while (true)
			{
				auto found_block = from.find_block<CheckReadable>(block);
				if (found_block.empty( ))
					break;

				from = from.shift_to_end(found_block);
				data.push_back(static_cast<memory_block&&>(found_block));
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

		bool have_flags(DWORD flags) const;
		bool dont_have_flags(DWORD flags) const;

		bool readable( ) const;
		bool readable_ex( ) const;
		bool writable( ) const;
		bool executable( ) const;
	};
}
