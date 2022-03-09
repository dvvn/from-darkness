module;

#include <string_view>

export module cheat.console.lifetime_notification;
import cheat.tools.object_name;

void console_log(const std::string_view name, const std::string_view msg);

export namespace cheat::console
{
	template<typename T>
	class lifetime_notification
	{
	protected:

		void message(const std::string_view msg)const
		{
			console_log(_Name( ), msg);
		}

		lifetime_notification( )
		{
			message("created");
		}

		~lifetime_notification( )
		{
			message("destroyed");
		}

	private:
		std::string_view _Name( )const
		{
			return tools::object_name<T>( );
		}
	};
}