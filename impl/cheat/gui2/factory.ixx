module;

#include <concepts>
#include <memory>
#ifdef _DEBUG
#include <stdexcept>
#endif

export module cheat.gui2.factory;

export namespace cheat::gui2
{
	template<class T>
	using allocator_t =
#ifdef CHEAT_GUI2_ALLOCATOR
		CHEAT_GUI2_ALLOCATOR
#else
		std::allocator
#endif
		<T>;

	template<class T>
	class simple_deleter
	{
	public:
		void operator()(T* const ptr) const
		{
			allocator_t<T> alloc;
			alloc.deallocate(ptr, 1);
		}
	};

	template<class T>
	class abstract_deleter
	{
		size_t obj_size_;

	public:
		abstract_deleter(const size_t obj_size)
			:obj_size_(obj_size)
		{
		}

		void operator()(T* const ptr) const
		{
			std::destroy_at(ptr);
			allocator_t<uint8_t> alloc;
			alloc.deallocate(reinterpret_cast<uint8_t*>(ptr), obj_size_);
		}
	};

	template<class T>
	using unique_pointer = std::unique_ptr<T, std::conditional_t<std::is_abstract_v<T>, abstract_deleter<T>, simple_deleter<T>>>;

	template<class T>
	class unique_factory
	{
	public:
		template <std::derived_from<T> T1 = T, typename ...Args>
		static auto _Create_unique(Args&&...args)
		{
			allocator_t<T1> alloc;
			auto ptr = alloc.allocate(1);

			std::construct_at(ptr, std::forward<Args>(args)...);

			if constexpr(std::is_abstract_v<T>)
				return std::unique_ptr<T, abstract_deleter<T>>(ptr, abstract_deleter<T>(sizeof(T1)));
			else
				return std::unique_ptr<T, simple_deleter<T>>(ptr);

		}
	};

	template<class T>
	using shared_pointer = std::shared_ptr<T>;

	template<class T>
	struct shared_factory : unique_factory<T>
	{
		template <class T1 = T, typename ...Args>
		static shared_pointer<T> _Create_shared(Args&&...args)
		{
			//force it to use simple deleter
			return unique_factory<T1>::_Create_unique(std::forward<Args>(args)...);
		}
	};
}