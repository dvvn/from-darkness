#pragma once

#include <fd/valve/recv_table.h>

namespace fd
{
namespace valve
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

    create_fn       create;
    create_event_fn create_event;
    const char*     name;
    recv_table*     table;
    client_class*   next;
    known_ids       id;
};
} // namespace valve

template <typename>
class range_view;

template <>
class range_view<const valve::client_class*>
{
    using ptr_type = const valve::client_class*;

    struct iterator
    {
        using reference = const valve::client_class&;

      private:
        ptr_type ptr_;

      public:
        iterator(const ptr_type ptr = nullptr)
            : ptr_(ptr)
        {
        }

        reference operator*() const
        {
            return *ptr_;
        }

        iterator& operator++()
        {
            ptr_ = ptr_->next;
            return *this;
        }

        iterator operator++(int)
        {
            auto ret = *this;
            ptr_     = ptr_->next;
            return ret;
        }

        bool operator==(const iterator&) const = default;
    };

    ptr_type begin_;

  public:
    range_view(const ptr_type begin)
        : begin_(begin)
    {
    }

    iterator begin() const
    {
        return begin_;
    }

    iterator end() const
    {
        (void)this;
        return nullptr;
    }
};

template <std::convertible_to<const valve::client_class*> P>
range_view(P) -> range_view<const valve::client_class*>;

} // namespace fd