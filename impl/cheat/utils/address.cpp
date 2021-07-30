#include "address.h"

using namespace cheat::utl;

void address::Error_handler_( ) const
{
	BOOST_ASSERT_MSG(value__ != 0, "Address is null!");
	BOOST_ASSERT_MSG(value__ != static_cast<uintptr_t>(-1), "Address is incorrect!");
}

uintptr_t address::value( ) const
{
	return value__;
}

address address::operator*( ) const
{
	return ref<uintptr_t>( );
}

address address::deref(size_t count) const
{
	BOOST_ASSERT_MSG(count != 0, "Count must be not zero!");

	auto result = *this;;
	while (count-- > 0)
		result = *result;
	return result;
}

address address::deref_safe(size_t count) const
{
	return count == 0 ? *this : deref(count);
}

address& address::operator+=(const address& offset)
{
	value__ += offset.value__;
	Error_handler_( );
	return *this;
}

address& address::operator-=(const address& offset)
{
	value__ -= offset.value__;
	Error_handler_( );
	return *this;
}

address& address::operator*=(const address& offset)
{
	value__ *= offset.value__;
	Error_handler_( );
	return *this;
}

address& address::operator/=(const address& offset)
{
	value__ /= offset.value__;
	Error_handler_( );
	return *this;
}

address address::operator+(const address& offset) const
{
#ifdef  _DEBUG
	if (offset == 0u)
		return *this;
	auto temp = *this;
	temp += offset;
	return temp;
#else
            return value__ + offset.value__;
#endif
}

address address::operator-(const address& offset) const
{
#ifdef  _DEBUG
	if (offset == 0u)
		return *this;
	auto temp = *this;
	temp -= offset;
	return temp;
#else
            return value__ - offset.value__;
#endif
}

address address::operator*(const address& offset) const
{
#ifdef  _DEBUG
	if (offset == 0u)
		return *this;
	auto temp = *this;
	temp *= offset;
	return temp;
#else
            return value__ * offset.value__;
#endif
}

address address::operator/(const address& offset) const
{
#ifdef  _DEBUG
	if (offset == 0u)
		return *this;
	auto temp = *this;
	temp /= offset;
	return temp;
#else
            return value__ / offset.value__;
#endif
}

address address::add(const address& offset) const
{
	return *this + offset;
}

address address::remove(const address& offset) const
{
	return *this - offset;
}

address address::multiply(const address& value) const
{
	return *this * value;
}

address address::divide(const address& value) const
{
	return *this / value;
}

address address::rel8(size_t offset) const
{
	const auto out = *this + offset;

	// get relative offset.
	const auto r = out.ref<uint8_t>( );
	BOOST_ASSERT_MSG(r != 0, "can't get rel8 offset");
	/*if (!r)
		return t{ };*/

	// relative to address of next instruction.
	// short jumps can go forward and backward depending on the size of the second byte.
	// if the second byte is below 128, the jmp goes forwards.
	// if the second byte is above 128, the jmp goes backwards ( subtract two's complement of the relative offset from the address of the next instruction ).
	if (r < 128)
		return (out + 1) + r;
	else
		return (out + 1) - static_cast<uint8_t>(~r + 1);
}

address address::rel32(size_t offset) const
{
	const auto out = *this + offset;

	// get rel32 offset.
	const auto r = out.ref<uint32_t>( );
	BOOST_ASSERT_MSG(r != 0, "can't get rel32 offset");

	// relative to address of next instruction.
	return (out + 4) + r;
}