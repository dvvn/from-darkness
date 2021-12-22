export module cheat.csgo.structs.ClientNetworkable;

export namespace cheat::csgo
{
	class bf_read;
	class IClientUnknown;
	class ClientClass;

	class IClientNetworkable
	{
	public:
		virtual IClientUnknown* GetIClientUnknown( ) = 0;
		virtual void            Release( ) = 0;
		virtual ClientClass*    GetClientClass( ) = 0;
		virtual void            NotifyShouldTransmit(int state) = 0;
		virtual void            OnPreDataChanged(int updateType) = 0;
		virtual void            OnDataChanged(int updateType) = 0;
		virtual void            PreDataUpdate(int updateType) = 0;
		virtual void            PostDataUpdate(int updateType) = 0;
		virtual void            __unkn() = 0;
		virtual bool            IsDormant() = 0;
		virtual int             EntIndex() const = 0;
		virtual void            ReceiveMessage(int classID, bf_read& msg) = 0;
		virtual void*           GetDataTableBasePtr( ) = 0;
		virtual void            SetDestroyedOnRecreateEntities() = 0;
	};
}