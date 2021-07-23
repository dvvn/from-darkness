#pragma once
namespace cheat::detail
{
	template <std::default_initializable T>
	class shared_holder: protected utl::shared_ptr<T>, public utl::noncopyable
	{
		utl::shared_ptr<T>& Get_shared_( )
		{
			return *static_cast<utl::shared_ptr<T>*>(this);
		}

		const utl::shared_ptr<T>& Get_shared_( ) const
		{
			return *static_cast<const utl::shared_ptr<T>*>(this);
		}

	protected:
		shared_holder( ) = default;

		virtual ~shared_holder( )
		{
			auto& ptr = Get_shared_( );
			if (ptr == nullptr)
				return;

			auto& in_use = ptr->in_use;
			BOOST_ASSERT(in_use==true);

			in_use = false;

			if (destroy_fn_)
				destroy_fn_(*ptr);
		}

		using destroy_fn_type = utl::function<void(T&)>;
		destroy_fn_type destroy_fn_;

		// ReSharper disable once CppInconsistentNaming
		void reset( ) = delete;

	public:
		virtual void init( )
		{
			auto& ptr = Get_shared_( );
			BOOST_ASSERT(ptr==nullptr);
			ptr = utl::make_shared<T>( );
			ptr->in_use = 1;
		}

		const utl::shared_ptr<T>& share( ) const
		{
			return Get_shared_( );
		}

		shared_holder(shared_holder&& other) noexcept
		{
			*this = utl::move(other);
		}

		void operator=(shared_holder&& other) noexcept
		{
			Get_shared_( ) = utl::move(other.Get_shared_( ));
			destroy_fn_ = utl::move(other.destroy_fn_);
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
