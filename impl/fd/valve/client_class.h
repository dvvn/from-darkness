#pragma once

#include <fd/valve/recv_table.h>

namespace fd::valve
{
    struct client_networkable;

    struct client_class
    {
        using create_fn       = client_networkable* (*)(int ent_num, int serial_num);
        using create_event_fn = client_networkable* (*)();

        enum known_ids
        {
            CAI_BaseNPC = 0,
            CAK47,
            CBaseAnimating,
            CBaseAnimatingOverlay,
            CBaseAttributableItem,
            CBaseButton,
            CBaseCombatCharacter,
            CBaseCombatWeapon,
            CBaseCSGrenade,
            CBaseCSGrenadeProjectile,
            CBaseDoor,
            CBaseEntity,
            CBaseFlex,
            CBaseGrenade,
            CBaseParticleEntity,
            CBasePlayer,
            CBasePropDoor,
            CBaseTeamObjectiveResource,
            CBaseTempEntity,
            CBaseToggle,
            CBaseTrigger,
            CBaseViewModel,
            CBaseVPhysicsTrigger,
            CBaseWeaponWorldModel,
            CBeam,
            CBeamSpotlight,
            CBoneFollower,
            CBRC4Target,
            CBreachCharge,
            CBreachChargeProjectile,
            CBreakableProp,
            CBreakableSurface,
            CBumpMine,
            CBumpMineProjectile,
            CC4,
            CCascadeLight,
            CChicken,
            CColorCorrection,
            CColorCorrectionVolume,
            CCSGameRulesProxy,
            CCSPlayer,
            CCSPlayerResource,
            CCSRagdoll,
            CCSTeam,
            CDangerZone,
            CDangerZoneController,
            CDEagle,
            CDecoyGrenade,
            CDecoyProjectile,
            CDrone,
            CDronegun,
            CDynamicLight,
            CDynamicProp,
            CEconEntity,
            CEconWearable,
            CEmbers,
            CEntityDissolve,
            CEntityFlame,
            CEntityFreezing,
            CEntityParticleTrail,
            CEnvAmbientLight,
            CEnvDetailController,
            CEnvDOFController,
            CEnvGasCanister,
            CEnvParticleScript,
            CEnvProjectedTexture,
            CEnvQuadraticBeam,
            CEnvScreenEffect,
            CEnvScreenOverlay,
            CEnvTonemapController,
            CEnvWind,
            CFEPlayerDecal,
            CFireCrackerBlast,
            CFireSmoke,
            CFireTrail,
            CFish,
            CFists,
            CFlashbang,
            CFogController,
            CFootstepControl,
            CFunc_Dust,
            CFunc_LOD,
            CFuncAreaPortalWindow,
            CFuncBrush,
            CFuncConveyor,
            CFuncLadder,
            CFuncMonitor,
            CFuncMoveLinear,
            CFuncOccluder,
            CFuncReflectiveGlass,
            CFuncRotating,
            CFuncSmokeVolume,
            CFuncTrackTrain,
            CGameRulesProxy,
            CGrassBurn,
            CHandleTest,
            CHEGrenade,
            CHostage,
            CHostageCarriableProp,
            CIncendiaryGrenade,
            CInferno,
            CInfoLadderDismount,
            CInfoMapRegion,
            CInfoOverlayAccessor,
            CItem_Healthshot,
            CItemCash,
            CItemDogtags,
            CKnife,
            CKnifeGG,
            CLightGlow,
            CMaterialModifyControl,
            CMelee,
            CMolotovGrenade,
            CMolotovProjectile,
            CMovieDisplay,
            CParadropChopper,
            CParticleFire,
            CParticlePerformanceMonitor,
            CParticleSystem,
            CPhysBox,
            CPhysBoxMultiplayer,
            CPhysicsProp,
            CPhysicsPropMultiplayer,
            CPhysMagnet,
            CPhysPropAmmoBox,
            CPhysPropLootCrate,
            CPhysPropRadarJammer,
            CPhysPropWeaponUpgrade,
            CPlantedC4,
            CPlasma,
            CPlayerPing,
            CPlayerResource,
            CPointCamera,
            CPointCommentaryNode,
            CPointWorldText,
            CPoseController,
            CPostProcessController,
            CPrecipitation,
            CPrecipitationBlocker,
            CPredictedViewModel,
            CProp_Hallucination,
            CPropCounter,
            CPropDoorRotating,
            CPropJeep,
            CPropVehicleDriveable,
            CRagdollManager,
            CRagdollProp,
            CRagdollPropAttached,
            CRopeKeyframe,
            CSCAR17,
            CSceneEntity,
            CSensorGrenade,
            CSensorGrenadeProjectile,
            CShadowControl,
            CSlideshowDisplay,
            CSmokeGrenade,
            CSmokeGrenadeProjectile,
            CSmokeStack,
            CSnowball,
            CSnowballPile,
            CSnowballProjectile,
            CSpatialEntity,
            CSpotlightEnd,
            CSprite,
            CSpriteOriented,
            CSpriteTrail,
            CStatueProp,
            CSteamJet,
            CSun,
            CSunlightShadowControl,
            CSurvivalSpawnChopper,
            CTablet,
            CTeam,
            CTeamplayRoundBasedRulesProxy,
            CTEArmorRicochet,
            CTEBaseBeam,
            CTEBeamEntPoint,
            CTEBeamEnts,
            CTEBeamFollow,
            CTEBeamLaser,
            CTEBeamPoints,
            CTEBeamRing,
            CTEBeamRingPoint,
            CTEBeamSpline,
            CTEBloodSprite,
            CTEBloodStream,
            CTEBreakModel,
            CTEBSPDecal,
            CTEBubbles,
            CTEBubbleTrail,
            CTEClientProjectile,
            CTEDecal,
            CTEDust,
            CTEDynamicLight,
            CTEEffectDispatch,
            CTEEnergySplash,
            CTEExplosion,
            CTEFireBullets,
            CTEFizz,
            CTEFootprintDecal,
            CTEFoundryHelpers,
            CTEGaussExplosion,
            CTEGlowSprite,
            CTEImpact,
            CTEKillPlayerAttachments,
            CTELargeFunnel,
            CTEMetalSparks,
            CTEMuzzleFlash,
            CTEParticleSystem,
            CTEPhysicsProp,
            CTEPlantBomb,
            CTEPlayerAnimEvent,
            CTEPlayerDecal,
            CTEProjectedDecal,
            CTERadioIcon,
            CTEShatterSurface,
            CTEShowLine,
            CTesla,
            CTESmoke,
            CTESparks,
            CTESprite,
            CTESpriteSpray,
            CTest_ProxyToggle_Networkable,
            CTestTraceline,
            CTEWorldDecal,
            CTriggerPlayerMovement,
            CTriggerSoundOperator,
            CVGuiScreen,
            CVoteController,
            CWaterBullet,
            CWaterLODControl,
            CWeaponAug,
            CWeaponAWP,
            CWeaponBaseItem,
            CWeaponBizon,
            CWeaponCSBase,
            CWeaponCSBaseGun,
            CWeaponCycler,
            CWeaponElite,
            CWeaponFamas,
            CWeaponFiveSeven,
            CWeaponG3SG1,
            CWeaponGalil,
            CWeaponGalilAR,
            CWeaponGlock,
            CWeaponHKP2000,
            CWeaponM249,
            CWeaponM3,
            CWeaponM4A1,
            CWeaponMAC10,
            CWeaponMag7,
            CWeaponMP5Navy,
            CWeaponMP7,
            CWeaponMP9,
            CWeaponNegev,
            CWeaponNOVA,
            CWeaponP228,
            CWeaponP250,
            CWeaponP90,
            CWeaponSawedoff,
            CWeaponSCAR20,
            CWeaponScout,
            CWeaponSG550,
            CWeaponSG552,
            CWeaponSG556,
            CWeaponShield,
            CWeaponSSG08,
            CWeaponTaser,
            CWeaponTec9,
            CWeaponTMP,
            CWeaponUMP45,
            CWeaponUSP,
            CWeaponXM1014,
            CWorld,
            CWorldVguiText,
            DustTrail,
            MovieExplosion,
            ParticleSmokeGrenade,
            RocketTrail,
            SmokeTrail,
            SporeExplosion,
            SporeTrail,
        };

