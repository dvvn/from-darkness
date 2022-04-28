module;

#include <vector>
#include <memory>

export module cheat.gui2.object;
export import cheat.gui2.event;
export import cheat.gui2.texture;
import nstd.indexed_iterator;

export namespace cheat::gui2
{
	class object;

	class objects_storage
	{
	public:
		using element_type = std::unique_ptr<object>;
		using storage_type = std::vector<element_type>;
		using pointer = object* const;
		using iterator = nstd::indexed_iterator<storage_type>;
		using const_iterator = nstd::indexed_iterator<const storage_type>;

		iterator begin( ) noexcept;
		iterator end( ) noexcept;
		const_iterator begin( ) const noexcept;
		const_iterator end( ) const noexcept;

		size_t size( ) const noexcept;
		bool empty( ) const noexcept;

		pointer add(element_type&& value) noexcept;

	private:
		storage_type storage_;
	};

	class child_storage
	{
	public:
		objects_storage* const child( ) noexcept;
		const objects_storage* const child( ) const noexcept;

	private:
		objects_storage storage_;
	};

	class object : public texture_renderer, public child_storage, public virtual type_info
	{
	public:
		virtual bool handle_event(event* const ev) noexcept;
	};
}