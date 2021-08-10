#include "settings detail.h"

#include "settings.h"

#include "cheat/gui/tools/info.h"
#include "cheat/gui/tools/push style var.h"

using namespace cheat;
using namespace cheat::detail::settings;
using namespace filesystem;

#if 1
bool known_configs::contains(const string_wrapper& str) const
{
	for (auto& c: data__)
	{
		if (c == str)
			return true;
	}
	return false;
}

ranges::borrowed_subrange_t<known_configs::storage_type&> known_configs::Remove(const span<wstring>& sample)
{
	return ranges::remove_if(data__, [&](const string_wrapper& name)-> bool
	{
		for (const wstring_view s: sample)
		{
			if (wstring_view(name) == s)
				return false;
		}
		return true;
	});
}

void known_configs::Update_longest_string( )
{
	if (data__.empty( ))
		longest_string__ = { };
	else
		longest_string__.init(*ranges::max_element(data__));
}

void known_configs::sync(const span<wstring>& vec)
{
	for (wstring_view s: vec)
	{
		if (data__.end( ) == ranges::find(data__, s, &string_wrapper::raw))
			data__.push_back(string_wrapper::raw_type(move(s)));
	}

	const auto to_remove = Remove(vec);

	data__.erase(to_remove.begin( ), to_remove.end( ));

	Update_longest_string( );
}

const string_wrapper& known_configs::longest_string( ) const
{
	return longest_string__.get( );
}

const known_configs::storage_type& known_configs::get( ) const
{
	return data__;
}

bool known_configs_selectable::select(const string_wrapper& str)
{
	for (size_t i = 0; i < data__.size( ); ++i)
	{
		if (data__[i] == str)
		{
			select(i);
			return true;
		}
	}

	config_selected__ = nullptr;
	return false;
}

void known_configs_selectable::select(size_t index)
{
	auto& target_config = data__[index];
	config_selected__ = addressof(target_config);
}

void known_configs_selectable::deselect( )
{
	config_selected__ = nullptr;
}

string_wrapper* known_configs_selectable::selected( ) const
{
	return config_selected__;
}

bool known_configs_selectable::selected(const string_wrapper& str) const
{
	return config_selected__ ? *config_selected__ == str : false;
}

void folder_with_configs::start_update( )
{
	updated__ = false;
}

void folder_with_configs::end_update( )
{
	updated__ = true;
}

bool folder_with_configs::updated( ) const
{
	return updated__;
}

string_wrapper::value_type folder_with_configs::Label( ) const
{
	return shared->name( );
}

folder_with_configs::folder_with_configs(settings_data* shared_data)
{
	shared = shared_data;
}

void folders_storage::add_folder(value_type&& folder)
{
	const auto& item = data__.emplace_back(move(folder));
	if (const auto name = wstring_view(item.shared->name( )); longest_title__ < name.size( ))
		longest_title__ = name.size( );
}

folders_storage::value_type* folders_storage::get(const wstring_view& str)
{
	for (auto& d: data__)
	{
		if (d.shared->name( ) == str)
		{
			return addressof(d);
		}
	}
	return nullptr;
}

