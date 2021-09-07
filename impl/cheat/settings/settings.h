#pragma once

#include "cheat/core/service.h"
#include "cheat/gui/objects/abstract page.h"
#include "cheat/gui/tools/string wrapper.h"

#include <nlohmann/json.hpp>

#include <filesystem>

namespace cheat
{
	namespace settings_detail
	{
		class folder_with_configs_mgr;
	}

	class settings_data: public gui::tools::string_wrapper
	{
	public:
		friend class settings;

		using tree_type = nlohmann::json;

		virtual ~settings_data( );

		settings_data(string_wrapper&& name);

		virtual bool save(const std::wstring_view& name) const;
		bool         save(const std::wstring_view& name);
		virtual bool load(const std::wstring_view& name);
		bool         remove(const std::wstring_view& name) const;
		bool         remove_all( ) const;

		virtual void update( ) =0;

		std::wstring_view path( ) const;

	protected:
		std::wstring Generate_path(const std::wstring_view& name) const;

		tree_type             tree_;
		std::filesystem::path path_;
	};

	class settings final: public gui::objects::renderable
						, public settings_data
						, public service<settings>
	{
	public:
		settings( );
		~settings()override;

		void update( ) override;
		void render( ) override;

	protected:
		load_result load_impl( ) override;

	private:
#if 0
		bool merge__ = false;
#endif

		std::unique_ptr<settings_detail::folder_with_configs_mgr> mgr__;
	};
}
