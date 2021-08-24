#pragma once
#include "memory block.h"

namespace nstd
{
	namespace detail
	{
		constexpr unknown_byte get_byte(known_byte chr)
		{
			struct char_and_number
			{
				known_byte number;
				known_byte character;
			};

			// ReSharper disable once CppInconsistentNaming
#define _C2N_(num) char_and_number(0x##num,#num[0])
			constexpr std::array data{
				_C2N_(0),_C2N_(1),_C2N_(2),_C2N_(3),_C2N_(4),_C2N_(5),_C2N_(6),_C2N_(7),_C2N_(8),_C2N_(9),
				_C2N_(a),_C2N_(b),_C2N_(c),_C2N_(d),_C2N_(e),_C2N_(f),
				_C2N_(A),_C2N_(B),_C2N_(C),_C2N_(D),_C2N_(E),_C2N_(F)
			};
#undef _C2N_
			///dont abuse std::ranges
			/*const auto found = ranges:: find(data, c, [](const info& i) { return i.second; });
			if (found == data.end( ))
				return std::nullopt;
			return found->first;*/

			for (const auto& [number, character]: data)
			{
				if (character == chr)
					return number;
			}

			return { };

			//magic numbers suck
			/*if (c >= '0' && c <= '9')
				return c - '0';
			if (c >= 'a' && c <= 'f')
				return c - 87;
			if (c >= 'A' && c <= 'F')
				return c - 55;

			return std::nullopt;*/
		}

		template <typename E, typename Tr>
		constexpr unknown_byte text_to_byte(const std::basic_string_view<E, Tr>& str)
		{
			//forgot why i add this
			//runtime_assert(str.size( ) == 1 || str.size( ) == 2, "Unable to convert text to byte");

#ifdef _DEBUG
			if constexpr (sizeof(E) != sizeof(known_byte))
			{
				for (const auto chr: str)
				{
					runtime_assert((size_t)chr < std::numeric_limits<known_byte>::max( ), "Unable to convert unicode text to byte");
				}
			}
#endif

			if (str.find('?') != str.npos)
			{
				runtime_assert(str.find_first_not_of('?') == str.npos, "Wrong unknown text byte");
				return { };
			}

			auto part = get_byte(str[0]);
			runtime_assert(part.has_value( ), "Wrong byte 1");

			if (str.size( ) == 1)
				return {*part};

			auto part2 = get_byte(str[1]);
			runtime_assert(part2.has_value( ), "Wrong byte 2");

			return {static_cast<size_t>(*part) * 16 + *part2};
		};

		template <typename E, typename Tr, typename Fn>
		void parse_text_as_bytes(std::basic_string_view<E, Tr> str, Fn&& store_fn)
		{
			const auto clamp_spaces = [&]
			{
				const auto spaces_end = str.find_first_not_of(' ');
				if (spaces_end != str.npos)
					str.remove_prefix(spaces_end);
			};

			clamp_spaces( );
			runtime_assert(!str.empty( ), "Signature range is empty!");

			while (!str.empty( ))
			{
				const auto end = str.find_first_of(' ');
				runtime_assert(end != str.npos || str.size( ) <= 2, "End of text byte not found");

				const auto part = str.substr(0, end);
				const auto byte = text_to_byte(part);

				std::invoke(store_fn, byte);
				if (end == str.npos)
					break;

				str.remove_prefix(end);
				clamp_spaces( );
			}

			//storage.shrink_to_fit( );
		};
	}

	using signature_bytes_raw = std::vector<known_byte>;
	using signature_bytes = std::vector<unknown_byte>;

	enum signature_parse_mode
	{
		/**
		 * \brief store string viewable data as TEXT all others as BYTES.
		 * while data is string viewable, even if all bytes are known, TEXT_AS_BYTES is never selected
		 */
		AUTO,
		/**
		 * \brief text converted to bytes and stored inside special container.
		 * unknown bytes allowed. when possible prefer to TEXT_AS_BYTES if all bytes known
		 */
		TEXT,
		/**
		 * \brief raw memory block. any type is stored like range of bytes
		 */
		BYTES,
		/**
		 * \brief text converted to bytes and stored like raw memory block. all bytes must be known
		 */
		TEXT_AS_BYTES
	};

	namespace detail
	{
		template <signature_parse_mode M>
		struct signature_creator_impl;

		template < >
		struct signature_creator_impl<TEXT_AS_BYTES>
		{
			template <typename Txt>
			signature_bytes_raw operator()(Txt&& text) const
			{
				auto vec = signature_bytes_raw( );

				const auto store_fn = [&vec](const unknown_byte& b)
				{
					runtime_assert(b.has_value( ), "Unknown byte detected, all bytes must be known in TEXT_AS_BYTES mode!");
					vec.push_back(*b);
				};
				parse_text_as_bytes(text, store_fn);

				vec.shrink_to_fit( );
				return vec;
			}
		};

		template < >
		struct signature_creator_impl<BYTES>
		{
			template <typename T>
			constexpr auto operator()(const T& val) const
			{
				if constexpr (!std::is_trivially_destructible_v<T>)
				{
					static_assert(ranges::/*random_access_*/range<T>);
					using rng_val = ranges::range_value_t<T>;
					static_assert(std::is_trivially_destructible_v<rng_val>);

					auto vec = signature_bytes_raw( );

					for (auto& v: val)
					{
						for (const auto b: std::invoke(*this, val))
						{
							static_assert(sizeof(decltype(b)) == sizeof(known_byte));
							vec.push_back(reinterpret_cast<const known_byte>(b));
						}
					}

					return vec;
				}
				else
				{
					if constexpr (!ranges::random_access_range<T>)
					{
						static_assert(!ranges::range<T>);
						return std::as_bytes(std::span(std::addressof(val), 1));
					}
					else
					{
						using rng_val = ranges::range_value_t<T>;
						static_assert(!std::is_pointer_v<rng_val>);

						if constexpr (sizeof(rng_val) != sizeof(known_byte))
							return std::as_bytes(std::span(ranges::data(val), ranges::size(val)));
						else
						{
							if constexpr (std::is_same_v<rng_val, char>)
								return std::string_view(val);
							else
								return std::span(val);
						}
					}
				}
			}
		};

		template < >
		struct signature_creator_impl<TEXT>
		{
			template <typename Txt>
			auto operator()(const Txt& text) const
			{
				auto vec = signature_bytes( );

				const auto store_fn = [&](const unknown_byte& b)
				{
					vec.push_back(b);
				};
				parse_text_as_bytes(text, store_fn);

				return vec;
			}
		};

		template < >
		struct signature_creator_impl<AUTO>
		{
			signature_creator_impl( ) = delete;
		};
	}

	template <signature_parse_mode M /*= AUTO*/>
	constexpr auto signature = []<typename T>(const T& data)
	{
		static_assert(M != AUTO, "AUTO mode disabled");
		/*if constexpr (M == AUTO)
			return signature<string_viewable<T> ? TEXT : BYTES>(data);
		else*/
		return std::invoke(detail::signature_creator_impl<M>( ), data);
	};
}
