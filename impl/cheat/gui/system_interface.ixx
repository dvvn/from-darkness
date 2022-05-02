module;

#include <RmlUi/Core/SystemInterface.h>

#include <chrono>

export module cheat.gui.system_interface;

using namespace Rml;

export namespace cheat::gui
{
	class system_interface final : public SystemInterface
	{
		using clock = std::chrono::high_resolution_clock;
		using out_duration = std::chrono::duration<double>;
		clock::time_point start_time_;

	public:
		system_interface( );

		double GetElapsedTime( ) override;
		bool LogMessage(Log::Type logtype, const String& message) override;
	};
}