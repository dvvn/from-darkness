#pragma once

#include "overload.h"
#include "runtime assert.h"

#include <tuple>
#include <type_traits>

namespace nstd
{
	// class size is only 4 bytes on x86-32 and 8 bytes on x86-64.
	class address
	{
		void error_handler( ) const;

	public:
		address( );
		address(uintptr_t a);
		explicit address(std::nullptr_t);
		address(const void* a);

		uintptr_t value( ) const;

		/// @brief cast / add offset and cast.
		template <typename T>
		T cast( ) const
		{
			error_handler( );
			return (T)value_;
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

		std::strong_ordering operator<=>(const address& other) const;

		bool operator==(const address& other) const;
		bool operator!=(const address& other) const;

		address& operator+=(const address& offset);
		address& operator-=(const address& offset);
		address& operator*=(const address& offset);
		address& operator/=(const address& offset);
		address  operator+(const address& offset) const;
		address  operator-(const address& offset) const;
		address  operator*(const address& offset) const;
		address  operator/(const address& offset) const;

		address add(const address& offset) const;
		address remove(const address& offset) const;
		address multiply(const address& value) const;
		address divide(const address& value) const;

		//--------------------------

		// follow relative8 and relative16/32 offsets.
		address rel8(size_t offset) const;
		address rel32(size_t offset) const;

	private:
		union
		{
			// ReSharper disable CppInconsistentNaming
			uintptr_t   value_;
			const void* ptr_;
			// ReSharper restore CppInconsistentNaming
		};
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
		auto do_invoke(Q&& obj, Curr&& fn, A&&...args)
		{
			//using fn_ret = std::invoke_result_t<Curr, Q>;//something wrong inside of this
			using fn_ret = decltype(ret_val(fn, obj));
			constexpr auto args_count = sizeof...(A);
			if constexpr (std::is_void_v<fn_ret>)
			{
				std::invoke(fn, obj);
				if constexpr (args_count > 0)
					return do_invoke(std::forward<Q>(obj), std::forward<A>(args)...);
				else
					return obj;
			}
			else
			{
				auto new_obj = std::invoke(fn, obj);
				if constexpr (args_count > 0)
					return do_invoke(std::move(new_obj), std::forward<A>(args)...);
				else
					return new_obj;
			}
		}

		template <typename T, typename ...Ts>
		struct address_pipe_impl: address_pipe_tag
		{
			address_pipe_impl(T&& addr, std::tuple<Ts...>&& tpl)
				: addr(std::move(addr)),
				  line(std::move(tpl))
			{
			}

			address_pipe_impl(const T& addr, std::tuple<Ts...>&& tpl)
				: addr(addr),
				  line(std::move(tpl))
			{
			}

			T                 addr;
			std::tuple<Ts...> line;

#ifdef _DEBUG
			mutable bool done = false;
#endif
			auto get_addr( ) const
			{
				if constexpr (!std::invocable<T>)
					return addr;
				else
					return std::invoke(addr);
			}

			template <size_t ...I>
			auto unpack(std::index_sequence<I...>) const
			{
				return do_invoke(get_addr( ), std::get<I>(line)...);
			}

			auto operator()( ) const
			{
#ifdef _DEBUG
				runtime_assert(done == false);
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
			return [=](address& addr)
			{
				addr -= value;
			};
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
		inline constexpr auto ref = nstd::overload([](const address& addr) -> const T&
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
			return address_pipe_impl<address, T>
				(
				 addr, std::make_tuple(obj)
				);
		}

		template <typename Tpl, typename T, size_t ...Idx>
		auto tuple_cat_custom(Tpl&& tpl, T&& val, std::index_sequence<Idx...>)
		{
			return std::make_tuple(std::move(std::get<Idx>(tpl))..., std::forward<T>(val));
		}

		template <typename T, typename ...Ts>
		auto operator|(address_pipe_impl<Ts...>&& pipe, T&& obj)
		{
			return address_pipe_impl<Ts..., T>
				(
				 std::move(pipe.addr), tuple_cat_custom(pipe.line, std::forward<T>(obj), std::make_index_sequence<sizeof...(Ts) - 1>( ))
				);
		}

		template <std::invocable Fn, typename T>
			requires(!std::derived_from<Fn, address_pipe_tag>)
		auto operator|(Fn&& addr, T&& obj)
		{
			return address_pipe_impl<std::remove_cvref_t<Fn>, T>
				(
				 std::forward<Fn>(addr), std::make_tuple(std::forward<T>(obj))
				);
		}
	}
}