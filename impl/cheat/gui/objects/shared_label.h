#pragma once
//#include "cheat/gui/tools/string wrapper.h"

#include <memory>

// ReSharper disable CppInconsistentNaming
struct ImVec2;
// ReSharper restore CppInconsistentNaming

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

		virtual const tools::string_wrapper& get_label( ) const =0;
		/*void                         set_label(tools::string_wrapper&& new_label);

	protected:
		virtual tools::string_wrapper& get_label_impl( ) =0;

	public:
		const ImVec2& get_size( ) const;*/

		/*private:
			struct impl;
			std::unique_ptr<impl> impl_;*/
	};

	using shared_label = std::shared_ptr<abstract_label>;

	class non_abstract_label: public abstract_label
	{
	public:
		non_abstract_label(tools::string_wrapper&& label);
		~non_abstract_label( ) override;

		 const tools::string_wrapper& get_label( ) const override;

	private:
		std::unique_ptr<tools::string_wrapper> label_;
	};

	[[deprecated]]
	std::shared_ptr<non_abstract_label> operator""_shl(const char* ptr, size_t size);
	[[deprecated]]
	std::shared_ptr<non_abstract_label> operator""_shl(const wchar_t* ptr, size_t size);
}
