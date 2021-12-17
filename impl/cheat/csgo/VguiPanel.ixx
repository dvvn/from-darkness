
export module cheat.csgo.structs:VguiPanel;

export namespace cheat::csgo
{
	class IVguiPanel
	{
	public:
#if 0
    const char *GetName(unsigned int vguiPanel)
    {
        typedef const char *(__thiscall* tGetName)(void*, unsigned int);
        return CallVFunction<tGetName>(this, 36)(this, vguiPanel);
    }
//#ifdef GetClassName
//#undef GetClassName
//#endif
    const char *GetClassName(unsigned int vguiPanel)
    {
        typedef const char *(__thiscall* tGetClassName)(void*, unsigned int);
        return CallVFunction<tGetClassName>(this, 37)(this, vguiPanel);
    }
#endif
	};
}