void folders_storage::render( )
{
#if 0
	auto& style = ImGui::GetStyle( );

	constexpr auto dummy_text = string_view("W");
	const auto sample_size = ImGui::CalcTextSize(dummy_text._Unchecked_begin( ), dummy_text._Unchecked_end( ));

	const auto frame_padding = style.FramePadding * 2.f;

	//---

#endif

	//todo: screen size & menu size based
	const auto num_rows = std::min<size_t>(data__.size( ), (2));
	const auto num_columns = data__.size( ) / num_rows;

	BOOST_ASSERT(longest_title__ > 0);
	BOOST_ASSERT(!data__.empty( ));

#if 0
	
	ImVec2 size;
	size.x = frame_padding.x +                            //space before and after
			 /*indent_headers +*/ //to indent first selectable
			 longest_title__ * num_rows * sample_size.x + //reserve width for all strings
			 style.ItemSpacing.x * (num_rows - 1);        //space between all headers

	size.y = frame_padding.y +                        //space before and after
			 num_columns * sample_size.y +            //all strings height						                            
			 style.ItemSpacing.y * (num_columns - 1); //space between all string

	if (!ImGui::BeginChildFrame(reinterpret_cast<ImGuiID>(this), size))
		return ImGui::EndChildFrame( );
		
	const auto max_size = size.x / num_rows;
	for (size_t i = 0; i < data__.size( ); ++i)
	{
		folder_with_configs& ref = data__[i];

		const auto ideal_size = ref.shared->name( ).raw( ).size( ) * sample_size.x;
		const auto extra_add = (max_size - ideal_size) / num_rows;

		memory_backup<ImVec4> header_color_saved;
		(void)header_color_saved;

		if (ref.configs.selected( ))
		{
			auto& header_color = style.Colors[ImGuiCol_Header];
			header_color_saved = memory_backup(header_color, ImVec4(header_color.x, 255.f, header_color.z, header_color.w));
		}

		if (ref(ImGuiSelectableFlags_None, {ideal_size + extra_add, sample_size.y}))
		{
			ref.toggle( );
			//selecteded_before_override = selectable.selected( );
		}

		if ((i + 1) % num_rows)
			ImGui::SameLine( );
	}

	ImGui::EndChildFrame( );

#endif

	if (!this->begin({num_rows, static_cast<float>(longest_title__), size_info::WORD}, {num_columns, static_cast<float>(longest_title__), size_info::WORD}, true))
		return this->end( );
	{
		auto&& sample_size = _Get_char_size( );

		for (size_t i = 0; i < data__.size( ); ++i)
		{
			auto& ref = data__[i];

			memory_backup<ImVec4> header_color_saved;
			(void)header_color_saved;

			if (ref.configs.selected( ))
			{
				auto& header_color = ImGui::GetStyle( ).Colors[ImGuiCol_Header];
				header_color_saved = memory_backup(header_color, ImVec4(header_color.x, 255.f, header_color.z, header_color.w));
			}

			if (ref(ImGuiSelectableFlags_None, {longest_title__ * sample_size.x, sample_size.y}))
			{
				ref.toggle( );
				//selecteded_before_override = selectable.selected( );
			}

			if ((i + 1) % num_rows)
				ImGui::SameLine( );
		}
	}
	this->end( );
}

void folders_storage::select_all( )
{
	for (auto& d: data__)
	{
		if (!d.selected( ))
			d.select( );
	}
}

void folders_storage::deselect_all( )
{
	for (auto& d: data__)
	{
		if (d.selected( ))
			d.deselect( );
	}
}

span<folders_storage::value_type> folders_storage::iterate( )
{
	return {data__.begin( ), data__.end( )};
}

void folders_storage::erase(iterator first, iterator last)
{
	data__.erase(first, last);
}

config_renderer::config_renderer(const string_wrapper& str)
{
	owner__ = str;
}

bool config_renderer::dead(const known_configs& source) const
{
	for (auto& c: source.get( ))
	{
		if (c == owner__)
			return false;
	}
	return true;
}

const string_wrapper& config_renderer::owner( ) const
{
	return owner__;
}

string_wrapper::value_type config_renderer::Label( ) const
{
	return owner__;
}

void configs_unique_renderer::sync(const known_configs& source)
{
	for (const auto& known_data = source.get( ); const auto& kd: known_data)
	{
		auto exists = false;
		for (const auto& d: data__)
		{
			if (d->owner( ) == kd)
			{
				exists = true;
				break;
			}
		}

		if (!exists)
		{
			data__.push_back(utl::make_shared<config_renderer>(kd));
		}
	}

	const auto to_remove = ranges::remove_if(data__, [&](const value_type& val) { return val->dead(source); });
	data__.erase(to_remove.begin( ), to_remove.end( ));

	if (data__.empty( ))
	{
		longest_title__ = 0;
		item_selected__ = { };
		return;
	}

	auto sizes = data__
				 | ranges::views::transform(&config_renderer::owner)
				 | ranges::views::transform(&string_wrapper::raw)
				 | ranges::views::transform(&wstring_view::size);
	longest_title__ = *ranges::max_element(sizes);

	ranges::sort(data__, { }, [](const value_type& val) { return val->owner( ).raw( ); });

	/*if (!item_selected_resolve__)
	{
		if (item_selected__.ptr.expired( ))
			item_selected__ = { };
		return;
	}*/

	if (!item_selected__.index)
	{
		BOOST_ASSERT(item_selected__.ptr.expired( ));
		Select_new_item_(0, true);
	}
	else
	{
		if (!item_selected__.ptr.expired( ))
			return;

		if (const auto idx = *item_selected__.index; idx < data__.size( ))
			Select_new_item_(idx, true); //idx - 1 ???
		else
		{
			//not a goot way, but im too lazy to doing this manually
			vector<wstring_view> temp;
			temp.reserve(data__.size( ) + 1);
			for (const auto& d: data__)
				temp.push_back(d->owner( ));
			temp.push_back(item_selected__.name);

			ranges::sort(temp);

			if (const auto ideal_place = ranges::find(temp, item_selected__.name.data( ), &wstring_view::data); ideal_place == std::prev(temp.end( )))
			{
				//element isnt sorted, still in the end
				Select_new_item_(data__.size( ) - 1, true);
			}
			else
			{
				//choose the same place where element could be located 
				const auto ideal_index = std::distance(temp.begin( ), ideal_place);
				Select_new_item_(ideal_index, true);
			}
		}
	}
}

