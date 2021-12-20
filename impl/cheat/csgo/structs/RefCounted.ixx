export module cheat.csgo.structs.RefCounted;

export namespace cheat::csgo
{
	class IRefCounted
	{
	private:
		volatile long refCount;

	public:
		virtual void destructor(char bDelete) = 0;
		virtual bool OnFinalRelease() = 0;

		/*void unreference()
		{
			if (InterlockedDecrement(&refCount) == 0 && OnFinalRelease())
			{
				destructor(1);
			}
		}*/
	};
}