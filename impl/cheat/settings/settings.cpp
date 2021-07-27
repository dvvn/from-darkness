#include "settings.h"

#include "cheat/features/aimbot.h"
#include "cheat/features/anti aim.h"

using namespace cheat;
using namespace gui;
using namespace objects;
using namespace utl;
using namespace filesystem;
using namespace property_tree;

settings_data::settings_data(const string_view& name) : settings_data(wstring(name.begin( ), name.end( )))
{
}

settings_data::settings_data(wstring&& name) : string_wrapper_base(raw_type(move(name)))
{
	path_ = all_modules::get( ).current( ).work_dir( );
	path_ /= L"settings";
	path_ /= this->raw( );
}

bool settings_data::save(const wstring_view& name) const
{
	const auto full_path = filesystem::path(Generate_path(name));
	if (const auto dir = full_path.parent_path( ); !exists(dir) && !create_directories(dir))
		return false;
	auto stream = std::basic_ofstream<wchar_t>(full_path.native( ));
	if (!stream)
		return false;
	write_json(stream, tree_);
	return true;
}

bool settings_data::save(const wstring_view& name)
{
	this->update( );
	return static_cast<const settings_data*>(this)->save(name);
}

bool settings_data::load(const wstring_view& name)
{
	auto stream = std::basic_ifstream<wchar_t>(Generate_path(name));
	if (!stream)
		return false;
	tree_.clear( );
	read_json(stream, tree_);
	this->update( );
	return true;
}

bool settings_data::remove(const wstring_view& name) const
{
	auto path = Generate_path(name);
	return filesystem::remove(path);
}

bool settings_data::remove_all( ) const
{
	return filesystem::remove(path_);
}

utl::wstring_view settings_data::path( ) const
{return path_.native();}

wstring settings_data::Generate_path(const wstring_view& name) const
{
	auto path = const_cast<wstring&&>((path_ / name).native( ));
	path += L".json";
	return path;
}

settings::settings( ) : settings_data("global")
{
}

void settings::update( )
{
#if 0
	Load_or_create("merge", merge__);
#endif
}

void settings::render( )
{
	mgr__.render( );
}

bool settings::Do_load( )
{
	mgr__.add_folder(features::aimbot::get_ptr( ));
	mgr__.add_folder(features::anti_aim::get_ptr( ));

	mgr__.set_work_dir(this->path_.parent_path( ));
	mgr__.rescan( );

	//todo: store all features here
	//load default/last settings or something

	return 1;
}
