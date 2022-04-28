module;

#include <memory>

export module cheat.gui2.render;
export import cheat.gui2.object;

export namespace cheat::gui2
{
	class render : public virtual type_info
	{
	public:
		virtual ~render( );

		virtual bool begin( ) = 0;
		virtual void run(objects_storage* const root_storage) = 0;
		virtual bool end( ) = 0;
	};

	template<class T, typename ...Args>
	void set_render(Args&&...args) noexcept;

	render* const get_render( ) noexcept;
}

using namespace cheat;
using render_obj = std::unique_ptr<gui2::render>;
void set_render_impl(render_obj&& obj) noexcept;

template<class T, typename ...Args>
void gui2::set_render(Args&&...args) noexcept
{
	render_obj obj = std::make_unique<T>(std::forward<Args>(args)...);
	set_render_impl(std::move(obj));
}