        //-----

        create_fn create;
        create_event_fn create_event;
        const char* name;
        recv_table* table;
        client_class* next;
        known_ids id;
    };

    class client_class_iterator
    {
        const client_class* ptr_;

      public:
#ifdef __cpp_lib_concepts
        using iterator_concept = std::contiguous_iterator_tag;
#endif
        using iterator_category = std::forward_iterator_tag;
        using value_type        = const client_class;
        using difference_type   = ptrdiff_t;
        using pointer           = const client_class*;
        using reference         = value_type&;

        client_class_iterator(const client_class* ptr = nullptr)
            : ptr_(ptr)
        {
        }

        const client_class& operator*() const
        {
            return *ptr_;
        }

        client_class_iterator& operator++()
        {
            ptr_ = ptr_->next;
            return *this;
        }

        client_class_iterator operator++(int)
        {
            auto ret = *this;
            operator++();
            return ret;
        }

        bool operator==(const client_class_iterator&) const = default;
    };

    class client_class_range
    {
        const client_class* start_;

      public:
        using value_type      = const client_class&;
        using difference_type = ptrdiff_t;

        client_class_range(const client_class* start = nullptr)
            : start_(start)
        {
        }

        client_class_iterator begin() const
        {
            return start_;
        }

        client_class_iterator end() const
        {
            return nullptr;
        }
    };

} // namespace fd::valve
