module;

#include <cstdint>

export module cheat.csgo.interfaces:EngineClient;
export import :MaterialSystem;

export namespace cheat::csgo
{
	using InputContextHandle_t = struct InputContextHandle_t__*;
	struct client_textmessage_t;
	class SurfInfo;
	class CSentence;
	class CAudioSource;
	class AudioState_t;
	class ISpatialQuery;
	class CPhysCollide;
	class IAchievementMgr;
	class model_t;

	enum class FLOW :uint32_t
	{
		OUTGOING = 0
		, INCOMING = 1

		//constexpr auto MAX_FLOWS	=	2	;	// in & out
	};

	class INetChannelInfo
	{
	public:
		enum
		{
			// must be first and is default group
			GENERIC = 0
			// bytes for local player entity update
			, LOCALPLAYER
			// bytes for other players update
			, OTHERPLAYERS
			// all other entity bytes
			, ENTITIES
			// game sounds
			, SOUNDS
			// event messages
			, EVENTS
			// user messages
			, USERMESSAGES
			// entity messages
			, ENTMESSAGES
			// voice data
			, VOICE
			// a stringtable update
			, STRINGTABLE
			// client move cmds
			, MOVE
			// string command
			, STRINGCMD
			, // various signondata
			SIGNON
			, // must be last and is not a real group
			TOTAL
		};

		virtual const char* GetName( ) const = 0;    // get channel name
		virtual const char* GetAddress( ) const = 0; // get channel IP utl::address as string
		virtual float GetTime( ) const = 0;          // current net time
		virtual float GetTimeConnected( ) const = 0; // get connection time in seconds
		virtual int GetBufferSize( ) const = 0;      // netchannel packet history size
		virtual int GetDataRate( ) const = 0;        // send data rate in byte/sec

		virtual bool IsLoopback( ) const = 0;  // true if loopback channel
		virtual bool IsTimingOut( ) const = 0; // true if timing out
		virtual bool IsPlayback( ) const = 0;  // true if demo playback

		virtual float GetLatency(FLOW flow) const = 0;                                  // current latency (RTT), more accurate but jittering
		virtual float GetAvgLatency(FLOW flow) const = 0;                               // average packet latency in seconds
		virtual float GetAvgLoss(FLOW flow) const = 0;                                  // avg packet loss[0..1]
		virtual float GetAvgChoke(FLOW flow) const = 0;                                 // avg packet choke[0..1]
		virtual float GetAvgData(FLOW flow) const = 0;                                  // data flow in bytes/sec
		virtual float GetAvgPackets(FLOW flow) const = 0;                               // avg packets/sec
		virtual int GetTotalData(FLOW flow) const = 0;                                  // total flow in/out in bytes
		virtual int GetSequenceNr(FLOW flow) const = 0;                                 // last send seq number
		virtual bool IsValidPacket(FLOW flow, int frame_number) const = 0;              // true if packet was not lost/dropped/chocked/flushed
		virtual float GetPacketTime(FLOW flow, int frame_number) const = 0;             // time when packet was send
		virtual int GetPacketBytes(FLOW flow, int frame_number, int group) const = 0;   // group size of this packet
		virtual bool GetStreamProgress(FLOW flow, int* received, int* total) const = 0; // TCP progress if transmitting
		virtual float GetTimeSinceLastReceived( ) const = 0;                             // get time since last recieved packet in seconds
		virtual float GetCommandInterpolationAmount(FLOW flow, int frame_number) const = 0;
		virtual void GetPacketResponseLatency(FLOW flow, int frame_number, int* pnLatencyMsecs, int* pnChoke) const = 0;
		virtual void GetRemoteFramerate(float* pflFrameTime, float* pflFrameTimeStdDeviation) const = 0;

		virtual float GetTimeoutSeconds( ) const = 0;
	};

	class ISPSharedMemory;
	class CGamestatsData;
	class KeyValues;
	class CSteamAPIContext;
	struct Frustum_t;

	using pfnDemoCustomDataCallback = void(*)(uint8_t* pData, size_t iSize);

