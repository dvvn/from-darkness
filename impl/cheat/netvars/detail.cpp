#include "detail.h"

#include <fstream>

using namespace cheat::detail::netvars;

lazy_file_writer::~lazy_file_writer( )
{
	if (file_.empty( ))
		return;

	const auto str = this->view( );
	if (str.empty( ))
		return;

	std::ofstream ofs(file_);
	if (!ofs)
		return;

	ofs << str;
}

lazy_file_writer::lazy_file_writer(std::filesystem::path&& file)
	: file_(std::move(file))
{
}

lazy_file_writer::lazy_file_writer(lazy_file_writer&& other) noexcept
{
	*this = std::move(other);
}

lazy_file_writer& lazy_file_writer::operator=(lazy_file_writer&& other) noexcept
{
	std::swap(file_, other.file_);
	std::swap<std::ostringstream>(*this, other);
	return *this;
}

lazy_fs_creator::~lazy_fs_creator( )
{
	if (this->empty( ))
		return;

	create_directories(*this);
}

lazy_fs_remover::~lazy_fs_remover( )
{
	if (this->empty( ))
		return;

	if (!all_)
		remove(*this);
	else
		remove_all(*this);
}
