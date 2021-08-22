#include "runtime assert.h"

using namespace nstd;

rt_assert_handler_ex nstd::rt_assert_object;

void rt_assert_handler::handle(bool result, chr_wchr&& expression, chr_wchr&& message, chr_wchr&& file_name, chr_wchr&& function, unsigned __int64 line) noexcept
{
	if (result == true)
		return;

	this->handle_impl(expression, message, {std::move(file_name), std::move(function), line});
}

rt_assert_handler_ex::rt_assert_handler_ex( )
{
	data_ = new data_type( );
}

rt_assert_handler_ex::~rt_assert_handler_ex( )
{
	if (data_ == nullptr)
		return;

	for (auto&& entry: *static_cast<data_type*>(data_) | ranges::views::reverse)
		delete entry;

	delete static_cast<data_type*>(data_);
}

rt_assert_handler_ex::rt_assert_handler_ex(rt_assert_handler_ex&& other) noexcept
{
	*this = std::move(other);
}

rt_assert_handler_ex& rt_assert_handler_ex::operator=(rt_assert_handler_ex&& other) noexcept
{
	std::swap(data_, other.data_);
	return *this;
}

void rt_assert_handler_ex::add(rt_assert_handler* handler)
{
	(void)this;
	static_cast<data_type*>(data_)->push_back(handler);
}

void rt_assert_handler_ex::remove(rt_assert_handler* handler)
{
	(void)this;
	auto& data = *static_cast<data_type*>(data_);
	auto  pos  = ranges::remove(data, handler);
	data.erase(pos.begin( ), pos.end( ));
}

void rt_assert_handler_ex::handle_impl(const chr_wchr& expression, const chr_wchr& message, const info_type& info) noexcept
{
	for (auto&& entry: *static_cast<data_type*>(data_))
		entry->handle_impl(expression, message, info);
}
