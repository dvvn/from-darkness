export module cheat.csgo.interfaces:ClientEntityList;
export import :ClientEntity;

export namespace cheat::csgo
{
	/*class IClientNetworkable;
	class IClientEntity;
	class CBaseHandle;*/

	class IClientEntityList
	{
	public:
		virtual IClientNetworkable* GetClientNetworkable(int entnum) = 0;
		virtual void* vtablepad0x1( ) = 0;
		virtual void* vtablepad0x2( ) = 0;
		virtual IClientEntity* GetClientEntity(int entNum) = 0;
		virtual IClientEntity* GetClientEntityFromHandle(CBaseHandle hEnt) = 0;
		virtual int NumberOfEntities(bool bIncludeNonNetworkable) = 0;
		virtual int GetHighestEntityIndex( ) = 0;
		virtual void SetMaxEntities(int maxEnts) = 0;
		virtual int GetMaxEntities( ) = 0;
	};
}