	using player_info_t = struct player_info_s
	{
		int64_t unknown; //0x0000 
		union
		{
			int64_t steam_id_num64; //0x0008 - SteamID64
			struct
			{
				int32_t low;
				int32_t high;
			} xuid;
		};

		char name[128];             //0x0010 - Player Name
		int user_id;                //0x0090 - Unique Server Identifier
		char steam_id[20];          //0x0094 - STEAM_X:Y:Z
		char pad_0x00A8[0x10];      //0x00A8
		unsigned long steam_id_num; //0x00B8 - SteamID 
		char friends_name[128];
		bool fake_player;
		bool hltv;
		unsigned int custom_files[4];
		unsigned char files_downloaded;
	};

	class IMapOverview // : public IBaseInterface
	{
	public:
		virtual ~IMapOverview( ) = default;

		virtual void SetVisible(bool state) = 0;                                                     // set map panel visible
		virtual void SetBounds(int x, int y, int wide, int tall) = 0;                                // set pos & size
		virtual void SetZoom(float zoom) = 0;                                                        // set zoom
		virtual void SetTime(float time) = 0;                                                        // set game time
		virtual void SetAngle(float angle) = 0;                                                      // set map orientation
		virtual void SetFollowAngle(bool state) = 0;                                                 // if true, map rotates with spectators view
		virtual void SetCenter(Vector2D& mappos) = 0;                                                // set map pos in center of panel
		virtual void SetPlayerPositions(int index, const Vector& position, const QAngle& angle) = 0; // update player position
		virtual Vector2D WorldToMap(Vector& worldpos) = 0;                                           // convert 3d world to 2d map pos

		virtual bool IsVisible( ) = 0;                                     // true if MapOverview is visible
		virtual void GetBounds(int& x, int& y, int& wide, int& tall) = 0; // get current pos & size
		virtual float GetZoom( ) = 0;

		// deatils properties
		virtual void ShowPlayerNames(bool state) = 0; // show player names under icons
		virtual void ShowTracers(bool state) = 0;     // show shooting traces as lines
		virtual void ShowExplosions(bool state) = 0;  // show, smoke, flash & HE grenades
		virtual void ShowHealth(bool state) = 0;      // show player health under icon
		virtual void ShowHurts(bool state) = 0;       // show player icon flashing if player is hurt
		virtual void ShowTracks(float seconds) = 0;   // show player trails for n seconds
	};

	//#ifdef CopyFile
	//#undef CopyFile
	//#endif

