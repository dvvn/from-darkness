#pragma once

#include "settings detail.h"
#include "cheat/core/service.h"

namespace cheat
{
	class settings_data: protected gui::tools::string_wrapper_base
	{
	public:
		friend class settings;

		using tree_type = utl::property_tree::wiptree;

		virtual ~settings_data( ) = default;

		settings_data(const utl::string_view& name);
		settings_data(utl::wstring&& name);

		virtual bool save(const utl::wstring_view& name) const;
		bool save(const utl::wstring_view& name);
		virtual bool load(const utl::wstring_view& name);
		bool remove(const utl::wstring_view& name) const;
		bool remove_all( ) const;

		virtual void update( ) =0;

		using string_wrapper_base::name;

	protected:
		template <typename T>
		void Load_or_create(const utl::string_view& name, T& value)
		{
			Load_or_create<T>(utl::wstring(name.begin( ), name.end( )), value);
		}

		template <typename T>
		void Load_or_create(const utl::property_tree::wpath& name, T& value)
		{
			if (const auto child = tree_.get_child_optional(name); !child)
				tree_.put(name, value);
			else
				value = child->get_value<T>( );
		}

		utl::wstring Generate_path(const utl::wstring_view& name) const;

		tree_type tree_;
		utl::filesystem::path path_;
	};

	class settings final: public gui::objects::empty_page, public settings_data,
						  public service_shared<settings, service_mode::async>
	{
	public:
		settings( );

#if 0
		auto save(const utl::wstring_view& name) const -> void override;
		auto load(const utl::wstring_view& name) -> void override;
#endif

		void update( ) override;
		void render( ) override;

	protected:
		void Load( ) override;

	private:
#if 0
		bool merge__ = false;
#endif
#if 0
		bool override__ = 0;
		bool override_reset__ = 0;

		bool select_all__ = 0;
#endif

		/*auto Settings_list_id_( ) const -> ImGuiID;
		auto Configs_list_id_( ) const -> ImGuiID;
		auto Popup_id_( ) const -> ImGuiID;*/

		//string_wrapper popup_message__;
		//auto           Open_popup_(string_wrapper&& message) -> void;

		detail::settings::folder_with_configs_mgr mgr__;

#if 0
		//settings_data*                        selected__ = nullptr;
		utl::vector<folder_with_configs> settings__;
		size_t                           longest_setting_string__ = 0;

		/*struct saved_configs_data_internal
		{
			gui::imgui::string_wrapper_abstract name;
			gui::imgui::animated_selectable     selectable;
		};*/

		//string_wrapper_abstract

		struct global_config_file: string_wrapper, gui::imgui::animated_selectable_base
		{
			global_config_file(string_wrapper&& cfg);
			global_config_file(const string_wrapper& cfg);

		protected:
			auto Name( ) const -> string_wrapper::value_type override;
		};


		
		utl::vector<config_file_selector> configs__;
		config_file_selector*             config_selected__;
		bool                              configs_list_updated__ = false;
		size_t                            longest_config_string__ = 0;

#endif
	};
}
