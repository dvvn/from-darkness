#pragma once

#include <memory>

namespace std
{
	template <class E, class Tr>
	class basic_string_view;

	template <typename E>
	struct char_traits;

	using wstring_view = basic_string_view<wchar_t, char_traits<wchar_t>>;
}

namespace cheat::settings
{
	class shared_data;

	class manager
	{
	public:
		class filter
		{
		public:
			filter( );
			~filter( );

			bool   empty( ) const;
			size_t size( ) const;
			bool   contains(const shared_data* shared) const;
			void   add(const shared_data* shared);
			bool   remove(const shared_data* shared);
			void   clear( );

			//for ranges
			auto get( ) const
			{
				return [&]<typename T>(const T& obj)
				{
					if constexpr (std::same_as<T, std::shared_ptr<shared_data>>)
						return this->contains(obj.get( ));
					else
						return this->contains(obj);
				};
			}

		private:
			struct storage;
			std::unique_ptr<storage> storage_;
		};

		manager( );
		~manager( );

		void add(const std::shared_ptr<shared_data>& shared);

		void save(const std::wstring_view& file_name, const filter& filter_obj = { }) const;
		void load(const std::wstring_view& file_name, const filter& filter_obj = { });

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};
}
