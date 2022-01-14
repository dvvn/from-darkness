module;
#include <filesystem>

export module cheat.netvars:lazy;

export namespace cheat::netvars_impl::lazy
{
	class file_writer final : public std::ostringstream
	{
	public:
		~file_writer( ) override;

		file_writer( ) = default;

		file_writer(std::filesystem::path&& file);
		file_writer(file_writer&& other) noexcept;
		file_writer& operator=(file_writer&& other) noexcept;

	private:
		std::filesystem::path file_;
	};

	class fs_creator final
	{
	public:
		~fs_creator( );

		fs_creator(std::filesystem::path&& source) noexcept
			: path_(std::move(source))
		{
		}

		fs_creator(const std::filesystem::path& source) noexcept
			: path_(source)
		{
		}

	private:
		std::filesystem::path path_;
	};

	class fs_remover final
	{
	public:
		~fs_remover( );

		fs_remover(std::filesystem::path&& source, bool all) noexcept
			: path_(std::move(source)), all_(all)
		{
		}

		fs_remover(const std::filesystem::path& source, bool all) noexcept
			: path_(source), all_(all)
		{
		}

		bool all( )const { return all_; }

	private:
		std::filesystem::path path_;
		bool all_;
	};

	struct files_storage
	{
		std::vector<file_writer> write;
		std::vector<fs_creator> create;
		std::vector<fs_remover> remove;
	};
}
