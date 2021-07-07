#include "settings.h"

#include "cheat/features/aimbot.h"
#include "cheat/features/anti aim.h"

using namespace cheat;
using namespace gui;
using namespace imgui;
using namespace utl;
using namespace filesystem;
using namespace property_tree;

settings_data::settings_data(const string_view& name) : settings_data(wstring(name.begin( ), name.end( )))
{
}

settings_data::settings_data(wstring&& name) : string_wrapper_base(raw_type(move(name)))
{
	path_ = mem::all_modules::get( ).current( ).work_dir( );
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

wstring settings_data::Generate_path(const wstring_view& name) const
{
	//BOOST_ASSERT(filesystem::path(folder.begin( ), folder.end( )).is_absolute());

	auto path = const_cast<wstring&&>((path_ / name).native( ));
	path += L".json";
	return path;

#if 0
	auto result = path_.native( );

#ifdef _WIN32
	result += '\\';
#else
	result += '/';
#endif
	result += name;
	result += '.';
	result += L"json";
	return result;
#endif
}

settings::settings( ) : settings_data("global")
{
}

#if 0

auto settings::save(const wstring_view& name) const -> void
{
#if 0
	if(!merge__)
	{
#endif
		settings_data::save(name);
		for(auto& other : settings__)
			other.shared->save(name);
#if 0
	}
	else
	{
		auto stream = std::basic_ofstream<wchar_t>(Generate_path(name));

		write_json(stream, tree_);
		for(auto& other : settings__)
			write_json(stream, other.shared->tree_);
	}
#endif
}

auto settings::load(const wstring_view& name) -> void
{
	auto stream = std::basic_ifstream<wchar_t>(Generate_path(name));

	tree_type all;
	read_json(stream, all);

	const auto fill_settings = [&all](settings_data* to)
	{
		try
		{
			//dont want to fuck with find()
			to->tree_ = move(all.get_child(to->raw));
			to->update( );
			return true;
		}
		catch(const ptree_bad_path&)
		{
			return false;
		}
	};

	const auto this_filled = fill_settings(this);
	BOOST_ASSERT(this_filled);
#if 0
	if(!merge__)
	{
#endif
		BOOST_ASSERT(all.size( ) == 1);
		for(auto other : settings__)
			other.shared->load(name);
#if 0
	}
	else
	{
		BOOST_ASSERT(all.size( ) > 1);
		for(auto other : settings__)
			fill_settings(other.shared);
	}
#endif
}

#endif

//---

void settings::update( )
{
#if 0
	Load_or_create("merge", merge__);
#endif

#if 0

	Load_or_create("override", override__);
	Load_or_create("select all", select_all__);

#endif
}

void settings::render( )
{
#if 0
	//ImGui::BeginTable ???

	auto& style = ImGui::GetStyle( );

	constexpr auto dummy_text = string_view("W");
	const auto     sample_size = ImGui::CalcTextSize(dummy_text._Unchecked_begin( ), dummy_text._Unchecked_end( ));

	const auto frame_padding = style.FramePadding * 2.f;

	const auto pop = push_style_var(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));
	(void)pop;

	{
		//todo: screen size & menu size based
		const auto num_rows = std::min(settings__.size( ), static_cast<size_t>(2));
		const auto num_columns = settings__.size( ) / num_rows;

		ImVec2 size;
		size.x = frame_padding.x +                                     //space before and after
				 /*indent_headers +*/ //to indent first selectable
				 longest_setting_string__ * num_rows * sample_size.x + //reserve width for all strings
				 style.ItemSpacing.x * (num_rows - 1);                 //space between all headers

		size.y = frame_padding.y +                        //space before and after
				 num_columns * sample_size.y +            //all strings height						                            
				 style.ItemSpacing.y * (num_columns - 1); //space between all string

		if (!ImGui::BeginChildFrame(Settings_list_id_( ), size))
			return ImGui::EndChildFrame( );

		const auto max_size = size.x / num_rows;
		for (auto i = 0; i < settings__.size( ); ++i)
		{
			auto& ref = settings__[i];

			const auto ideal_size = ref.shared->name( ).raw( ).size( ) * sample_size.x;
			const auto extra_add = (max_size - ideal_size) / num_rows;

			mem::memory_backup<ImVec4> header_color_saved;
			(void)header_color_saved;
			if (config_selected__ != nullptr && ref.is_saved(*config_selected__))
			{
				auto& header_color = style.Colors[ImGuiCol_Header];
				header_color_saved = mem::memory_backup(header_color, ImVec4{header_color.x, 255.f, header_color.z, header_color.w});
			}
			const auto pressed = ref(/*select_all__? ImGuiSelectableFlags_Disabled :*/ImGuiSelectableFlags_None, //todo: add animations for disabled text
																					  {ideal_size + extra_add, sample_size.y});

			if (pressed /*&& !override__*/)
			{
				ref.toggle( );
				//selecteded_before_override = selectable.selected( );
			}

			if ((i + 1) % num_rows)
				ImGui::SameLine( );
		}

		ImGui::EndChildFrame( );
	}

	ImGui::SameLine( );

	static auto empty_string = string_wrapper(_T("<Empty>"));

	const auto get_selected_data = [&]
	{
		return settings__ | ranges::views::filter([](settings_data_internal& di)
		{
			return di.selected( ) == true;
		});
	};
	const auto get_saved_selected_data = [&](const string_wrapper& name)
	{
		return settings__ | ranges::views::filter([&](settings_data_internal& di)
		{
			return di.selected( ) == true && di.is_saved(name);
		});
	};
	const auto get_deselected_data = [&]
	{
		return settings__ | ranges::views::filter([](settings_data_internal& di)
		{
			return di.selected( ) == false;
		});
	};

	const auto reanimate_selected_settings_force = [&](const optional<string_wrapper&> selected_before = { })
	{
		for (auto& s: settings__)
		{
			if (!s.selected( ) || s.animating( ))
				continue;

			if (!selected_before.has_value( ) || ranges::find(s.configs, *selected_before) != s.configs.end( ))
				s.select( );
		};
	};
	const auto reanimate_selected_settings = [&](const string_wrapper& selected_before, const string_wrapper& selected_now)
	{
		for (auto& s: settings__)
		{
			if (!s.selected( ) || s.animating( ))
				continue;

			const auto before = ranges::find(s.configs, selected_before) != s.configs.end( );
			const auto now = ranges::find(s.configs, selected_now) != s.configs.end( );

			if (before != now)
				s.select( ); //re-select it
		}
	};
	const auto update_configs_list = [&]
	{
		size_t elments_added = 0;
		size_t elements_removed = 0;

		decltype(configs__) global_configs_temp;

		for (auto& dir: directory_iterator(path_.parent_path( )))
		{
			if (is_empty(dir))
				continue;

			auto folder_name = dir.path( ).filename( ).native( );

			auto found = ranges::find(settings__, wstring_view(folder_name), [](settings_data_internal& di) { return di.shared->name( ); });
			if (found == settings__.end( ))
				continue;

			//auto& entry = *found;
			//entry.configs.clear( );

			decltype(settings_data_internal::configs) local_configs_temp;
			for (auto& file: directory_iterator(dir))
			{
				if (!is_regular_file(file))
					continue;
				auto file_name_raw = file.path( ).filename( );
				if (!file_name_raw.has_extension( ))
					continue;

				auto extension = file_name_raw.extension( ).native( );
				if (extension != L".json")
					continue;

				auto& file_name_native = file_name_raw.native( );
				auto  file_name = wstring(file_name_native.begin( ), file_name_native.end( ) - extension.size( ));

				// add file name to temp local storage
				auto& cfg = local_configs_temp.emplace_back(saved_config(string_wrapper(move(file_name)), true));

				//try add file name to global storage
				if (global_configs_temp.end( ) == ranges::find(global_configs_temp, static_cast<string_wrapper&>(cfg)))
				{
					global_configs_temp.push_back(saved_configs_data_internal(cfg));
				}
			}

			auto& local_configs = found->configs;

			//remove deleted elements
			const auto elements_removed_local = ranges::remove_if(local_configs, [&](const wstring_view& name)
			{
				return (local_configs_temp.end( ) == ranges::find(local_configs_temp, name));
			}).size( );
			local_configs.erase(local_configs.end( ) - elements_removed_local, local_configs.end( ));

			size_t elments_added_local = 0;
			//add new elements
			for (auto& name_stored: local_configs_temp)
			{
				if (local_configs.end( ) == ranges::find(local_configs, (name_stored)))
				{
					local_configs.push_back(move(name_stored));
					++elments_added_local;
				}
			}

			elments_added += elments_added_local;
			elements_removed += elements_removed_local;
		}

		struct last_config_selected_t
		{
			size_t         index;
			string_wrapper name;
		};

		optional<last_config_selected_t> last_config_selected;
		if (config_selected__ != nullptr)
		{
			BOOST_ASSERT(!configs__.empty( ));

			last_config_selected_t data;
			data.index = std::distance(configs__._Unchecked_begin( ), config_selected__);
			data.name = *static_cast<string_wrapper*>(config_selected__);

			last_config_selected.emplace(move(data));
		}

		//remove deleted elements
		const auto elements_removed_global = ranges::remove_if(configs__, [&](const wstring_view& name)
		{
			return (global_configs_temp.end( ) == ranges::find(global_configs_temp, name));
		}).size( );
		configs__.erase(configs__.end( ) - elements_removed_global, configs__.end( ));

		size_t elments_added_global = 0;
		//add new elements
		for (auto& name_stored: global_configs_temp)
		{
			if (configs__.end( ) == ranges::find(configs__, static_cast<string_wrapper&>(name_stored)))
			{
				configs__.push_back(move(name_stored));
				++elments_added_global;
			}
		}

		if (configs__.empty( ))
		{
			config_selected__ = nullptr;
			longest_config_string__ = empty_string.raw( ).size( );

			if (elements_removed_global > 0 && last_config_selected.has_value( ))
				reanimate_selected_settings_force(last_config_selected->name);
			else
				reanimate_selected_settings_force( );
		}
		else if (elments_added != 0 || elements_removed != 0)
		{
			if (elments_added_global != 0 || elements_removed_global != 0)
			{
				///warning: probably not good for unicode
				ranges::sort(configs__, { }, &string_wrapper::raw);
			}

			if (!last_config_selected)
			{
				config_selected__ = (configs__._Unchecked_begin( ));
				reanimate_selected_settings_force(*config_selected__);
			}
			else if (last_config_selected->index >= configs__.size( ))
			{
				config_selected__ = (configs__._Unchecked_end( ) - 1);
				reanimate_selected_settings(last_config_selected->name, *config_selected__);
			}
			else
			{
				auto new_selected_config_index = std::distance(configs__._Unchecked_begin( ), config_selected__);
				if (new_selected_config_index < last_config_selected->index)
				{
					configs__[last_config_selected->index].deselect( );
				}
				else
				{
					//pointer see at the same element (already selected)
				}
			}

			if (!config_selected__->selected( ))
				config_selected__->select( );

			auto sizes = configs__ |
						 ranges::views::transform(&string_wrapper::raw) |
						 ranges::views::transform(&wstring_view::size);
			longest_config_string__ = *ranges::max_element(sizes);
		}
	};

	if (ImGui::Button("Select all"))
	{
		for (auto& d: get_deselected_data( ))
			d.select( );
	}
	ImGui::SameLine( );
	if (ImGui::Button("Deselect all"))
	{
		for (auto& d: get_selected_data( ))
			d.deselect( );
	}

	if (ImGui::Button("Update files list") || !configs_list_updated__)
	{
		configs_list_updated__ = true;
		update_configs_list( );
	}

	{
		// ReSharper disable once CppInconsistentNaming
		static constexpr size_t MIN_COLUMNS = 5;

		//const auto num_columns = configs__.size( ) <= MIN_COLUMNS ? MIN_COLUMNS : configs__.size( );
		ImVec2 size;
		size.x = frame_padding.x +                                               //space before and after
				 longest_config_string__ * sample_size.x +                       //reserve width for longest string
				 (configs__.size( ) <= MIN_COLUMNS ? 0.f : style.ScrollbarSize); //add extra space for scrollbar

		size.y = frame_padding.y +                        //space before and after
				 MIN_COLUMNS * sample_size.y +            //all strings height						                            
				 style.ItemSpacing.y * (MIN_COLUMNS - 1); //space between all string

		if (!ImGui::BeginChildFrame(Configs_list_id_( ), size))
			return ImGui::EndChildFrame( );

		switch (configs__.size( ))
		{
			case 0:
			{
				ImGui::Selectable(empty_string, false);
				break;
			}
			case 1:
			{
				config_selected__ = addressof(configs__[0]);
				auto& selectable = *static_cast<animated_selectable*>(config_selected__);
				if (selectable(*config_selected__) && !selectable.selected( ))
				{
					selectable.select( );
					//reanimate_selected_settings( );
				}
				break;
			}
			default:
			{
				for (auto& cfg: configs__)
				{
					const auto cfg_ptr = addressof(cfg);
					if (cfg( ) && config_selected__ != cfg_ptr)
					{
						reanimate_selected_settings(*config_selected__, cfg);

						config_selected__->deselect( );
						config_selected__ = cfg_ptr;
						cfg.select( );
					}
				}
				break;
			}
		}

		ImGui::EndChildFrame( );
	}

	ImGui::SameLine( );

	const auto do_save = [&](const string_wrapper& name)
	{
		auto data = get_selected_data( );
		if (data.empty( ))
			return false;

		for (auto& d: data)
			d.shared->save(name);

		for (auto& d: data)
		{
			if (!d.is_saved(name))
			{
				update_configs_list( );
				break;
			}
		}

		return true;
	};
	const auto do_load = [&](const string_wrapper& name)
	{
		auto data = get_saved_selected_data(name);
		for (auto& d: data)
		{
			d.shared->load(name);
		}
		return !data.empty( );
	};
	const auto do_remove = [&](const string_wrapper& name)
	{
#if 0
		if (get_selected_data( ).empty( ))
		{
			//remove all configs with name
			BOOST_ASSERT(!settings__.empty());
			for (auto& d: settings__)
				d.shared->remove(name);
			return true;
		}
#endif
		//remove all selected configs with name
		auto data = get_saved_selected_data(name);
		for (auto& d: data)
			d.shared->remove(name);
		return !data.empty( );
	};

	ImGui::BeginGroup( );
	{
		if (ImGui::Button("Save"))
		{
			if (!config_selected__)
				Open_popup_(L"No selected config!");
			else if (!do_save(*config_selected__))
				Open_popup_(L"Nothing to save!");
		}
		if (ImGui::Button("Save to"))
		{
			if (new_config_name__.empty( )) //todo: check for valid symbols here. or use filter while input
				Open_popup_(L"Incorrent file name!");
			else if (configs__.end( ) != ranges::find(configs__, new_config_name__, &string_wrapper::multibyte))
				Open_popup_(L"Name already exists");
			else
			{
				auto temp_name = string_wrapper(new_config_name__);
				if (!do_save(temp_name))
					Open_popup_(L"Nothing to save!");
				else
				{
					BOOST_ASSERT(ranges::find(configs__, temp_name) != configs__.end( ));
					new_config_name__.clear( );
				}
			}
		}
		ImGui::SameLine( );
		if (ImGui::InputText(new_config_input_id__, addressof(new_config_name__), ImGuiInputTextFlags_None))
		{
			///todo: close popup by id
			//ImGui::CloseCurrentPopup(Popup_id_( ));
			///todo: valid chars filter, length limit
		}
		if (ImGui::Button("Load"))
		{
			if (!config_selected__)
				Open_popup_(L"No selected config!");
			else if (!do_load(*config_selected__))
				Open_popup_(L"Nothing to load!");
		}
		if (ImGui::Button("Remove") && config_selected__ != 0)
		{
			if (!config_selected__)
				Open_popup_(L"No selected config!");
			else if (!do_remove(*config_selected__))
				Open_popup_(L"Nothing to remove!");
			else
			{
				update_configs_list( );
			}
		}
	}
	ImGui::EndGroup( );

	constexpr auto begin_popup_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings;
	if (ImGui::BeginPopupEx(Popup_id_( ), begin_popup_flags))
	{
		ImGui::Text(popup_message__);
		ImGui::EndPopup( );
	}
#endif

	mgr__.render( );
}

