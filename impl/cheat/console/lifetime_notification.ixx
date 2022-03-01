module;

#include <nstd/type name.h>

export module cheat.console.lifetime_notification;
import cheat.tools.object_name;

void console_log(const std::string_view name, const std::string_view msg);

export namespace cheat::console
{
	template<typename T>
	class lifetime_notification
	{
	protected:
		lifetime_notification( )
		{
			console_log(_Name( ), "created");
		}

		~lifetime_notification( )
		{
			console_log(_Name( ), "destroyed");
		}

	private:
		std::string_view _Name( )const
		{
			return tools::object_name<T>( );
		}
	};
}