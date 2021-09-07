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

namespace cheat
{
	class settings_shared;

	class settings_mgr
	{
	public:
		class filter
		{
		public:
			filter( );
			~filter( );

			bool   empty( ) const;
			size_t size( ) const;
			bool   contains(const settings_shared* shared) const;
			void   add(const settings_shared* shared);
			bool   remove(const settings_shared* shared);
			void   clear( );

			//for ranges
			auto get( ) const
			{
				return [&]<typename T>(const T& shared)
				{
					if constexpr (std::same_as<T, std::shared_ptr<settings_shared>>)
						return this->contains(shared.get( ));
					else
						return this->contains(shared);
				};
			}

		private:
			struct storage;
			std::unique_ptr<storage> storage_;
		};

		settings_mgr( );
		~settings_mgr( );

		void add(const std::shared_ptr<settings_shared>& shared);

		void save(const std::wstring_view& file_name, const filter& filter_obj = { }) const;
		void load(const std::wstring_view& file_name, const filter& filter_obj = { });

	private:
		struct impl;
		std::unique_ptr<impl> impl_;
	};
}
