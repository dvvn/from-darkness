module;

#include <memory>

export module cheat.gui2.texture;
export import cheat.gui2.type_info;

export namespace cheat::gui2
{
	class texture : public virtual type_info
	{
	public:
		virtual ~texture( );
		virtual void update( ) = 0;
	};

	template<class T>
	auto make_texture( ) noexcept
	{
		static_assert(std::is_convertible_v<T*, texture*>, __FUNCSIG__": incorrect type passed!");
		return std::make_unique<T>( );
	}

	class texture_renderer
	{
	public:
		using value_type = std::unique_ptr<texture>;
		using pointer = texture* const;

		virtual ~texture_renderer( );

		pointer get_texture( ) const noexcept;

	protected:
		pointer set_texture(value_type&& tex) noexcept;

	private:
		value_type texture_;
	};
}