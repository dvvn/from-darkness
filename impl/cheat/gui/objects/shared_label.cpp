#include "shared_label.h"

#include "cheat/gui/tools/string wrapper.h"

#include <string>

using namespace cheat::gui::objects;
using cheat::gui::tools::string_wrapper;

non_abstract_label::non_abstract_label(string_wrapper&& label)
{
	label_ = std::make_unique<string_wrapper>(std::move(label));
}

non_abstract_label::~non_abstract_label( )
{
}

const string_wrapper& non_abstract_label::label( ) const
{
	return *label_;
}

template <typename T>
static auto _Make_shared_label(const T* ptr, size_t size)
{
	return std::make_shared<non_abstract_label>(string_wrapper(std::basic_string<T>(ptr, std::next(ptr, size))));
}

std::shared_ptr<non_abstract_label> cheat::gui::objects::operator ""_shl(const char* ptr, size_t size)
{
	return _Make_shared_label(ptr, size);
}

std::shared_ptr<non_abstract_label> cheat::gui::objects::operator ""_shl(const wchar_t* ptr, size_t size)
{
	return _Make_shared_label(ptr, size);
}
