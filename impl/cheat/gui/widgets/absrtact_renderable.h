#pragma once

namespace cheat::gui::widgets
{
	struct abstract_renderable
	{
		virtual ~abstract_renderable() = default;
		virtual void render() =0;
	};
}
