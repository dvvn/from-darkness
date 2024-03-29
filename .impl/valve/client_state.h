module;

#include <cstdint>

export module fd.valve.client_state;
export import fd.math.qangle;

using namespace fd::math;

class client_class;

#pragma pack(push, 1)

struct INetChannel
{
    char pad_0000[20];         // 0x0000
    bool ProcessingMessages;   // 0x0014
    bool ShouldDelete;         // 0x0015
    char pad_0016[2];          // 0x0016
    int32_t nOutSequenceNr;    // 0x0018 last send outgoing sequence number
    int32_t nInSequenceNr;     // 0x001C last received incoming sequnec number
    int32_t nOutSequenceNrAck; // 0x0020 last received acknowledge outgoing sequnce number
    int32_t nOutReliableState; // 0x0024 state of outgoing reliable data (0/1) flip flop used for loss detection
    int32_t nInReliableState;  // 0x0028 state of incoming reliable data
    int32_t nChokedPackets;    // 0x002C number of choked packets
    char pad_0030[1044];       // 0x0030
};                             // Size: 0x0444

struct clock_drift_mgr
{
    float ClockOffsets[16];   // 0x0000
    uint32_t iCurClockOffset; // 0x0044
    uint32_t nServerTick;     // 0x0048
    uint32_t nClientTick;     // 0x004C
};                            // Size: 0x0050

struct event_info
{
    enum
    {
        EVENT_INDEX_BITS    = 8,
        EVENT_DATA_LEN_BITS = 11,
        MAX_EVENT_DATA      = 192,
        // ( 1<<8 bits == 256, but only using 192 below )
    };

    event_info()
    {
        classID      = 0;
        fire_delay   = 0.0f;
        flags        = 0;
        pSendTable   = nullptr;
        pClientClass = nullptr;
        Packed       = 0;
    }

    short classID;
    short pad;
    float fire_delay;
    void* pSendTable;
    client_class* pClientClass;
    int Packed;
    int flags;
    int filter[8];
    event_info* next;
};

enum signon_states
{
    // no state yet, about to connect
    SIGNONSTATE_NONE        = 0,
    // client challenging server, all OOB packets
    SIGNONSTATE_CHALLENGE   = 1,
    // client is connected to server, netchans ready
    SIGNONSTATE_CONNECTED   = 2,
    // just got serverinfo and string tables
    SIGNONSTATE_NEW         = 3,
    // received signon buffers
    SIGNONSTATE_PRESPAWN    = 4,
    // ready to receive entity packets
    SIGNONSTATE_SPAWN       = 5,
    // we are fully connected, first non-delta packet received
    SIGNONSTATE_FULL        = 6,
    // server is changing level, please wait
    SIGNONSTATE_CHANGELEVEL = 7
};

struct client_state
{
    void ForceFullUpdate()
    {
        nDeltaTick = -1;
    }

    char pad_0000[156];
    INetChannel* NetChannel;
    int nChallengeNr;
    char pad_00A4[100];
    signon_states nSignonState;
    int signon_pads[2];
    float flNextCmdTime;
    int nServerCount;
    int nCurrentSequence;
    int musor_pads[2];
    clock_drift_mgr ClockDriftMgr;
    int nDeltaTick;
    bool Paused;
    char paused_align[3];
    int nViewEntity;
    int nPlayerSlot;
    int bruh;
    char szLevelName[260];
    char szLevelNameShort[80];
    char szGroupName[80];
    char pad_032[92];
    int nMaxClients;
    char pad_0314[18828];
    float nLastServerTickTime;
    bool InSimulation;
    char pad_4C9D[3];
    int nOldTickCount;
    float flTickReminder;
    float flFrametime;
    int nLastOutgoingCommand;
    int nChokedCommands;
    int nLastCommandAck;
    int nPacketEndTickUpdate;
    int nCommandAck;
    int nSoundSequence;
    char pad_4CCD[76];
    qangle viewangles;
    int pads[54];
    event_info* pEvents;
};

#pragma pack(pop)

/*static_assert(FIELD_OFFSET(client_state, NetChannel) == 0x009C, "Wrong struct offset");
static_assert(FIELD_OFFSET(client_state, nCurrentSequence) == 0x011C, "Wrong struct offset");
static_assert(FIELD_OFFSET(client_state, nDeltaTick) == 0x0174, "Wrong struct offset");
static_assert(FIELD_OFFSET(client_state, nMaxClients) == 0x0388, "Wrong struct offset");
static_assert(FIELD_OFFSET(client_state, viewangles) == 0x4D90, "Wrong struct offset");*/

export namespace fd::valve
{
    using ::client_state;
}
