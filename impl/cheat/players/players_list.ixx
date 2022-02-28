module;

#include <vector>

export module cheat.players:list;
export import :player;
import nstd.one_instance;

template<typename T>
class extended_vector
{
protected:
	void resize(size_t count)
	{
		storage_.resize(count);
	}

public:
	auto& at(size_t index)
	{
		return storage_[index - 1];
	}

	const auto& at(size_t index) const
	{
		return storage_[index - 1];
	}

	auto begin( )
	{
		return storage_.begin( );
	}
	auto begin( )const
	{
		return storage_.begin( );
	}

	auto end( )
	{
		return storage_.end( );
	}
	auto end( )const
	{
		return storage_.end( );
	}

private:
	std::vector<T> storage_;
};

namespace cheat
{
	export struct players_list :extended_vector<player>, nstd::one_instance<players_list>
	{
		void update( );
	};
}
