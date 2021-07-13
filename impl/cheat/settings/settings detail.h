#pragma once
#include "cheat/gui/menu/menu page.h"

namespace cheat
{
	class settings_data;
}

namespace cheat::detail::settings
{
	using namespace gui;

	using imgui::string_wrapper;
	using imgui::string_wrapper_abstract;

	class known_configs
	{
	public:
		friend class known_configs_selectable;

		~known_configs( ) = default;

		bool contains(const string_wrapper& str) const;

	protected:
		using storage_type = utl::vector<string_wrapper>;

		ranges::borrowed_subrange_t<storage_type&> Remove(const utl::span<utl::wstring>& sample);
		void                                       Update_longest_string( );

	public:
		void sync(const utl::span<utl::wstring>& vec);

		const string_wrapper& longest_string( ) const;
		const storage_type&   get( ) const;

	private:
		string_wrapper_abstract longest_string__;
		storage_type            data__;
	};

	class known_configs_selectable final: public known_configs
	{
	public:
		bool select(const string_wrapper& str);
		void select(size_t index);
		void deselect( );

		string_wrapper* selected( ) const;
		bool            selected(const string_wrapper& str) const;

	private:
		//must be manually controlled!
		string_wrapper* config_selected__ = nullptr;
	};

	//folder with configs
	struct folder_with_configs final: imgui::animated_selectable_base
	{
		folder_with_configs(settings_data* shared_data);

		//save/load interface
		settings_data* shared;
		//all configs in current folder
		known_configs_selectable configs;

		void start_update( );
		void end_update( );
		bool updated( ) const;

	protected:
		string_wrapper::value_type Name( ) const override;

	private:
		bool updated__ = false;
	};

	class folders_storage final: public menu::empty_page
	{
	public:
		using value_type = folder_with_configs;
		using storage_type = utl::vector<value_type>;
		using iterator = storage_type::iterator;

		void add_folder(value_type&& folder);

		value_type* get(const utl::wstring_view& str);

		void render( ) override;

		void select_all( );
		void deselect_all( );

		iterator begin( );
		iterator end( );

		[[deprecated]]
		void erase(iterator first, iterator last);

	private:
		storage_type data__;
		size_t       longest_title__ = 0;
	};

	class config_renderer final: public imgui::animated_selectable_base
	{
	public:
		config_renderer(string_wrapper&& str) = delete;
		config_renderer(const string_wrapper& str);

		bool                  dead(const known_configs& source) const;
		const string_wrapper& owner( ) const;

	protected:
		string_wrapper::value_type Name( ) const override;

	private:
		string_wrapper owner__;
	};

	class configs_unique_renderer final: public menu::empty_page
	{
	public:
		using value_type = utl::shared_ptr<config_renderer>;

		void sync(const known_configs& source);
		void after_sync( );

		void render( ) override;

		void mark_folders_selected(folders_storage& folders) const;

		//auto selected( ) const -> value_type;
		value_type::weak_type selected/*_weak*/( ) const;

		void select(const string_wrapper& str);

		//auto auto_resolve_selected_item(bool enabled) -> void;
		//auto selected_item_auto_resolved( ) const -> bool;

	private:
		utl::vector<value_type> data__;
		size_t                  longest_title__ = 0;

		struct
		{
			value_type::weak_type ptr;
			utl::wstring          name;
			utl::optional<size_t> index;
			//-
		} item_selected__;
		//bool item_selected_resolve__ = 1;

		void Select_new_item_(size_t index, bool set_selected);
		void Select_new_item_(const value_type& item, bool set_selected);
	};

	class folder_with_configs_mgr final: public menu::empty_page
	{
	public:
		void set_work_dir(const utl::filesystem::path& dir);
		void set_work_dir(utl::filesystem::path&& dir);

		void rescan( );

		void render( ) override;
		void add_folder(folder_with_configs&& folder);

	private:
		utl::vector<utl::wstring> Process_folder_(const utl::filesystem::directory_entry& dir);
		void                      Process_path_(const utl::filesystem::path& path);

		folders_storage         folders__;
		known_configs           files__;
		configs_unique_renderer files_list__;

		utl::filesystem::path working_dir__;

		enum io_result:size_t
		{
			unset=0,
			processed=1 << 0,
			rescan_wanted=1 << 1,
			error=1 << 2
		};

		using io_flags = utl::bitflag<io_result>;

		_NODISCARD io_flags Do_save_(const string_wrapper& name);
		_NODISCARD io_flags Do_load_(const string_wrapper& name);
		_NODISCARD io_flags Do_remove_(const string_wrapper& name);

		//only works with selected file
		//any external changes to other files ignored
		void Save_( );
		void Load_( );
		//it also consider all external changes
		void Save_to_( );

		utl::string new_config_name__;

		void Remove_( );
	};
}
