module;

#include <fstream>
#include <filesystem>

module cheat.netvars:lazy;

using namespace cheat::netvars_impl::lazy;

file_writer::~file_writer( )
{
	if (file_.empty( ))
		return;

	const auto str = this->view( );
	if (str.empty( ))
		return;

	auto ofs = std::ofstream(file_);
	if (!ofs)
		return;

	ofs << str;
}

file_writer::file_writer(std::filesystem::path&& file)
	: file_(std::move(file))
{
}

file_writer::file_writer(file_writer&& other) noexcept
{
	*this = std::move(other);
}

file_writer& file_writer::operator=(file_writer&& other) noexcept
{
	file_ = std::move(other.file_);
	*static_cast<std::ostringstream*>(this) = static_cast<std::ostringstream&&>(other);
	return *this;
}

fs_creator::~fs_creator( )
{
	if (path_.empty( ))
		return;

	create_directories(path_);
}

fs_remover::~fs_remover( )
{
	if (path_.empty( ))
		return;

	try
	{
		if (all_)
			remove_all(path_);
		else
			remove(path_);
	}
	catch (...)
	{
	}
}