void configs_unique_renderer::render( )
{
	const auto& style = ImGui::GetStyle( );

	constexpr auto dummy_text = string_view("W");
	const auto sample_size = ImGui::CalcTextSize(dummy_text._Unchecked_begin( ), dummy_text._Unchecked_end( ));

	const auto frame_padding = style.FramePadding * 2.f;

	//----

	static auto empty_string = string_wrapper(_T("<Empty>"));

	// ReSharper disable once CppInconsistentNaming
	static constexpr size_t MIN_COLUMNS = 5;

	//const auto num_columns = data__.size( ) <= MIN_COLUMNS ? MIN_COLUMNS : data__.size( );
	ImVec2 size;
	size.x = frame_padding.x +                                                                          //space before and after
			 sample_size.x * (data__.empty( ) ? wstring_view(empty_string).size( ) : longest_title__) + //reserve width for longest string
			 (data__.size( ) <= MIN_COLUMNS ? 0.f : style.ScrollbarSize);                               //add extra space for scrollbar

	size.y = frame_padding.y +                        //space before and after
			 MIN_COLUMNS * sample_size.y +            //all strings height						                            
			 style.ItemSpacing.y * (MIN_COLUMNS - 1); //space between all string

	if (!ImGui::BeginChildFrame(reinterpret_cast<ImGuiID>(this), size))
		return ImGui::EndChildFrame( );

	switch (data__.size( ))
	{
		case 0:
		{
			ImGui::Selectable(empty_string, false);
			break;
		}
		case 1:
		{
			BOOST_ASSERT(!item_selected__.ptr.expired( ));
			const auto selected = item_selected__.ptr.lock( );
			BOOST_ASSERT(selected->selected( ));

			invoke(*selected);

			break;
		}
		default:
		{
			BOOST_ASSERT(!item_selected__.ptr.expired( ));
			const auto selected = item_selected__.ptr.lock( );
			BOOST_ASSERT(selected->selected( ));

			for (auto& cfg: data__)
			{
				if (invoke(*cfg) && selected != cfg)
				{
					//reanimate_selected_settings(*item_selected__, cfg);
					this->Select_new_item_(cfg, true);
				}
			}
			break;
		}
	}

	ImGui::EndChildFrame( );
}

void configs_unique_renderer::after_sync( )
{
	longest_title__ = 0;
	for (const auto& d: data__)
	{
		if (const auto size = wstring_view(d->owner( )).size( ); longest_title__ < size)
			longest_title__ = size;
	}
}

void configs_unique_renderer::mark_folders_selected(folders_storage& folders) const
{
	if (item_selected__.ptr.expired( ))
	{
		for (auto& f: folders.iterate( ))
			f.configs.deselect( );
	}
	else
	{
		const auto ptr = item_selected__.ptr.lock( );
		const auto& name = ptr->owner( );

		for (auto& f: folders.iterate( ))
		{
			if (const auto selected = f.configs.selected( ); !selected || name != *selected)
				f.configs.select(name);
		}
	}
}

//auto configs_unique_renderer::selected( ) const -> value_type
//{
//	return item_selected__.ptr.lock( );
//}

configs_unique_renderer::value_type::weak_type configs_unique_renderer::selected/*_weak*/( ) const
{
	return item_selected__.ptr;
}

void configs_unique_renderer::select(const string_wrapper& str)
{
	for (size_t i = 0; i < data__.size( ); ++i)
	{
		if (data__[i]->owner( ) == str)
			return Select_new_item_(i, true);
	}

	BOOST_ASSERT("Unable to select element with give name!");
}

//auto configs_unique_renderer::auto_resolve_selected_item(bool enabled) -> void
//{
//	item_selected_resolve__ = enabled;
//}
//
//auto configs_unique_renderer::selected_item_auto_resolved( ) const -> bool
//{
//	return item_selected_resolve__;
//}

