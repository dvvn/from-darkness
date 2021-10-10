#pragma once

#include <functional>
#include <memory>

namespace cheat::detail
{
	template <std::default_initializable B>
	struct shared_holder_obj : B
	{
		bool held = true;
	};

	template <class T>
	class shared_holder
	{
	public:
		using element_type = shared_holder_obj<T>;
		using shared_type = std::shared_ptr<element_type>;

		shared_holder(const shared_holder& other)            = delete;
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

		shared_holder()
		{
			shared_ = std::make_shared<T>( );
		}

		~shared_holder()
		{
			if (!shared_) //moved
				return;
			shared_->held = false;
		}

		//---

		const element_type* get() const { return shared_.get( ); }
		auto use_count() const { return shared_.use_count( ); }
		const element_type* operator->() const { return shared_.operator->( ); }
		const element_type& operator*() const { return shared_.operator*( ); }

		const shared_type& share() const { return shared_; }

	private:
		shared_type shared_;
	};
}
