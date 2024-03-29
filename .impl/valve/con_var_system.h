#pragma once

#include <fd/valve/app_system.h>
#include <fd/valve/color.h>
#include <fd/valve/vector.h>

namespace fd::valve
{
    class IConVar;
    class CCommand;
    class ConCommandBase;
    class con_var;
    class ConCommand;

    enum ConVarFlags : int
    {
        // convar systems
        FCVAR_NONE            = 0,
        FCVAR_UNREGISTERED    = (1 << 0),
        // if this is set, don't add to linked list, etc.
        FCVAR_DEVELOPMENTONLY = (1 << 1),
        // hidden in released products. flag is removed automatically if allow_development_cvars is defined.
        FCVAR_GAMEDLL         = (1 << 2),
        // defined by the game dll
        FCVAR_CLIENTDLL       = (1 << 3),
        // defined by the client dll
        FCVAR_HIDDEN          = (1 << 4),
        // hidden. doesn't appear in find or autocomplete. like developmentonly, but can't be compiled out.

        // convar only
        FCVAR_PROTECTED       = (1 << 5),
        // it's a server cvar, but we don't send the data since it's a password, etc.  sends 1 if it's not bland/zero, 0 otherwise as value
        FCVAR_SPONLY          = (1 << 6),
        // this cvar cannot be changed by clients connected to a multiplayer server.
        FCVAR_ARCHIVE         = (1 << 7),
        // set to cause it to be saved to vars.rc
        FCVAR_NOTIFY          = (1 << 8),
        // notifies players when changed
        FCVAR_USERINFO        = (1 << 9),
        // changes the client's info string
        FCVAR_FDS             = (1 << 14),
        // only useable in singleplayer / debug / multiplayer & sv_fdss
        FCVAR_PRINTABLEONLY   = (1 << 10),
        // this cvar's string cannot contain unprintable characters ( e.g., used for player name etc ).
        FCVAR_UNLOGGED        = (1 << 11),
        // if this is a fcvar_server, don't log changes to the log file / console if we are creating a log
        FCVAR_NEVER_AS_STRING = (1 << 12),
        // never try to print that cvar

        // it's a convar that's shared between the client and the server.
        // at signon, the values of all such convars are sent from the server to the client (skipped for local client, ofc )
        // if a change is requested it must come from the console (i.e., no remote client changes)
        // if a value is changed while a server is active, it's replicated to all connected clients
        FCVAR_SERVER                  = (1 << 13),
        // server setting enforced on clients, replicated
        FCVAR_DEMO                    = (1 << 16),
        // record this cvar when starting a demo file
        FCVAR_DONTRECORD              = (1 << 17),
        // don't record these command in demofiles
        FCVAR_RELOAD_MATERIALS        = (1 << 20),
        // if this cvar changes, it forces a material reload
        FCVAR_RELOAD_TEXTURES         = (1 << 21),
        // if this cvar changes, if forces a texture reload
        FCVAR_NOT_CONNECTED           = (1 << 22),
        // cvar cannot be changed by a client that is connected to a server
        FCVAR_MATERIAL_SYSTEM_THREAD  = (1 << 23),
        // indicates this cvar is read from the material system thread
        FCVAR_ARCHIVE_XBOX            = (1 << 24),
        // cvar written to config.cfg on the xbox
        FCVAR_ACCESSIBLE_FROM_THREADS = (1 << 25),
        // used as a debugging tool necessary to check material system thread convars
        FCVAR_SERVER_CAN_EXECUTE      = (1 << 28),
        // the server is allowed to execute this command on clients via clientcommand/net_stringcmd/cbaseclientstate::processstringcmd.
        FCVAR_SERVER_CANNOT_QUERY     = (1 << 29),
        // if this is set, then the server is not allowed to query this cvar's value (via iserverpluginhelpers::startquerycvarvalue).
        FCVAR_CLIENTCMD_CAN_EXECUTE   = (1 << 30),
        // ivengineclient::clientcmd is allowed to execute this command.
        FCVAR_MATERIAL_THREAD_MASK    = (FCVAR_RELOAD_MATERIALS | FCVAR_RELOAD_TEXTURES | FCVAR_MATERIAL_SYSTEM_THREAD)
    };

    //-----------------------------------------------------------------------------
    // Called when a con_var changes value
    // NOTE: For FCVAR_NEVER_AS_STRING ConVars, pOldValue == 0
    //-----------------------------------------------------------------------------
    typedef void (*FnChangeCallback_t)(IConVar* var, const char* pOldValue, float flOldValue);
    typedef void (*FnChangeCallbackV1_t)();
    using CVarDLLIdentifier_t = int;

    class IConVar
    {
      public:
        // Value set
        virtual void SetValue(const char* pValue) = 0;
        virtual void SetValue(float flValue)      = 0;
        virtual void SetValue(int nValue)         = 0;

        // Return name of command
        virtual const char* GetName() const = 0;

        // Accessors.. not as efficient as using GetState()/GetInfo()
        // if you call these methods multiple times on the same IConVar
        virtual bool IsFlagSet(int nFlag) const = 0;
    };

    class ConCommandBase
    {
#if 0
		friend class CCvar;
		friend class con_var;
		friend class ConCommand;
		friend void ConVar_Register(int nCVarFlag, IConCommandBaseAccessor* pAccessor);
		friend void ConVar_PublishToVXConsole( );

