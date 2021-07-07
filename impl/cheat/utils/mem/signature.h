#pragma once

namespace cheat::utl::mem
{
	namespace detail
	{
		template <typename T>
		class simple_optional
		{
		public:
			constexpr simple_optional(T val): val__(val),
											  has_val__(true)
			{
			}

			constexpr simple_optional( ): val__(make_default<T>( )),
										  has_val__(false)
			{
			}

			constexpr bool has_value( ) const
			{
				return has_val__;
			}

			constexpr T operator*( ) const
			{
				return val__;
			}

			optional<T> parent( ) const
			{
				if (has_val__)
					return val__;
				return { };
			}

		private:
			T    val__;
			bool has_val__;
		};

		class test_to_byte_fn
		{
			struct char_and_number
			{
				uint8_t number;
				uint8_t character;
			};

			static constexpr simple_optional<uint8_t> Get_byte_(uint8_t chr)
			{
				// ReSharper disable once CppInconsistentNaming
#define INFO__(num) char_and_number(0x##num,#num[0])
				constexpr std::array data{
					INFO__(0),INFO__(1),INFO__(2),INFO__(3),INFO__(4),INFO__(5),INFO__(6),INFO__(7),INFO__(8),INFO__(9),
					INFO__(a),INFO__(b),INFO__(c),INFO__(d),INFO__(e),INFO__(f),
					INFO__(A),INFO__(B),INFO__(C),INFO__(D),INFO__(E),INFO__(F)
				};
#undef INFO__
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

		public:
			template <typename Rng>
			constexpr simple_optional<uint8_t> operator()(Rng&& rng) const
			{
				auto str = basic_string_view(rng);
				BOOST_ASSERT_MSG(str.size( ) == 1 || str.size( ) == 2, "Unable to convert text to byte");

#ifdef _DEBUG
				if constexpr (sizeof(typename decltype(str)::value_type) != sizeof(uint8_t))
				{
					for (size_t chr: str)
					{
						BOOST_ASSERT_MSG(chr < std::numeric_limits<uint8_t>::max( ), "Unable to convert unicode text to byte");
					}
				}
#endif

				if (str.find('?') != str.npos)
				{
					BOOST_ASSERT_MSG(str.find_first_not_of('?') == str.npos, "Wrong unknown text byte");
					//return {uint8_t(-1)};
					return { };
				}

				auto part = Get_byte_(str[0]);
				BOOST_ASSERT_MSG(part.has_value(), "Wrong byte 1");

				if (str.size( ) == 1)
					return {*part};

				auto part2 = Get_byte_(str[1]);
				BOOST_ASSERT_MSG(part2.has_value(), "Wrong byte 2");

				return {static_cast<size_t>(*part) * 16 + *part2};
			}
		};
		static constexpr auto text_to_byte = test_to_byte_fn( );

		template <typename Rng, typename Fn>
		void _Parse_text_as_bytes(Rng&& rng, Fn&& store_fn)
		{
			auto str_view = basic_string_view(rng);

			const auto clamp_spaces = [&]
			{
				const auto spaces_end = str_view.find_first_not_of(' ');
				if (spaces_end != str_view.npos)
					str_view.remove_prefix(spaces_end);
			};

			clamp_spaces( );
			BOOST_ASSERT_MSG(!str_view.empty(), "Signature range is empty!");

			while (!str_view.empty( ))
			{
				const auto end = str_view.find_first_of(' ');
				BOOST_ASSERT_MSG(end != str_view.npos || str_view.size() <= 2, "End of text byte not found");

				const auto part = str_view.substr(0, end);
				const auto byte = text_to_byte(part);

				/*if constexpr (std::same_as<typename T::value_type, uint8_t>)
				{
					BOOST_ASSERT_MSG(byte.has_value(), "Unknown byte detected, all bytes must be known");
					storage.push_back(*byte);
				}
				else
				{
					storage.push_back(byte);
				}*/
				invoke(store_fn, byte);
				if (end == str_view.npos)
					break;

				str_view.remove_prefix(end);
				clamp_spaces( );
			}

			//storage.shrink_to_fit( );
		}

		using signature_bytes = vector<optional<uint8_t>>;
		using signature_bytes_raw = vector<uint8_t>;
	}

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
				auto       vec = signature_bytes_raw( );
				const auto store_fn = [&vec](const simple_optional<uint8_t>& b)
				{
					BOOST_ASSERT_MSG(b.has_value(), "Unknown byte detected, all bytes must be known in TEXT_AS_BYTES mode!");
					vec.push_back(*b);
				};
				_Parse_text_as_bytes(text, store_fn);

				vec.shrink_to_fit( );
				return (vec);
			}
		};

		template < >
		struct signature_creator_impl<BYTES>
		{
			template <typename T>
			constexpr auto operator()(const T& val) const
			{
				if constexpr (std::is_trivially_destructible_v<T>)
				{
					if constexpr (ranges::random_access_range<T>)
					{
						using rng_val = ranges::range_value_t<T>;
						static_assert(!std::is_pointer_v<rng_val>);
						if constexpr (sizeof(rng_val) != sizeof(std::byte))
							return as_bytes(span(ranges::data(val), ranges::size(val)));
						else
						{
							if constexpr (std::is_same_v<rng_val, char>)
								return string_view(val);
							else
								return span(val);
						}
					}
					else
					{
						static_assert(!ranges::range<T>);
						return as_bytes(span(addressof(val), 1));
					}
				}
				else
				{
					static_assert(ranges::/*random_access_*/range<T>);
					using rng_val = ranges::range_value_t<T>;
					static_assert(std::is_trivially_destructible_v<rng_val>);

					auto vec = signature_bytes_raw( );

					for (auto& v: val)
					{
						for (const auto b: this->operator()(val))
						{
							vec.push_back(reinterpret_cast<const uint8_t>(b));
						}
					}

					return vec;
				}

#if 0
                if constexpr (/*simple_type*/std::is_trivially_destructible_v<T>)
                {
                    static_assert(!std::is_pointer_v<T>);
                    if constexpr (!ranges::range<T>)
                    {
                        return as_bytes(span(addressof(val), 1));
                    }
                    else
                    {
                        using raw_t = std::remove_cvref_t<T>;
                        if constexpr (std::is_class_v<raw_t> && !std::is_trivially_destructible_v<raw_t>)
                            return static_cast<const T&>(val);
                        else
                            return basic_string_view(val);
                    }
                }
                else
                {
                    auto vec = signature_bytes_raw( );

                    const auto bytes = bytes_view(val);
                    vec.reserve(bytes.size( ));
                    for (auto b: bytes)
                        vec.push_back(b);

                    return move(vec);
                }
#endif
			}
		};

		template < >
		struct signature_creator_impl<TEXT>
		{
			template <typename Txt>
			auto operator()(Txt&& text) const
			{
				auto vec = signature_bytes( );

				const auto store_fn = [&](const simple_optional<uint8_t>& b)
				{
					vec.push_back(b.parent( ));
				};
				_Parse_text_as_bytes(text, store_fn);

				return (vec);
			}
		};

		template < >
		struct signature_creator_impl<AUTO>
		{
			signature_creator_impl( ) = delete;
		};
	}

	template <signature_parse_mode M /*= AUTO*/, typename T>
	constexpr auto signature(const T& data)
	{
		static_assert(M != AUTO,"AUTO mode disabled");
		/*if constexpr (M == AUTO)
			return signature<string_viewable<T> ? TEXT : BYTES>(data);
		else*/
		return invoke(detail::signature_creator_impl<M>( ), (data));
	}
}
