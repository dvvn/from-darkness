#pragma once

#include "cheat/gui/objects/renderable object.h"

#include <memory>

// ReSharper disable CppInconsistentNaming
struct ImGuiWindow;
struct ImFont;
struct ImRect;
struct ImVec2;
// ReSharper restore CppInconsistentNaming

namespace cheat::gui::tools
{
	class perfect_string;
	class string_wrapper;
}

namespace cheat::gui::widgets
{
	class text : public objects::renderable
	{
	public:
		text();
		~text() override;

		text(text&&) noexcept;
		text& operator=(text&&) noexcept;

		void render() override;

	protected:
		void   render_text(ImGuiWindow* wnd, const ImVec2& pos);
		ImRect make_rect(ImGuiWindow* wnd) const;

	public:
		void    set_font(ImFont* font);
		ImFont* get_font();

		void set_label(tools::string_wrapper&& label);

		const tools::string_wrapper& get_label() const;
		const ImVec2&                label_size() const;

	private:
		struct data;
		std::unique_ptr<data> data_;
	};
}
