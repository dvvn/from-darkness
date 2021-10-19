#pragma once
#include "CUtlVector.hpp"

namespace cheat::csgo
{
	class Color;
	//-----------------------------------------------------------------------------
	// Forward declarations
	//-----------------------------------------------------------------------------
	class IConVar;
	class CCommand;

	enum ConVarFlags : int
	{
		// convar systems
		FCVAR_NONE = 0,
		FCVAR_UNREGISTERED = (1 << 0),
		// if this is set, don't add to linked list, etc.
		FCVAR_DEVELOPMENTONLY = (1 << 1),
		// hidden in released products. flag is removed automatically if allow_development_cvars is defined.
		FCVAR_GAMEDLL = (1 << 2),
		// defined by the game dll
		FCVAR_CLIENTDLL = (1 << 3),
		// defined by the client dll
		FCVAR_HIDDEN = (1 << 4),
		// hidden. doesn't appear in find or autocomplete. like developmentonly, but can't be compiled out.

		// convar only
		FCVAR_PROTECTED = (1 << 5),
		// it's a server cvar, but we don't send the data since it's a password, etc.  sends 1 if it's not bland/zero, 0 otherwise as value
		FCVAR_SPONLY = (1 << 6),
		// this cvar cannot be changed by clients connected to a multiplayer server.
		FCVAR_ARCHIVE = (1 << 7),
		// set to cause it to be saved to vars.rc
		FCVAR_NOTIFY = (1 << 8),
		// notifies players when changed
		FCVAR_USERINFO = (1 << 9),
		// changes the client's info string
		FCVAR_CHEAT = (1 << 14),
		// only useable in singleplayer / debug / multiplayer & sv_cheats
		FCVAR_PRINTABLEONLY = (1 << 10),
		// this cvar's string cannot contain unprintable characters ( e.g., used for player name etc ).
		FCVAR_UNLOGGED = (1 << 11),
		// if this is a fcvar_server, don't log changes to the log file / console if we are creating a log
		FCVAR_NEVER_AS_STRING = (1 << 12),
		// never try to print that cvar

		// it's a convar that's shared between the client and the server.
		// at signon, the values of all such convars are sent from the server to the client (skipped for local client, ofc )
		// if a change is requested it must come from the console (i.e., no remote client changes)
		// if a value is changed while a server is active, it's replicated to all connected clients
		FCVAR_SERVER = (1 << 13),
		// server setting enforced on clients, replicated
		FCVAR_DEMO = (1 << 16),
		// record this cvar when starting a demo file
		FCVAR_DONTRECORD = (1 << 17),
		// don't record these command in demofiles
		FCVAR_RELOAD_MATERIALS = (1 << 20),
		// if this cvar changes, it forces a material reload
		FCVAR_RELOAD_TEXTURES = (1 << 21),
		// if this cvar changes, if forces a texture reload
		FCVAR_NOT_CONNECTED = (1 << 22),
		// cvar cannot be changed by a client that is connected to a server
		FCVAR_MATERIAL_SYSTEM_THREAD = (1 << 23),
		// indicates this cvar is read from the material system thread
		FCVAR_ARCHIVE_XBOX = (1 << 24),
		// cvar written to config.cfg on the xbox
		FCVAR_ACCESSIBLE_FROM_THREADS = (1 << 25),
		// used as a debugging tool necessary to check material system thread convars
		FCVAR_SERVER_CAN_EXECUTE = (1 << 28),
		// the server is allowed to execute this command on clients via clientcommand/net_stringcmd/cbaseclientstate::processstringcmd.
		FCVAR_SERVER_CANNOT_QUERY = (1 << 29),
		// if this is set, then the server is not allowed to query this cvar's value (via iserverpluginhelpers::startquerycvarvalue).
		FCVAR_CLIENTCMD_CAN_EXECUTE = (1 << 30),
		// ivengineclient::clientcmd is allowed to execute this command. 
		FCVAR_MATERIAL_THREAD_MASK = (FCVAR_RELOAD_MATERIALS | FCVAR_RELOAD_TEXTURES | FCVAR_MATERIAL_SYSTEM_THREAD)
	};

	//-----------------------------------------------------------------------------
	// Called when a ConVar changes value
	// NOTE: For FCVAR_NEVER_AS_STRING ConVars, pOldValue == 0
	//-----------------------------------------------------------------------------
	typedef void (*FnChangeCallback_t)(IConVar* var, const char* pOldValue, float flOldValue);
	typedef void (*FnChangeCallbackV1_t)(void);

	//-----------------------------------------------------------------------------
	// Abstract interface for ConVars
	//-----------------------------------------------------------------------------
	class IConVar
	{
	public:
		virtual void        SetValue(const char* pValue) = 0;
		virtual void        SetValue(float flValue) = 0;
		virtual void        SetValue(int nValue) = 0;
		virtual void        SetValue(Color value) = 0;
		virtual const char* GetName( ) const = 0;
		virtual const char* GetBaseName( ) const = 0;
		virtual bool        IsFlagSet(int nFlag) const = 0;
		virtual int         GetSplitScreenPlayerSlot( ) const = 0;
	};

	struct CVValue_t
	{
		char* m_pszString;
		int   m_StringLength;

		// Values
		float m_fValue;
		int   m_nValue;
	};

	class ConVar
	{
	public:
		template <typename T>
		void set(T value);

		template <typename T>
		T get( ) const;

	private:
		void* virtualtable;
	public:
		ConVar*              m_pNext;
		int                  m_bRegistered;
		char*                m_pszName;
		char*                m_pszHelpString;
		ConVarFlags          m_nFlags;
		FnChangeCallbackV1_t m_fnChangeCallbacksV1;
		ConVar*              m_pParent;
		char*                m_pszDefaultValue;
		CVValue_t            m_Value;
		int                  m_bHasMin;
		float                m_fMinVal;
		int                  m_bHasMax;
		float                m_fMaxVal;

		CUtlVector<FnChangeCallback_t> m_fnChangeCallbacks; // note: this is also accessible as FnChangeCallback_t* instead of CUtlVector

		/*float GetValue();
		int GetValueN();*/
	}; //Size=0x0048*
}
