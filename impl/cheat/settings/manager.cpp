#include "manager.h"
#include "shared_data.h"

#include "nstd/runtime assert.h"

#ifdef CHEAT_GUI_TEST
#include "nstd/os/module info.h"
#else
#include <shlobj_core.h>
#endif

#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

using namespace cheat::settings;

struct manager::filter::storage: std::vector<const shared_data*>
{
};

manager::filter::filter( )  = default;
manager::filter::~filter( ) = default;

bool manager::filter::empty( ) const
{
	return storage_ == nullptr || storage_->empty( );
}

size_t manager::filter::size( ) const
{
	return storage_ == nullptr ? 0 : storage_->size( );
}

template <typename T, std::convertible_to<T> P>
static bool _Contains(const std::vector<T>& vec, P ptr)
{
	return std::ranges::find(vec, ptr) != vec.end( );
}

bool manager::filter::contains(const shared_data* shared) const
{
	if (this->empty( ))
		return false;
	return _Contains(*storage_, shared);
}

void manager::filter::add(const shared_data* shared)
{
	if (!storage_)
		storage_ = std::make_unique<storage>( );
	runtime_assert(!this->contains(shared));
	storage_->push_back(shared);
}

bool manager::filter::remove(const shared_data* shared)
{
	switch (this->size( ))
	{
		case 0:
			break;
		case 1:
		{
			if (_Contains(*storage_, shared))
			{
				this->clear( );
				return true;
			}
			break;
		}
		default:
		{
			const auto it = std::ranges::find(*storage_, shared);
			if (it != storage_->end( ))
			{
				std::iter_swap(it, std::prev(storage_->end( )));
				storage_->pop_back( );
				return true;
			}
			break;
		}
	}

	return false;
}

void manager::filter::clear( )
{
	storage_.reset( );
}

struct manager::impl
{
	std::filesystem::path path;

	impl( )
	{
		path = []( )-> std::filesystem::path
		{
#ifdef CHEAT_GUI_TEST
			return NSTD_STRINGIZE_RAW(VS_SolutionDir) NSTD_STRINGIZE_RAW(\.out\);
			//return nstd::os::all_modules::get_ptr( )->current( ).work_dir( );
#else
			static const auto path = []
			{
				PWSTR      buffer = nullptr;
				const auto hr     = SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DEFAULT, nullptr, std::addressof(buffer));
				runtime_assert(SUCCEEDED(hr));
				auto ret = std::filesystem::path(buffer);
				CoTaskMemFree(buffer);
				return ret /= _STRINGIZE(VS_SolutionName);
			}( );
			return path;

#endif
		}( ) /= "settings";
	}

	std::vector<std::shared_ptr<shared_data>> data;
};

manager::manager( )
{
	impl_ = std::make_unique<impl>( );
}

manager::~manager( ) = default;

// ReSharper disable once CppMemberFunctionMayBeConst
void manager::add(const std::shared_ptr<shared_data>& shared)
{
	impl_->data.push_back(shared);
}

static std::filesystem::path _Generate_path(const std::filesystem::path& folder, const std::wstring_view& file_name)
{
	runtime_assert(!file_name.empty( ));

	constexpr std::wstring_view file_extension = L".json";

	auto full_path = folder / file_name;
	if (!file_name.ends_with(file_extension))
		full_path += file_extension;
	return full_path;
}

void manager::save(const std::wstring_view& file_name, const filter& filter_obj) const
{
	const auto& path = impl_->path;
	if (!exists(path))
	{
		[[maybe_unused]] const auto create = create_directories(path);
		runtime_assert(create);
	}
	else
	{
		runtime_assert(is_directory(path));
		//using perms = std::filesystem::perms;
		//runtime_assert((status(path).permissions( ) & perms::_All_write) != perms::none);
	}

	const auto full_path = _Generate_path(path, file_name);
	auto       json      = shared_data::json( );
	if (!filter_obj.empty( ) && exists(full_path) && is_regular_file(full_path))
	{
		//backup all other settings from this file
		auto stream = std::ifstream(full_path);
		if (!stream.fail( ))
			stream >> json;
	}

	for (auto&& d: impl_->data | std::views::filter(filter_obj.get( )))
		d->save(json);

	auto stream = std::ofstream(full_path);
	runtime_assert(!stream.fail( ));

	stream << json;
}

void manager::load(const std::wstring_view& file_name, const filter& filter_obj)
{
	(void)this;

	const auto& path = impl_->path;
	runtime_assert(exists(path));
	runtime_assert(is_directory(path));

	auto       json      = shared_data::json( );
	const auto full_path = _Generate_path(path, file_name);

	auto stream = std::ifstream(full_path);
	runtime_assert(!stream.fail( ));
	stream >> json;

	for (auto&& d: impl_->data | std::views::filter(filter_obj.get( )))
		d->load(json);
}
