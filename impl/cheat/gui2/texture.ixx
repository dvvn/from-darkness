module;

export module cheat.gui2.texture;
export import cheat.gui2.type_info;
export import cheat.gui2.factory;

export namespace cheat::gui2
{
	class texture : public virtual type_info, public unique_factory<texture>
	{
	public:
		virtual ~texture( );

	protected:
		virtual void update_texture( ) noexcept;
	};

	/*class texture_renderer
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
	};*/
}