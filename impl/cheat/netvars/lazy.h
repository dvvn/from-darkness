#pragma once
#include <filesystem>

namespace cheat::detail::netvars
{
	class lazy_file_writer final : public std::ostringstream
	{
	public:
		~lazy_file_writer( ) override;

		lazy_file_writer(std::filesystem::path&& file);
		lazy_file_writer(lazy_file_writer&& other) noexcept;
		lazy_file_writer& operator=(lazy_file_writer&& other) noexcept;

	private:
		std::filesystem::path file_;
	};

	class lazy_fs_creator final
	{
	public:
		~lazy_fs_creator( );

		lazy_fs_creator(std::filesystem::path&& source) noexcept
			: path_(std::move(source))
		{
		}

		lazy_fs_creator(const std::filesystem::path& source) noexcept
			: path_(source)
		{
		}

	private:
		std::filesystem::path path_;
	};

	class lazy_fs_remover final
	{
	public:
		~lazy_fs_remover( );

		lazy_fs_remover(std::filesystem::path&& source, bool all) noexcept
			: path_(std::move(source)), all_(all)
		{
		}

		lazy_fs_remover(const std::filesystem::path& source, bool all) noexcept
			: path_(source), all_(all)
		{
		}

	private:
		std::filesystem::path path_;
		bool all_;
	};

	struct lazy_files_storage
	{
		std::vector<lazy_file_writer> write;
		std::vector<lazy_fs_creator> create;
		std::vector<lazy_fs_remover> remove;
	};
}
