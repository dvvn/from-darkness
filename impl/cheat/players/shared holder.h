#pragma once

#include "nstd/runtime assert.h"

#include <functional>
#include <memory>

namespace cheat::detail
{
	template <std::default_initializable T>
	class shared_holder: protected std::shared_ptr<T>
	{
		std::shared_ptr<T>& Get_shared_( )
		{
			return *static_cast<std::shared_ptr<T>*>(this);
		}

		const std::shared_ptr<T>& Get_shared_( ) const
		{
			return *static_cast<const std::shared_ptr<T>*>(this);
		}

	protected:
		shared_holder( ) = default;

		virtual ~shared_holder( )
		{
			auto& ptr = Get_shared_( );
			if (ptr == nullptr)
				return;

			auto& in_use = ptr->in_use;
			runtime_assert(in_use == true);

			in_use = false;

			if (destroy_fn_)
				destroy_fn_(*ptr);
		}

		using destroy_fn_type = std::function<void(T&)>;
		destroy_fn_type destroy_fn_;

		// ReSharper disable once CppInconsistentNaming
		void reset( ) = delete;

	public:
		shared_holder(const shared_holder&)            = delete;
		shared_holder& operator=(const shared_holder&) = delete;

		/*virtual*/
		void init( )
		{
			auto& ptr = Get_shared_( );
			runtime_assert(ptr == nullptr);
			ptr         = std::make_shared<T>( );
			ptr->in_use = 1;
		}

		const std::shared_ptr<T>& share( ) const
		{
			return Get_shared_( );
		}

		shared_holder(shared_holder&& other) noexcept
		{
			*this = std::move(other);
		}

		void operator=(shared_holder&& other) noexcept
		{
			std::swap(Get_shared_( ), other.Get_shared_( ));
			std::swap(destroy_fn_, other.destroy_fn_);
		}

		bool operator==(nullptr_t) const
		{
			return Get_shared_( ) == nullptr;
		}

		bool operator!=(nullptr_t) const
		{
			return !(*this == nullptr);
		}
	};

	struct shared_holder_info
	{
		bool in_use = 0;
	};
}
