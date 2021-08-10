#pragma once

namespace cheat::utl
{
	namespace detail
	{
		struct checksum_impl
		{
			template <typename Chr>
			string operator ()(const basic_string_view<Chr>& s) const
			{
				return to_string(invoke(std::hash<basic_string_view<Chr>>( ), s));
			}

			template <typename T>
			string operator ()(const span<T>& vec) const
			{
				return to_string(std::_Hash_array_representation(vec._Unchecked_begin( ), vec._Unchecked_end( )));
			}

			string operator()(const filesystem::path& p) const
			{
				auto str = string( );

				if (exists(p))
				{
					auto ifs = std::ifstream(p);
					using itr_t = std::istreambuf_iterator<char>;
					if (!ifs.fail( ))
					{
						const auto tmp = vector(itr_t(ifs), itr_t( ));
						str            = invoke(*this, tmp);
					}
				}

				return str;
			}

			template <typename Chr>
			string operator()(const std::basic_ostringstream<Chr>& ss) const
			{
				return invoke(*this, ss.str( ));
			}
		};
	}

	inline static constexpr auto checksum = detail::checksum_impl( );
}