void configs_unique_renderer::Select_new_item_(size_t index, bool set_selected)
{
	const auto& item = data__[index];
	if (set_selected)
	{
		if (!item_selected__.ptr.expired( ))
		{
			if (const auto ptr = item_selected__.ptr.lock( ); ptr->selected( ))
				ptr->deselect( );
		}
		if (!item->selected( ))
			item->select( );
	}

	item_selected__.ptr = item;
	item_selected__.name = item->owner( );
	item_selected__.index = index;
}

void configs_unique_renderer::Select_new_item_(const value_type& item, bool set_selected)
{
	for (size_t i = 0; i < data__.size( ); ++i)
	{
		if (data__[i] == item)
			return Select_new_item_(i, set_selected);
	}

	BOOST_ASSERT("Unable to select given element!");
}

void folder_with_configs_mgr::set_work_dir(const path& dir)
{
	auto copy = dir;
	set_work_dir(move(copy));
}

void folder_with_configs_mgr::set_work_dir(path&& dir)
{
	BOOST_ASSERT(dir.is_absolute( ));

	working_dir__ = move(dir);
}

void folder_with_configs_mgr::rescan( )
{
	Process_path_(working_dir__);
}

void folder_with_configs_mgr::render( )
{
	const auto pop = push_style_var(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
	(void)pop;

	ImGui::PushID(reinterpret_cast<ImGuiID>(this));

	//
	//
	//

	folders__.render( );

	ImGui::SameLine( );

	if (ImGui::Button("Select all"))
		folders__.select_all( );
	ImGui::SameLine( );
	if (ImGui::Button("Deselect all"))
		folders__.deselect_all( );

	files_list__.render( );
	files_list__.mark_folders_selected(folders__);

	ImGui::SameLine( );

	ImGui::BeginGroup( );
	{
		Save_( );
		Load_( );
		Save_to_( );
		Remove_( );
	}
	ImGui::EndGroup( );

	//
	//
	//

	ImGui::PopID( );
}

vector<wstring> folder_with_configs_mgr::Process_folder_(const directory_entry& dir)
{
	const auto folder_name = dir.path( ).filename( ).native( );
	const auto folder = folders__.get(folder_name);
	if (!folder)
		return { };

	vector<wstring> files_detected;
	for (auto& file: directory_iterator(dir))
	{
		if (!is_regular_file(file))
			continue;
		const auto file_name_raw = file.path( ).filename( );
		if (!file_name_raw.has_extension( ))
			continue;

		const auto extension = file_name_raw.extension( ).native( );
		if (extension != L".json")
			continue;

		const auto& file_name_native = file_name_raw.native( );
		auto file_name = wstring(file_name_native.begin( ), file_name_native.end( ) - extension.size( ));

		files_detected.push_back(move(file_name));
	}

	BOOST_ASSERT(!folder->updated());
	folder->end_update( );
	folder->configs.sync(files_detected);
	return files_detected;
}

void folder_with_configs_mgr::Process_path_(const path& path)
{
	ranges::for_each(folders__.iterate( ), &folder_with_configs::start_update);

	auto configs_found = utl::vector<wstring>( );
	if (exists(path) && !is_empty(path))
	{
		for (auto& dir: directory_iterator(path))
		{
			auto configs = Process_folder_(dir);
			if (configs.empty( ))
				continue;

			for (auto& c: configs)
			{
				if (configs_found.end( ) == ranges::find(configs_found, (c)))
					configs_found.push_back(move(c));
			}
		}
	}

	for (auto& f: folders__.iterate( ))
	{
		if (!f.updated( ))
			f.configs = { };
	}

	files__.sync(configs_found);
	files_list__.sync(files__);
	files_list__.after_sync( );
}

folder_with_configs_mgr::io_result folder_with_configs_mgr::Do_save_(const string_wrapper& name)
{
	io_result flags;

	for (auto& f: folders__.iterate( ))
	{
		if (f.selected( ))
		{
			if (!flags.has(io_result::rescan_wanted) && !f.configs.contains(name))
				flags.add(io_result::rescan_wanted);

			if (!f.shared->save(name))
				flags.add(io_result::rescan_wanted, io_result::error);
			else
				flags.add(io_result::processed);
		}
	}

	/*if (new_file_added)
		this->rescan( );*/

	return flags;
}

folder_with_configs_mgr::io_result folder_with_configs_mgr::Do_load_(const string_wrapper& name)
{
	io_result flags;

	for (auto& f: folders__.iterate( ))
	{
		if (f.selected( ) && f.configs.selected(name))
		{
			if (!f.shared->load(name))
				flags.add(io_result::rescan_wanted, io_result::error);
			else
				flags.add(io_result::processed);
		}
	}

	return flags;
}

folder_with_configs_mgr::io_result folder_with_configs_mgr::Do_remove_(const string_wrapper& name)
{
	io_result flags;

	for (auto& f: folders__.iterate( ))
	{
		if (f.selected( ) && f.configs.selected(name))
		{
			if (!f.shared->remove(name))
				flags.add(io_result::rescan_wanted, io_result::error);
			else
				flags.add(io_result::rescan_wanted, io_result::processed);
		}
	}

#if 0
	if(!any_removed)
		return 0;

	///do it outside, or weak_ptr never expires
	//this->rescan( );
	return 1;

#endif

	return flags;
}

void folder_with_configs_mgr::Save_( )
{
	if (!ImGui::Button("Save"))
		return;

	const auto selected = files_list__.selected( );
	if (selected.expired( ))
		return;

	if (const auto saved = Do_save_(selected.lock( )->owner( )); saved.has(io_result::rescan_wanted))
	{
		//BOOST_ASSERT_MSG(files_list__.selected_item_auto_resolved( ), "Unable to resolve selected file!");
		this->rescan( );
	}
}

void folder_with_configs_mgr::Load_( )
{
	if (!ImGui::Button("Load"))
		return;

	if (const auto selected = files_list__.selected( ); !selected.expired( ))
	{
		if (const auto loaded = Do_load_(selected.lock( )->owner( )); loaded.has(io_result::rescan_wanted))
			this->rescan( );
	}
}

void folder_with_configs_mgr::Save_to_( )
{
	auto config_name_cleaned = false;

	if (ImGui::Button("Save to"))
	{
		if (!new_config_name__.empty( ))
		{
			const auto config_name = string_wrapper(string(new_config_name__.begin( ), new_config_name__.end( )));

			//const auto resolve_selected_before = files_list__.selected_item_auto_resolved( );
			//files_list__.auto_resolve_selected_item(false);

			if (const auto saved = Do_save_(config_name); saved != io_result::unset)
			{
				if (saved.has(io_result::rescan_wanted))
					this->rescan( );
				if (saved.has(io_result::processed) && !saved.has(io_result::error))
				{
					files_list__.select(config_name);
					new_config_name__.clear( );
					config_name_cleaned = true;
				}
			}

			//files_list__.auto_resolve_selected_item(resolve_selected_before);
		}
	}

	ImGui::SameLine( );

	if (config_name_cleaned)
	{
		static auto unused = string_wrapper("");
		static char unused_buffer[] = {' ', '\0'};

		ImGui::InputText(unused, unused_buffer, 1);
	}
	else if (ImGui::InputText(_CONCAT("##", _STRINGIZE(__LINE__)), addressof(new_config_name__)))
	{
		///todo: close popup by id
		//ImGui::CloseCurrentPopup(Popup_id_( ));
		///todo: valid chars filter, length limit
	}
}

void folder_with_configs_mgr::Remove_( )
{
	if (!ImGui::Button("Remove"))
		return;

	if (const auto selected = files_list__.selected( ); !selected.expired( ))
	{
		if (const auto removed = Do_remove_(selected.lock( )->owner( )); removed.has(io_result::rescan_wanted))
			this->rescan( );
	}
}

void folder_with_configs_mgr::add_folder(folder_with_configs&& folder)
{
	folders__.add_folder(move(folder));
}

#endif

//-----------------

#if 0

using namespace cheat::gui::widgets;
using namespace cheat::gui::widgets::detail;

void settings_data_on_disc::update( )
{
	auto&& dir = data__->path( );

	vector<wstring> files_detected;
	for (auto& file: directory_iterator(dir))
	{
		if (!is_regular_file(file))
			continue;
		const auto file_name_raw = file.path( ).filename( );
		if (!file_name_raw.has_extension( ))
			continue;

		const auto extension = file_name_raw.extension( ).native( );
		if (extension != L".json")
			continue;

		const auto& file_name_native = file_name_raw.native( );
		auto file_name = wstring(file_name_native.begin( ), file_name_native.end( ) - extension.size( ));

		files_detected.push_back(move(file_name));
	}

	files__ = move(files_detected);
}

#endif
