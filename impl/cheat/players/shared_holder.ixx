module;

#include <memory>

export module cheat.players.shared_holder;

export namespace cheat
{
	template <class T>
	struct shared_holder_obj : T
	{
		template <typename ...Ts>
		shared_holder_obj(Ts&& ...types)
			: T(std::forward<Ts>(types)...)
		{
		}

		bool held = true;
	};

	template <class T>
	class shared_holder
	{
	public:
		using element_type = shared_holder_obj<T>;
		using shared_type = std::shared_ptr<element_type>;

		shared_holder(const shared_holder& other) = delete;
		shared_holder& operator=(const shared_holder& other) = delete;

		shared_holder(shared_holder&& other) noexcept
		{
			shared_ = std::move(other.shared_);
		}

		shared_holder& operator=(shared_holder&& other) noexcept
		{
			std::swap(shared_, other.shared_);
			return *this;
		}

		template <typename ...Ts>
		shared_holder(Ts&& ...types)
		{
			shared_ = std::make_shared<element_type>(std::forward<Ts>(types)...);
		}

		~shared_holder( )
		{
			if (!shared_) //moved
				return;
			shared_->held = false;
		}

		//---

		const element_type* get( ) const { return shared_.get( ); }
		auto use_count( ) const { return shared_.use_count( ); }
		const element_type* operator->( ) const { return shared_.operator->( ); }
		const element_type& operator*( ) const { return shared_.operator*( ); }

		const shared_type& share( ) const { return shared_; }

	private:
		shared_type shared_;
	};
}
