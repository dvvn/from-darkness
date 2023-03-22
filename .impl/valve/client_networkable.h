#pragma once

namespace fd::valve
{
    struct client_unknown;
    struct client_class;
    class bf_read;

    struct client_networkable
    {
        virtual client_unknown* GetIClientUnknown()            = 0;
        virtual void Release()                                 = 0;
        virtual client_class* GetClientClass()                 = 0;
        virtual void NotifyShouldTransmit(int state)           = 0;
        virtual void OnPreDataChanged(int updateType)          = 0;
        virtual void OnDataChanged(int updateType)             = 0;
        virtual void PreDataUpdate(int updateType)             = 0;
        virtual void PostDataUpdate(int updateType)            = 0;
        virtual void __unkn()                                  = 0;
        virtual bool IsDormant()                               = 0;
        virtual int EntIndex() const                           = 0;
        virtual void ReceiveMessage(int classID, bf_read& msg) = 0;
        virtual void* GetDataTableBasePtr()                    = 0;
        virtual void SetDestroyedOnRecreateEntities()          = 0;
    };

} // namespace fd::valve