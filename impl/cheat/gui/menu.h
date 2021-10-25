#pragma once

#include "cheat/core/service.h"
#include "widgets/absrtact_renderable.h"

// ReSharper disable CppInconsistentNaming
using UINT = unsigned int;
using UINT_PTR =
#ifdef _W64
_W64
#endif
unsigned
#if defined(_WIN64)
__int64
#else
int;
#endif
using WPARAM = UINT_PTR;
// ReSharper restore CppInconsistentNaming

#if defined(_DEBUG) ||  defined(CHEAT_GUI_TEST)
#define CHEAT_GUI_HAS_DEMO_WINDOW 1
#else
#define CHEAT_GUI_HAS_DEMO_WINDOW 0
#endif

namespace cheat::gui
{
	namespace widgets
	{
		class tab_bar_with_pages;
	}

	class menu final : public service_instance_shared<menu>, public widgets::abstract_renderable
	{
	public:
		menu();
		~menu() override;

		void render() override;
		bool toggle(UINT msg, WPARAM wparam);
		bool visible()const;
		bool updating()const;

	protected:
		load_result load_impl()noexcept override;

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};
}
