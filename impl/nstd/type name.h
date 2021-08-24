#pragma once

namespace nstd
{
	template <typename T, chars_cache ...Ignore>
	constexpr std::string_view type_name( )
	{
		static_assert(sizeof...(Ignore) == 0 || std::_All_same<char, typename decltype(Ignore)::value_type...>::value);

		constexpr auto full_name    = std::string_view(__FUNCSIG__);
		constexpr auto left_marker  = std::string_view("type_name<");
		constexpr auto right_marker = std::string_view(sizeof...(Ignore) == 0
														   ? ">(" //type_name<X>
														   : ","  //type_name<X,a,b,c>
													  );

		constexpr auto left_marker_index = full_name.find(left_marker);
		//static_assert(left_marker_index != std::string_view::npos);
		constexpr auto start_index = left_marker_index + left_marker.size( );
		constexpr auto end_index   = full_name.find(right_marker, left_marker_index);
		//static_assert(end_index != std::string_view::npos);
		constexpr auto length = end_index - start_index;

		constexpr auto obj_name = [&]
		{
			auto name = full_name.substr(start_index, length);
			if constexpr (std::_Has_class_or_enum_type<T>)
			{
				//type_name<class X>
				constexpr std::string_view skip[] = {"struct", "class", "enum", "union"};
				for (auto& s: skip)
				{
					if (name.starts_with(s))
					{
						name.remove_prefix(s.size( ));
						//type_name<class X >
						if (name.starts_with(' '))
							name.remove_prefix(1);
						break;
					}
				}
			}

			//type_name<...,>
			if (name.ends_with(','))
				name.remove_suffix(1);

			return name;
		}( );
		constexpr auto ignore_buffer = []
		{
			if constexpr (sizeof...(Ignore) == 0)
				return std::array<char, 0>( );
			else
			{
				auto buff     = std::array<char, ((Ignore.view( ).size( ) + 2/*::*/) + ...) + 1/*\0*/>( );
				buff.back( )  = '\0';
				auto   names  = std::array{Ignore.view( )...};
				size_t offset = 0;
				for (std::string_view& str: names)
				{
					for (auto c: str)
						buff[offset++] = c;

					buff[offset++] = ':';
					buff[offset++] = ':';
				}
				return buff;
			}
		}( );

		constexpr auto left_marker2 = [&]
		{
			if constexpr (sizeof...(Ignore) == 0)
				return std::string_view( );
			else
				return std::string_view(ignore_buffer._Unchecked_begin( ), ignore_buffer.size( ) - 1);
		}( );

		constexpr auto ret_val = [&]
		{
			const auto left_marker_index2 = obj_name.find(left_marker2);
			if (left_marker_index2 != std::string_view::npos)
			{
				return obj_name.substr(left_marker_index2 + left_marker2.size( ));
			}
			else
			{
				const auto space_index = obj_name.find(' ');
				return space_index == std::string_view::npos ? obj_name : obj_name.substr(space_index + 1);
			}
		}( );
		/*constexpr auto ret_val_namespaces_dropped = [&]
		{
			size_t end;
			if (const auto internal_template_start = ret_val.find('<'); internal_template_start == ret_val.npos)
			{
				end = ret_val.rfind(':');
			}
			else
			{
				const auto ret_val2 = ret_val.substr(0, internal_template_start - 1);
				end                 = ret_val2.rfind(':');
			}

			if (end == ret_val.npos)
				return ret_val;

			return ret_val.substr(end + 1);
		}( );

		return drop_namespace ? ret_val_namespaces_dropped : ret_val;*/

		return ret_val;
	}
}
