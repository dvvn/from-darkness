module;

#include <vector>
#include <memory>

export module cheat.gui2.object;
export import cheat.gui2.event;
export import cheat.gui2.texture;

export namespace cheat::gui2
{
	class object : public virtual type_info, public unique_factory<object>
	{
	public:
		using texture_type = unique_pointer<texture>;

		virtual bool handle_event(event* const ev) noexcept = 0;

		texture* get_texture( ) noexcept;
		void set_texture(texture_type&& tex) noexcept;

	private:
		texture_type texture_;
	};

	template<class T = object, class ValT = unique_pointer<T>>
	class objects_storage :public std::vector<ValT, allocator_t<ValT>>
	{
	};

}