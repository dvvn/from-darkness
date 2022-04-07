module;

export module cheat.hooks:unloader;

export namespace cheat::hooks
{
	void set_external_handle(void* const hmodule) noexcept;
	void unload( ) noexcept;
}