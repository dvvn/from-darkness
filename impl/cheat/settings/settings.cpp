#include "settings.h"
#include "settings detail.h"

#include "cheat/features/aimbot.h"
#include "cheat/features/anti aim.h"

#include <nstd/os/module info.h>

#include <WS2tcpip.h>
#include <ShlObj.h>

#include <fstream>
using namespace cheat;
using namespace gui;
using namespace objects;

settings_data::~settings_data( ) = default;

settings_data::settings_data(string_wrapper&& name)
	: string_wrapper(std::move(name))
{
	path_ = []( )-> decltype(auto)
	{
#ifdef CHEAT_GUI_TEST
		return nstd::os::all_modules::get_ptr( )->current( ).work_dir( );
#else
		static const auto path = []
		{
			PWSTR      buffer = nullptr;
			const auto hr     = SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DEFAULT, nullptr, std::addressof(buffer));
			runtime_assert(SUCCEEDED(hr));
			auto ret = std::filesystem::path(buffer);
			CoTaskMemFree(buffer);
			return ret;
		}( );
		return path;

#endif
	}( );
	path_ /= L"settings";
	path_ /= this->raw( );
}

bool settings_data::save(const std::wstring_view& name) const
{
	const auto full_path = std::filesystem::path(Generate_path(name));
	if (const auto dir = full_path.parent_path( ); !exists(dir) && !create_directories(dir))
		return false;
	auto stream = std::ofstream(full_path);
	if (!stream)
		return false;

	stream << tree_;
	return true;
}

bool settings_data::save(const std::wstring_view& name)
{
	this->update( );
	return static_cast<const settings_data*>(this)->save(name);
}

bool settings_data::load(const std::wstring_view& name)
{
	auto stream = std::ifstream(Generate_path(name));
	if (!stream)
		return false;
	tree_.clear( );
	(stream >> tree_);
	this->update( );
	return true;
}

bool settings_data::remove(const std::wstring_view& name) const
{
	const auto path = Generate_path(name);
	return std::filesystem::remove(path);
}

bool settings_data::remove_all( ) const
{
	return std::filesystem::remove(path_);
}

std::wstring_view settings_data::path( ) const
{
	return path_.native( );
}

std::wstring settings_data::Generate_path(const std::wstring_view& name) const
{
	auto path = const_cast<std::wstring&&>((path_ / name).native( ));
	path += L".json";
	return path;
}

settings::settings( )
	: settings_data("global")
{
	mgr__ = std::make_unique<settings_detail::folder_with_configs_mgr>( );
}

settings::~settings( ) = default;

void settings::update( )
{
#if 0
	Load_or_create("merge", merge__);
#endif
}

void settings::render( )
{
	mgr__->render( );
}

service_impl::load_result settings::load_impl( )
{
	mgr__->add_folder(features::aimbot::get_ptr( ));
	mgr__->add_folder(features::anti_aim::get_ptr( ));

	mgr__->set_work_dir(this->path_.parent_path( ));
	mgr__->rescan( );

	//todo: store all features here
	//load default/last settings or something

	co_return service_state::loaded;
}