	class IVEngineClient
	{
	public:
		virtual int GetIntersectingSurfaces(const model_t* model, const Vector& vCenter, float radius, bool bOnlyVisibleSurfaces, SurfInfo* pInfos, int nMaxInfos) = 0;
		virtual Vector GetLightForPoint(const Vector& pos, bool bClamp) = 0;
		virtual IMaterial* TraceLineMaterialAndLighting(const Vector& start, const Vector& end, Vector& diffuseLightColor, Vector& baseColor) = 0;
		virtual const char* ParseFile(const char* data, char* token, int maxlen) = 0;
		virtual bool CopyFile(const char* source, const char* destination) = 0;
		virtual void GetScreenSize(int& width, int& height) = 0;
		virtual void ServerCmd(const char* szCmdString, bool bReliable = true) = 0;
		virtual void ClientCmd(const char* szCmdString) = 0;
		virtual bool GetPlayerInfo(int ent_num, player_info_t* pinfo) = 0;
		virtual int GetPlayerForUserID(int userID) = 0;
		virtual client_textmessage_t* TextMessageGet(const char* pName) = 0; // 10
		virtual bool Con_IsVisible( ) = 0;
		virtual int GetLocalPlayer( ) = 0;
		virtual const model_t* LoadModel(const char* pName, bool bProp = false) = 0;
		virtual float GetLastTimeStamp( ) = 0;
		virtual CSentence* GetSentence(CAudioSource* pAudioSource) = 0; // 15
		virtual float GetSentenceLength(CAudioSource* pAudioSource) = 0;
		virtual bool IsStreaming(CAudioSource* pAudioSource) const = 0;
		virtual void GetViewAngles(QAngle& va) = 0;
		virtual void SetViewAngles(QAngle& va) = 0;
		virtual int GetMaxClients( ) = 0; // 20
		virtual const char* Key_LookupBinding(const char* pBinding) = 0;
		virtual const char* Key_BindingForKey(int& code) = 0;
		virtual void Key_SetBinding(int, const char*) = 0;
		virtual void StartKeyTrapMode( ) = 0;
		virtual bool CheckDoneKeyTrapping(int& code) = 0;
		virtual bool IsInGame( ) = 0;
		virtual bool IsConnected( ) = 0;
		virtual bool IsDrawingLoadingImage( ) = 0;
		virtual void HideLoadingPlaque( ) = 0;
		virtual void Con_NPrintf(int pos, const char* fmt, ...) = 0; // 30
		virtual void Con_NXPrintf(const struct con_nprint_s* info, const char* fmt, ...) = 0;
		virtual int IsBoxVisible(const Vector& mins, const Vector& maxs) = 0;
		virtual int IsBoxInViewCluster(const Vector& mins, const Vector& maxs) = 0;
		virtual bool CullBox(const Vector& mins, const Vector& maxs) = 0;
		virtual void Sound_ExtraUpdate( ) = 0;
		virtual const char* GetGameDirectory( ) = 0;
		virtual const VMatrix& WorldToScreenMatrix( ) = 0;
		virtual const VMatrix& WorldToViewMatrix( ) = 0;
		virtual int GameLumpVersion(int lumpId) const = 0;
		virtual int GameLumpSize(int lumpId) const = 0; // 40
		virtual bool LoadGameLump(int lumpId, void* pBuffer, int size) = 0;
		virtual int LevelLeafCount( ) const = 0;
		virtual ISpatialQuery* GetBSPTreeQuery( ) = 0;
		virtual void LinearToGamma(float* linear, float* gamma) = 0;
		virtual float LightStyleValue(int style) = 0; // 45
		virtual void ComputeDynamicLighting(const Vector& pt, const Vector* pNormal, Vector& color) = 0;
		virtual void GetAmbientLightColor(Vector& color) = 0;
		virtual int GetDXSupportLevel( ) = 0;
		virtual bool SupportsHDR( ) = 0;
		virtual void Mat_Stub(IMaterialSystem* pMatSys) = 0; // 50
		virtual void GetChapterName(char* pchBuff, int iMaxLength) = 0;
		virtual const char* GetLevelName( ) = 0;
		virtual const char* GetLevelNameShort( ) = 0;
		virtual const char* GetMapGroupName( ) = 0;
		virtual struct IVoiceTweak_s* GetVoiceTweakAPI( ) = 0;
		virtual void SetVoiceCasterID(unsigned int someint) = 0; // 56
		virtual void EngineStats_BeginFrame( ) = 0;
		virtual void EngineStats_EndFrame( ) = 0;
		virtual void FireEvents( ) = 0;
		virtual int GetLeavesArea(unsigned short* pLeaves, int nLeaves) = 0;
		virtual bool DoesBoxTouchAreaFrustum(const Vector& mins, const Vector& maxs, int iArea) = 0; // 60
		virtual int GetFrustumList(Frustum_t** pList, int listMax) = 0;
		virtual bool ShouldUseAreaFrustum(int i) = 0;
		virtual void SetAudioState(const AudioState_t& state) = 0;
		virtual int SentenceGroupPick(int groupIndex, char* name, int nameBufLen) = 0;
		virtual int SentenceGroupPickSequential(int groupIndex, char* name, int nameBufLen, int sentenceIndex, int reset) = 0;
		virtual int SentenceIndexFromName(const char* pSentenceName) = 0;
		virtual const char* SentenceNameFromIndex(int sentenceIndex) = 0;
		virtual int SentenceGroupIndexFromName(const char* pGroupName) = 0;
		virtual const char* SentenceGroupNameFromIndex(int groupIndex) = 0;
		virtual float SentenceLength(int sentenceIndex) = 0;
		virtual void ComputeLighting(const Vector& pt, const Vector* pNormal, bool bClamp, Vector& color, Vector* pBoxColors = 0) = 0;
		virtual void ActivateOccluder(int nOccluderIndex, bool bActive) = 0;
		virtual bool IsOccluded(const Vector& vecAbsMins, const Vector& vecAbsMaxs) = 0; // 74
		virtual int GetOcclusionViewId( ) = 0;
		virtual void* SaveAllocMemory(size_t num, size_t size) = 0;
		virtual void SaveFreeMemory(void* pSaveMem) = 0;
		virtual INetChannelInfo* GetNetChannelInfo( ) = 0;
		virtual void DebugDrawPhysCollide(const CPhysCollide* pCollide, IMaterial* pMaterial, const matrix3x4_t& transform, const uint8_t* color) = 0; //79
		virtual void CheckPoint(const char* pName) = 0;                                                                                                // 80
		virtual void DrawPortals( ) = 0;
		virtual bool IsPlayingDemo( ) = 0;
		virtual bool IsRecordingDemo( ) = 0;
		virtual bool IsPlayingTimeDemo( ) = 0;
		virtual int GetDemoRecordingTick( ) = 0;
		virtual int GetDemoPlaybackTick( ) = 0;
		virtual int GetDemoPlaybackStartTick( ) = 0;
		virtual float GetDemoPlaybackTimeScale( ) = 0;
		virtual int GetDemoPlaybackTotalTicks( ) = 0;
		virtual bool IsPaused( ) = 0; // 90
		virtual float GetTimescale( ) const = 0;
		virtual bool IsTakingScreenshot( ) = 0;
		virtual bool IsHLTV( ) = 0;
		virtual bool IsLevelMainMenuBackground( ) = 0;
		virtual void GetMainMenuBackgroundName(char* dest, int destlen) = 0;
		virtual void SetOcclusionParameters(const int /*OcclusionParams_t*/& params) = 0; // 96
		virtual void GetUILanguage(char* dest, int destlen) = 0;
		virtual int IsSkyboxVisibleFromPoint(const Vector& vecPoint) = 0;
		virtual const char* GetMapEntitiesString( ) = 0;
		virtual bool IsInEditMode( ) = 0; // 100
		virtual float GetScreenAspectRatio(int viewportWidth, int viewportHeight) = 0;
		virtual bool REMOVED_SteamRefreshLogin(const char* password, bool isSecure) = 0;
		virtual bool REMOVED_SteamProcessCall(bool& finished) = 0;
		virtual unsigned int GetEngineBuildNumber( ) = 0;   // engines build
		virtual const char* GetProductVersionString( ) = 0; // mods version number (steam.inf)
		virtual void GrabPreColorCorrectedFrame(int x, int y, int width, int height) = 0;
		virtual bool IsHammerRunning( ) const = 0;
		virtual void ExecuteClientCmd(const char* szCmdString) = 0; //108
		virtual bool MapHasHDRLighting( ) = 0;
		virtual bool MapHasLightMapAlphaData( ) = 0;
		virtual int GetAppID( ) = 0;
		virtual Vector GetLightForPointFast(const Vector& pos, bool bClamp) = 0;
		virtual void ClientCmd_Unrestricted(const char*, int, bool) = 0;
		virtual void ClientCmd_Unrestricted(const char* szCmdString) = 0; // 114
		virtual void SetRestrictServerCommands(bool bRestrict) = 0;
		virtual void SetRestrictClientCommands(bool bRestrict) = 0;
		virtual void SetOverlayBindProxy(int iOverlayID, void* pBindProxy) = 0;
		virtual bool CopyFrameBufferToMaterial(const char* pMaterialName) = 0;
		virtual void ReadConfiguration(int iController, bool readDefault) = 0;
		virtual void SetAchievementMgr(IAchievementMgr* pAchievementMgr) = 0;
		virtual IAchievementMgr* GetAchievementMgr( ) = 0;
		virtual bool MapLoadFailed( ) = 0;
		virtual void SetMapLoadFailed(bool bState) = 0;
		virtual bool IsLowViolence( ) = 0;
		virtual const char* GetMostRecentSaveGame( ) = 0;
		virtual void SetMostRecentSaveGame(const char* lpszFilename) = 0;
		virtual void StartXboxExitingProcess( ) = 0;
		virtual bool IsSaveInProgress( ) = 0;
		virtual bool IsAutoSaveDangerousInProgress( ) = 0;
		virtual unsigned int OnStorageDeviceAttached(int iController) = 0;
		virtual void OnStorageDeviceDetached(int iController) = 0;
		virtual const char* GetSaveDirName( ) = 0;
		virtual void WriteScreenshot(const char* pFilename) = 0;
		virtual void ResetDemoInterpolation( ) = 0;
		virtual int GetActiveSplitScreenPlayerSlot( ) = 0;
		virtual int SetActiveSplitScreenPlayerSlot(int slot) = 0;
		virtual bool SetLocalPlayerIsResolvable(const char* pchContext, int nLine, bool bResolvable) = 0;
		virtual bool IsLocalPlayerResolvable( ) = 0;
		virtual int GetSplitScreenPlayer(int nSlot) = 0;
		virtual bool IsSplitScreenActive( ) = 0;
		virtual bool IsValidSplitScreenSlot(int nSlot) = 0;
		virtual int FirstValidSplitScreenSlot( ) = 0;                 // -1 == invalid
		virtual int NextValidSplitScreenSlot(int nPreviousSlot) = 0; // -1 == invalid
		virtual ISPSharedMemory* GetSinglePlayerSharedMemorySpace(const char* szName, int ent_num = (1 << 11)) = 0;
		virtual void ComputeLightingCube(const Vector& pt, bool bClamp, Vector* pBoxColors) = 0;
		virtual void RegisterDemoCustomDataCallback(const char* szCallbackSaveID, pfnDemoCustomDataCallback pCallback) = 0;
		virtual void RecordDemoCustomData(pfnDemoCustomDataCallback pCallback, const void* pData, size_t iDataLength) = 0;
		virtual void SetPitchScale(float flPitchScale) = 0;
		virtual float GetPitchScale( ) = 0;
		virtual bool LoadFilmmaker( ) = 0;
		virtual void UnloadFilmmaker( ) = 0;
		virtual void SetLeafFlag(int nLeafIndex, int nFlagBits) = 0;
		virtual void RecalculateBSPLeafFlags( ) = 0;
		virtual bool DSPGetCurrentDASRoomNew( ) = 0;
		virtual bool DSPGetCurrentDASRoomChanged( ) = 0;
		virtual bool DSPGetCurrentDASRoomSkyAbove( ) = 0;
		virtual float DSPGetCurrentDASRoomSkyPercent( ) = 0;
		virtual void SetMixGroupOfCurrentMixer(const char* szgroupname, const char* szparam, float val, int setMixerType) = 0;
		virtual int GetMixLayerIndex(const char* szmixlayername) = 0;
		virtual void SetMixLayerLevel(int index, float level) = 0;
		virtual int GetMixGroupIndex(const char* groupname) = 0;
		virtual void SetMixLayerTriggerFactor(int i1, int i2, float fl) = 0;
		virtual void SetMixLayerTriggerFactor(const char* char1, const char* char2, float fl) = 0;
		virtual bool IsCreatingReslist( ) = 0;
		virtual bool IsCreatingXboxReslist( ) = 0;
		virtual void SetTimescale(float flTimescale) = 0;
		virtual void SetGamestatsData(CGamestatsData* pGamestatsData) = 0;
		virtual CGamestatsData* GetGamestatsData( ) = 0;
		virtual void GetMouseDelta(int& dx, int& dy, bool b) = 0; // unknown
		virtual const char* Key_LookupBindingEx(const char* pBinding, int iUserId = -1, int iStartCount = 0, int iAllowJoystick = -1) = 0;
		virtual int Key_CodeForBinding(const char*, int, int, int) = 0;
		virtual void UpdateDAndELights( ) = 0;
		virtual int GetBugSubmissionCount( ) const = 0;
		virtual void ClearBugSubmissionCount( ) = 0;
		virtual bool DoesLevelContainWater( ) const = 0;
		virtual float GetServerSimulationFrameTime( ) const = 0;
		virtual void SolidMoved(class IClientEntity* pSolidEnt, class ICollideable* pSolidCollide, const Vector* pPrevAbsOrigin, bool accurateBboxTriggerChecks) = 0;
		virtual void TriggerMoved(class IClientEntity* pTriggerEnt, bool accurateBboxTriggerChecks) = 0;
		virtual void ComputeLeavesConnected(const Vector& vecOrigin, int nCount, const int* pLeafIndices, bool* pIsConnected) = 0;
		virtual bool IsInCommentaryMode( ) = 0;
		virtual void SetBlurFade(float amount) = 0;
		virtual bool IsTransitioningToLoad( ) = 0;
		virtual void SearchPathsChangedAfterInstall( ) = 0;
		virtual void ConfigureSystemLevel(int nCPULevel, int nGPULevel) = 0;
		virtual void SetConnectionPassword(const char* pchCurrentPW) = 0;
		virtual CSteamAPIContext* GetSteamAPIContext( ) = 0;
		virtual void SubmitStatRecord(const char* szMapName, unsigned int uiBlobVersion, unsigned int uiBlobSize, const void* pvBlob) = 0;
		virtual void ServerCmdKeyValues(KeyValues* pKeyValues) = 0; // 203
		virtual void SpherePaintSurface(const model_t* model, const Vector& location, unsigned char chr, float fl1, float fl2) = 0;
		virtual bool HasPaintmap( ) = 0;
		virtual void EnablePaintmapRender( ) = 0;
		//virtual void                TracePaintSurface( const model_t *model, const Vector& position, float radius, CUtlVector<Color>& surfColors ) = 0;
		virtual void SphereTracePaintSurface(const model_t* model, const Vector& position, const Vector& vec2, float radius
											 , /*CUtlVector<unsigned char, CUtlMemory<unsigned char, int>>*/ int& utilVecShit) = 0;
		virtual void RemoveAllPaint( ) = 0;
		virtual void PaintAllSurfaces(unsigned char uchr) = 0;
		virtual void RemovePaint(const model_t* model) = 0;
		virtual bool IsActiveApp( ) = 0;
		virtual bool IsClientLocalToActiveServer( ) = 0;
		virtual void TickProgressBar( ) = 0;
		virtual InputContextHandle_t GetInputContext(int /*EngineInputContextId_t*/ id) = 0;
		virtual void GetStartupImage(char* filename, int size) = 0;
		virtual bool IsUsingLocalNetworkBackdoor( ) = 0;
		virtual void SaveGame(const char*, bool, char*, int, char*, int) = 0;
		virtual void GetGenericMemoryStats(/* GenericMemoryStat_t */ void**) = 0;
		virtual bool GameHasShutdownAndFlushedMemory( ) = 0;
		virtual int GetLastAcknowledgedCommand( ) = 0;
		virtual void FinishContainerWrites(int i) = 0;
		virtual void FinishAsyncSave( ) = 0;
		virtual int GetServerTick( ) = 0;
		virtual const char* GetModDirectory( ) = 0;
		virtual bool AudioLanguageChanged( ) = 0;
		virtual bool IsAutoSaveInProgress( ) = 0;
		virtual void StartLoadingScreenForCommand(const char* command) = 0;
		virtual void StartLoadingScreenForKeyValues(KeyValues* values) = 0;
		virtual void SOSSetOpvarFloat(const char*, float) = 0;
		virtual void SOSGetOpvarFloat(const char*, float&) = 0;
		virtual bool IsSubscribedMap(const char*, bool) = 0;
		virtual bool IsFeaturedMap(const char*, bool) = 0;
		virtual void GetDemoPlaybackParameters( ) = 0;
		virtual int GetClientVersion( ) = 0;
		virtual bool IsDemoSkipping( ) = 0;
		virtual void SetDemoImportantEventData(const KeyValues* values) = 0;
		virtual void ClearEvents( ) = 0;
		virtual int GetSafeZoneXMin( ) = 0;
		virtual bool IsVoiceRecording( ) = 0;
		virtual void ForceVoiceRecordOn( ) = 0;
		virtual bool IsReplay( ) = 0;
	};
}