void settings::Load( )
{
	//settings__[L"aimbot"] = features::aimbot::get_ptr( );

	mgr__.add_folder(features::aimbot::get_ptr( ));
	mgr__.add_folder(features::anti_aim::get_ptr( ));

	mgr__.set_work_dir(this->path_.parent_path( ));
	mgr__.rescan( );

	/*auto sizes = settings__ | ranges::views::transform([](settings_data_internal& data)
	{
		return data.shared->name( ).raw( ).size( );
	});

	longest_setting_string__ = *ranges::max_element(sizes);*/

	//todo: store all features here

	//load default/last settings or something
}

//auto settings::Settings_list_id_( ) const -> ImGuiID
//{
//	return reinterpret_cast<ImGuiID>(this);
//}
//
//auto settings::Configs_list_id_( ) const -> ImGuiID
//{
//	return reinterpret_cast<ImGuiID>(this) + 1;
//}
//
//auto settings::Popup_id_( ) const -> ImGuiID
//{
//	return reinterpret_cast<ImGuiID>(this) + 2;
//}

//auto settings::Open_popup_(string_wrapper&& message) -> void
//{
//	popup_message__ = move(message);
//	ImGui::OpenPopupEx(Popup_id_( ));
//}

#if 0

settings::settings_data_internal::settings_data_internal(settings_data* shared_data) : shared(shared_data)
{
}

auto settings::settings_data_internal::is_saved(const string_wrapper& str) const -> bool
{
	for (auto& c: configs)
	{
		if (c == str)
			return c.saved;
	}

	return 0;
}

auto settings::settings_data_internal::set_saved(const string_wrapper& str, bool saved) -> bool
{
	for (auto& c: configs)
	{
		if (c == str)
		{
			c.saved = saved;
			return 1;
		}
	}

	return 0;
}

auto settings::settings_data_internal::Name( ) const -> string_wrapper::value_type
{
	return shared->name( );
}

settings::saved_configs_data_internal::saved_configs_data_internal(saved_config&& cfg) : saved_config(utl::move(cfg))
{
}

settings::saved_configs_data_internal::saved_configs_data_internal(const saved_config& cfg) : saved_config((cfg))
{
}

auto settings::saved_configs_data_internal::Name( ) const -> string_wrapper::value_type
{
	return *static_cast<const saved_config*>(this);
}
#endif
