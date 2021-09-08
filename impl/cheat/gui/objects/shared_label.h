#pragma once
//#include "cheat/gui/tools/string wrapper.h"

#include <memory>

namespace cheat::gui::tools
{
	class string_wrapper;
}

namespace cheat::gui::objects
{
	class abstract_label
	{
	public:
		virtual ~abstract_label( ) = default;

		virtual const tools::string_wrapper& label( ) const = 0;
	};

	using shared_label = std::shared_ptr<abstract_label>;

	class non_abstract_label: public abstract_label
	{
	public:
		non_abstract_label(tools::string_wrapper&& label);
		~non_abstract_label( ) override;

		const tools::string_wrapper& label( ) const final;

	private:
		std::unique_ptr<tools::string_wrapper> label_;
	};

	std::shared_ptr<non_abstract_label> operator""_shl(const char* ptr, size_t size);
	std::shared_ptr<non_abstract_label> operator""_shl(const wchar_t* ptr, size_t size);
}
