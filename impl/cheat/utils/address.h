#pragma once

namespace cheat::utl
{
	// class size is only 4 bytes on x86-32 and 8 bytes on x86-64.
	class address
	{
		uintptr_t value__;

		void Error_handler_( ) const;

	public:
		address( ) : value__(0)
		{
		}

		address(uintptr_t a) : value__(a)
		{
		}

		explicit address(std::nullptr_t) : address( )
		{
		}

		address(const void* a) : address(reinterpret_cast<uintptr_t>(a))
		{
		}

		uintptr_t value( ) const;

		/// @brief cast / add offset and cast.
		template <typename T>
		T cast( ) const
		{
			Error_handler_( );
			return (T)value__;
		}

		template <typename T = uintptr_t>
		T* ptr( ) const
		{
			return cast<T*>( );
		}

		template <typename T = uintptr_t>
		const T& ref( ) const
		{
			return *ptr<T>( );
		}

		template <typename T = uintptr_t>
		T& ref( )
		{
			return *ptr<T>( );
		}

		address operator*( ) const;

		//derefference
		address deref(size_t count) const;
		address deref_safe(size_t count) const;

		constexpr auto operator<=>(const address& other) const = default;

		address& operator+=(const address& offset);
		address& operator-=(const address& offset);
		address& operator*=(const address& offset);
		address& operator/=(const address& offset);
		address operator+(const address& offset) const;
		address operator-(const address& offset) const;
		address operator*(const address& offset) const;
		address operator/(const address& offset) const;

		address add(const address& offset) const;
		address remove(const address& offset) const;
		address multiply(const address& value) const;
		address divide(const address& value) const;

		//--------------------------

		// follow relative8 and relative16/32 offsets.
		address rel8(size_t offset) const;
		address rel32(size_t offset) const;
	};

	namespace address_pipe
	{
		struct address_pipe_tag
		{
		};

		template <typename Fn, typename Arg>
		auto ret_val(const Fn& fn, Arg& arg)
		{
			return fn(arg);
		}

		template <typename Q, typename Curr, typename ...A>
		static auto do_invoke(Q&& obj, Curr&& fn, A&&...args)
		{
			//using fn_ret = std::invoke_result_t<Curr, Q>;//something wrong inside of this
			using fn_ret = decltype(ret_val(fn, obj));

			constexpr auto args_count = sizeof...(A);

			if constexpr (std::is_void_v<fn_ret>)
			{
				utl::invoke(fn, obj);
				if constexpr (args_count > 0)
					return do_invoke(forward<Q>(obj), forward<A>(args)...);
				else
					return obj;
			}
			else
			{
				auto new_obj = utl::invoke(fn, obj);
				if constexpr (args_count > 0)
					return do_invoke(move(new_obj), forward<A>(args)...);
				else
					return new_obj;
			}
		}

		template <typename T, typename ...Ts>
		struct address_pipe_impl: address_pipe_tag
		{
			address_pipe_impl(T&& addr, tuple<Ts...>&& tpl) : addr(move(addr)),
															  line(move(tpl))
			{
			}

			address_pipe_impl(const T& addr, tuple<Ts...>&& tpl) : addr((addr)),
																   line(move(tpl))
			{
			}

			T addr;
			tuple<Ts...> line;

#ifdef _DEBUG
			mutable bool done = false;
#endif
			auto get_addr( ) const
			{
				if constexpr (!std::invocable<T>)
					return addr;
				else
					return utl::invoke(addr);
			}

			template <size_t ...I>
			auto unpack(std::index_sequence<I...>) const
			{
				return do_invoke(get_addr( ), utl::get<I>(line)...);
			}

			auto operator()( ) const
			{
#ifdef _DEBUG
				BOOST_ASSERT(done==false);
				done = true;
#endif
				return unpack(std::index_sequence_for<Ts...>( ));
			}
		};

		template <typename T>
		auto add(const T& value)
		{
			return [=](address& addr)
			{
				addr += value;
			};
		}

		template <typename T>
		auto remove(const T& value)
		{
			return ([=](address& addr)
			{
				addr -= value;
			});
		}

		template <typename T>
		auto multiply(const T& value)
		{
			return [=](address& addr)
			{
				addr *= value;
			};
		}

		template <typename T>
		auto divide(const T& value)
		{
			return [=](address& addr)
			{
				addr /= value;
			};
		}

		inline auto deref(size_t count)
		{
			return [=](const address& addr)
			{
				return addr.deref(count);
			};
		}

		inline constexpr auto value = [](const address& addr)
		{
			return addr.value( );
		};

		template <typename T>
		inline constexpr auto cast = [](const address& addr) -> T
		{
			return addr.cast<T>( );
		};

		template <typename T>
		inline constexpr auto ref = overload([](const address& addr) -> const T&
											 {
												 return addr.ref<T>( );
											 },
											 [](address& addr) -> T&
											 {
												 return addr.ref<T>( );
											 });

		template <typename T>
		inline constexpr auto ptr = [](const address& addr) -> T*
		{
			return addr.ptr<T>( );
		};

		template <typename T>
		auto operator|(const address& addr, const T& obj)
		{
			return address_pipe_impl
					<address, T>
					(addr, make_tuple(obj));
		}

		template <typename Tpl, typename T, size_t ...Idx>
		auto tuple_cat_custom(Tpl&& tpl, T&& val, std::index_sequence<Idx...>)
		{
			return make_tuple(move(get<Idx>(tpl))..., forward<T>(val));
		}

		template <typename T, typename ...Ts>
		auto operator|(address_pipe_impl<Ts...>&& pipe, T&& obj)
		{
			return address_pipe_impl<Ts..., T>
					(
					 move(pipe.addr), tuple_cat_custom(pipe.line, forward<T>(obj), std::make_index_sequence<sizeof...(Ts) - 1>( ))
					);
		}

		template <std::invocable Fn, typename T>
			requires(!std::derived_from<Fn, address_pipe_tag>)
		auto operator|(Fn&& addr, T&& obj)
		{
			return address_pipe_impl<std::remove_cvref_t<Fn>, T>
					(
					 forward<Fn>(addr), make_tuple(forward<T>(obj))
					);
		}
	}
}