		// FIXME: Remove when con_var changes are done
		friend class CDefaultCvar;
#endif
      public:
        // ConCommandBase();
        // ConCommandBase(const char* pName, const char* pHelpString = 0, int flags = 0);

        virtual ~ConCommandBase() = default;

        virtual bool IsCommand() const = 0;

        // Check flag
        virtual bool IsFlagSet(int flag) const = 0;
        // Set flag
        virtual void AddFlags(int flags)       = 0;

        // Return name of cvar
        virtual const char* GetName() const = 0;

        // Return help text for cvar
        virtual const char* GetHelpText() const = 0;

        // Deal with next pointer
        // const ConCommandBase* GetNext() const;
        // ConCommandBase* GetNext();

        virtual bool IsRegistered() const = 0;

        // Returns the DLL identifier
        virtual CVarDLLIdentifier_t GetDLLIdentifier() const = 0;

      protected:
        virtual void CreateBase(const char* pName, const char* pHelpString = 0, int flags = 0) = 0;

        // Used internally by OneTimeInit to initialize/shutdown
        virtual void Init() = 0;
        // void						Shutdown( );

        // Internal copy routine ( uses new operator from correct module )
        // char* CopyString(const char* from);

      public:
        // Next con_var in chain
        // Prior to register, it points to the next convar in the DLL.
        // Once registered, though, Next is reset to point to the next
        // convar in the global list
        ConCommandBase* Next;

        // Has the cvar been added to the global list?
        bool Registered;

        // Static data
        const char* name;
        const char* szHelpString;

        // con_var flags
        int Flags;

      protected:
        // ConVars add themselves to this list for the executable.
        // Then ConVar_Register runs through  all the console variables
        // and registers them into a global list stored in vstdlib.dll
        // static ConCommandBase* s_pConCommandBases;

        // ConVars in this executable use this 'global' to access values.
        // static IConCommandBaseAccessor* s_pAccessor;
    };

    struct CVValue_t
    {
        char* szString;
        int m_StringLength;

        // Values
        float m_fValue;
        int Value;
    };

    struct con_var : ConCommandBase, IConVar
    {
#if 0
        template <typename T>
        T get() const;
        template <typename T>
        void set(T value);

#define CVAR_GET_SET(_TYPE_) \
    template <>              \
    _TYPE_ get() const;      \
    template <>              \
    void set(_TYPE_ value);

        CVAR_GET_SET(const char*);
        CVAR_GET_SET(float);
        CVAR_GET_SET(int);
        CVAR_GET_SET(bool);
#endif

        // This either points to "this" or it points to the original declaration of a con_var.
        // This allows ConVars to exist in separate modules, and they all use the first one to be declared.
        // Parent->m_pParent must equal Parent (ie: Parent must be the root, or original, con_var).
        con_var* Parent;

        // Static data
        const char* szDefaultValue;

        CVValue_t m_Value;

        // Min/Max values
        bool HasMin;
        float m_fMinVal;
        bool HasMax;
        float m_fMaxVal;

        // Call this function when con_var changes
        vector<FnChangeCallback_t> m_fnChangeCallbacks;

        /*float GetValue();
        int GetValueN();*/
    }; // Size=0x0048*

    //-----------------------------------------------------------------------------
    // Abstract interface for ConVars
    //-----------------------------------------------------------------------------

    struct con_var_system : app_system
    {
        virtual CVarDLLIdentifier_t AllocateDLLIdentifier()                                              = 0;
        virtual void RegisterConCommand(con_var* pCommandBase, int iDefaultValue = 1)                    = 0;
        virtual void UnregisterConCommand(con_var* pCommandBase)                                         = 0;
        virtual void UnregisterConCommands(CVarDLLIdentifier_t id)                                       = 0;
        virtual const char* GetCommandLineValue(const char* szVariableName)                              = 0;
        virtual ConCommandBase* FindCommandBase(const char* name)                                        = 0;
        virtual const ConCommandBase* FindCommandBase(const char* name) const                            = 0;
        virtual con_var* FindVar(const char* szVariableName)                                             = 0;
        virtual const con_var* FindVar(const char* szVariableName) const                                 = 0;
        virtual ConCommand* FindCommand(const char* name)                                                = 0;
        virtual const ConCommand* FindCommand(const char* name) const                                    = 0;
        virtual void InstallGlobalChangeCallback(FnChangeCallback_t callback)                            = 0;
        virtual void RemoveGlobalChangeCallback(FnChangeCallback_t callback)                             = 0;
        virtual void CallGlobalChangeCallbacks(con_var* pVar, const char* szOldString, float flOldValue) = 0;
        virtual void InstallConsoleDisplayFunc(void* pDisplayFunc)                                       = 0;
        virtual void RemoveConsoleDisplayFunc(void* pDisplayFunc)                                        = 0;
        virtual void ConsoleColorPrintf(const color& color, const char* pFormat, ...) const              = 0;
        virtual void ConsolePrintf(const char* pFormat, ...) const                                       = 0;
        virtual void ConsoleDPrintf(const char* pFormat, ...) const                                      = 0;
        virtual void RevertFlaggedConVars(int nFlag)                                                     = 0;

        con_var* FindVar(const char* name, const size_t size) const;
    };

} // namespace fd::valve
