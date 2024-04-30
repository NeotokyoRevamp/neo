#include "cbase.h"
#include "weapon_neobasecombatweapon.h"

#include "in_buttons.h"

#ifdef GAME_DLL
#include "player.h"
#endif

#include "basecombatweapon_shared.h"

#include <initializer_list>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( NEOBaseCombatWeapon, DT_NEOBaseCombatWeapon )

BEGIN_NETWORK_TABLE( CNEOBaseCombatWeapon, DT_NEOBaseCombatWeapon )
#ifdef CLIENT_DLL
	RecvPropTime(RECVINFO(m_flSoonestAttack)),
	RecvPropTime(RECVINFO(m_flLastAttackTime)),
	RecvPropFloat(RECVINFO(m_flAccuracyPenalty)),
	RecvPropInt(RECVINFO(m_nNumShotsFired)),
#else
	SendPropTime(SENDINFO(m_flSoonestAttack)),
	SendPropTime(SENDINFO(m_flLastAttackTime)),
	SendPropFloat(SENDINFO(m_flAccuracyPenalty)),
	SendPropInt(SENDINFO(m_nNumShotsFired)),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CNEOBaseCombatWeapon)
	DEFINE_PRED_FIELD(m_flSoonestAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_flLastAttackTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_flAccuracyPenalty, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_nNumShotsFired, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS(neobasecombatweapon, CNEOBaseCombatWeapon);

#ifdef GAME_DLL
BEGIN_DATADESC( CNEOBaseCombatWeapon )
	DEFINE_FIELD(m_flSoonestAttack, FIELD_TIME),
	DEFINE_FIELD(m_flLastAttackTime, FIELD_TIME),
	DEFINE_FIELD(m_flAccuracyPenalty, FIELD_FLOAT),
	DEFINE_FIELD(m_nNumShotsFired, FIELD_INTEGER),
END_DATADESC()
#endif

const char *GetWeaponByLoadoutId(int id)
{
	if (id < 0 || id >= NEO_WEP_LOADOUT_ID_COUNT)
	{
		Assert(false);
		return "";
	}

	const char* weps[] = {
		"weapon_mpn",
		"weapon_srm",
		"weapon_srm_s",
		"weapon_jitte",
		"weapon_jittescoped",
		"weapon_zr68c",
		"weapon_zr68s",
		"weapon_zr68l",
		"weapon_mx",
		"weapon_pz",
		"weapon_supa7",
		"weapon_m41",
		"weapon_m41l",
	};

	COMPILE_TIME_ASSERT(NEO_WEP_LOADOUT_ID_COUNT == ARRAYSIZE(weps));

	return weps[id];
}

CNEOBaseCombatWeapon::CNEOBaseCombatWeapon( void )
{
	m_bReadyToAimIn = false;
}

void CNEOBaseCombatWeapon::Spawn()
{
	// If this fires, either the enum bit mask has overflowed,
	// this derived gun has no valid NeoBitFlags set,
	// or we are spawning an instance of this base class for some reason.
	Assert(GetNeoWepBits() > NEO_WEP_INVALID);

	BaseClass::Spawn();

#ifdef GAME_DLL
	AddSpawnFlags(SF_NORESPAWN);
#endif
}

bool CNEOBaseCombatWeapon::Reload( void )
{
	return BaseClass::Reload();

#if(0)
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if (!pOwner)
	{
		return false;
	}

	if (pOwner->m_afButtonPressed & IN_RELOAD)
	{
		return DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
	}

#ifdef CLIENT_DLL
	if (!ClientWantsAutoReload())
	{
		return false;
	}
#else
	if (!ClientWantsAutoReload(pOwner))
	{
		return false;
	}
#endif

	return DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
#endif
}

bool CNEOBaseCombatWeapon::CanBeSelected(void)
{
	if (GetWeaponFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY)
	{
		return true;
	}

	return BaseClass::CanBeSelected();
}

bool CNEOBaseCombatWeapon::Deploy(void)
{
	const bool ret = BaseClass::Deploy();

	if (ret)
	{
		m_bReadyToAimIn = false;

#ifdef DEBUG
		CNEO_Player* pOwner = NULL;
		if (GetOwner())
		{
			pOwner = dynamic_cast<CNEO_Player*>(GetOwner());
			Assert(pOwner);
		}
#else
		auto pOwner = static_cast<CNEO_Player*>(GetOwner());
#endif

		if (pOwner)
		{
			if (pOwner->GetFlags() & FL_DUCKING)
			{
				pOwner->SetMaxSpeed(pOwner->GetCrouchSpeed_WithWepEncumberment(this));
			}
			else if (pOwner->IsWalking())
			{
				pOwner->SetMaxSpeed(pOwner->GetWalkSpeed_WithWepEncumberment(this));
			}
			else if (pOwner->IsSprinting())
			{
				pOwner->SetMaxSpeed(pOwner->GetSprintSpeed_WithWepEncumberment(this));
			}
			else
			{
				pOwner->SetMaxSpeed(pOwner->GetNormSpeed_WithWepEncumberment(this));
			}
		}
	}

	return ret;
}

#ifdef CLIENT_DLL
bool CNEOBaseCombatWeapon::Holster(CBaseCombatWeapon* pSwitchingTo)
{
#ifdef DEBUG
	CNEO_Player* pOwner = NULL;
	if (GetOwner())
	{
		pOwner = dynamic_cast<CNEO_Player*>(GetOwner());
		Assert(pOwner);
	}
#else
	auto pOwner = static_cast<CNEO_Player*>(GetOwner());
#endif

	if (pOwner)
	{
		pOwner->Weapon_SetZoom(false);
	}

	return BaseClass::Holster(pSwitchingTo);
}
#endif

void CNEOBaseCombatWeapon::CheckReload(void)
{
	if (!m_bInReload && UsesClipsForAmmo1() && m_iClip1 == 0 && GetOwner() && !ClientWantsAutoReload(GetOwner()))
	{
		return;
	}

	BaseClass::CheckReload();
}

void CNEOBaseCombatWeapon::ItemPreFrame(void)
{
	if (!m_bReadyToAimIn)
	{
		if (gpGlobals->curtime >= m_flNextPrimaryAttack)
		{
			m_bReadyToAimIn = true;
		}
	}
}

ConVar sv_neo_wep_acc_penalty_scale("sv_neo_wep_acc_penalty_scale", "7.5", FCVAR_REPLICATED,
	"Temporary global neo wep accuracy penalty scaler.", true, 0.01, true, 9999.0);

ConVar sv_neo_wep_cone_min_scale("sv_neo_wep_cone_min_scale", "0.01", FCVAR_REPLICATED,
	"Temporary global neo wep bloom min cone scaler.", true, 0.01, true, 10.0);

ConVar sv_neo_wep_cone_max_scale("sv_neo_wep_cone_max_scale", "0.7", FCVAR_REPLICATED,
	"Temporary global neo wep bloom max cone scaler.", true, 0.01, true, 10.0);

// NEO HACK/FIXME (Rain): Doing some temporary bloom accuracy scaling here for easier testing.
// Need to clean this up later once we have good values!!
#define TEMP_WEP_STR(name) #name
#define MAKE_TEMP_WEP_BLOOM_SCALER(weapon, defval) ConVar sv_neo_##weapon##_bloom_scale(TEMP_WEP_STR(sv_neo_##weapon##_bloom_scale), #defval, FCVAR_REPLICATED, TEMP_WEP_STR(Temporary weapon bloom scaler for #weapon), true, 0.01, true, 9999.0)

MAKE_TEMP_WEP_BLOOM_SCALER(weapon_jitte,			2);
MAKE_TEMP_WEP_BLOOM_SCALER(weapon_jittescoped,		2.5);
MAKE_TEMP_WEP_BLOOM_SCALER(weapon_kyla,				2.5);
MAKE_TEMP_WEP_BLOOM_SCALER(weapon_m41,				2.5);
MAKE_TEMP_WEP_BLOOM_SCALER(weapon_m41l,				2.5);
MAKE_TEMP_WEP_BLOOM_SCALER(weapon_m41s,				2.5);
MAKE_TEMP_WEP_BLOOM_SCALER(weapon_milso,			2.5);
MAKE_TEMP_WEP_BLOOM_SCALER(weapon_mpn,				20.0);
MAKE_TEMP_WEP_BLOOM_SCALER(weapon_mpn_unsilenced,	4.0);
MAKE_TEMP_WEP_BLOOM_SCALER(weapon_mx,				2.5);
MAKE_TEMP_WEP_BLOOM_SCALER(weapon_mx_silenced,		2.5);
MAKE_TEMP_WEP_BLOOM_SCALER(weapon_pz,				2.5);
MAKE_TEMP_WEP_BLOOM_SCALER(weapon_smac,				2.5);
MAKE_TEMP_WEP_BLOOM_SCALER(weapon_srm,				2.5);
MAKE_TEMP_WEP_BLOOM_SCALER(weapon_srm_s,			2.5);
MAKE_TEMP_WEP_BLOOM_SCALER(weapon_tachi,			2.5);
MAKE_TEMP_WEP_BLOOM_SCALER(weapon_zr68c,			2.5);
MAKE_TEMP_WEP_BLOOM_SCALER(weapon_zr68s,			2.5);
#ifdef INCLUDE_WEP_PBK
MAKE_TEMP_WEP_BLOOM_SCALER(weapon_pbk56s,			2.5);
#endif

const Vector& CNEOBaseCombatWeapon::GetBulletSpread(void)
{
	static Vector cone;

	// NEO HACK/FIXME (Rain): Doing some temporary bloom accuracy scaling here for easier testing.
	// Need to clean this up later once we have good values!!
	const std::initializer_list<ConVar*> bloomScalers = {
		&sv_neo_weapon_jitte_bloom_scale,
		&sv_neo_weapon_jittescoped_bloom_scale,
		&sv_neo_weapon_kyla_bloom_scale,
		&sv_neo_weapon_m41_bloom_scale,
		&sv_neo_weapon_m41l_bloom_scale,
		&sv_neo_weapon_m41s_bloom_scale,
		&sv_neo_weapon_milso_bloom_scale,
		&sv_neo_weapon_mpn_bloom_scale,
		&sv_neo_weapon_mpn_unsilenced_bloom_scale,
		&sv_neo_weapon_mx_bloom_scale,
		&sv_neo_weapon_mx_silenced_bloom_scale,
		&sv_neo_weapon_pz_bloom_scale,
		&sv_neo_weapon_smac_bloom_scale,
		&sv_neo_weapon_srm_bloom_scale,
		&sv_neo_weapon_srm_s_bloom_scale,
		&sv_neo_weapon_tachi_bloom_scale,
		&sv_neo_weapon_zr68c_bloom_scale,
		&sv_neo_weapon_zr68s_bloom_scale,
#ifdef INCLUDE_WEP_PBK
		& sv_neo_weapon_pbk56s_bloom_scale,
#endif
	};
	float wepSpecificBloomScale = 1.0f;
	for (ConVar* scaler : bloomScalers)
	{
		if (V_strstr(scaler->GetName(), GetName()))
		{
			wepSpecificBloomScale = scaler->GetFloat();
			break;
		}
	}

	Assert(GetInnateInaccuracy() <= GetMaxAccuracyPenalty());

	const float ramp = RemapValClamped(m_flAccuracyPenalty,
		GetInnateInaccuracy(),
		GetMaxAccuracyPenalty() * sv_neo_wep_acc_penalty_scale.GetFloat(),
		0.0f,
		1.0f);

	// We lerp from very accurate to inaccurate over time
	VectorLerp(
		GetMinCone() * sv_neo_wep_cone_min_scale.GetFloat(),
		GetMaxCone() * sv_neo_wep_cone_max_scale.GetFloat() * wepSpecificBloomScale,
		ramp,
		cone);

	return cone;
}

void CNEOBaseCombatWeapon::PrimaryAttack(void)
{
	Assert(!ShootingIsPrevented());

	if (gpGlobals->curtime < m_flSoonestAttack)
	{
		return;
	}
	// Can't shoot again yet
	else if (gpGlobals->curtime - m_flLastAttackTime < GetFireRate())
	{
		return;
	}
	else if (m_iClip1 == 0)
	{
		if (!m_bFireOnEmpty)
		{
			CheckReload();
		}
		else
		{
			WeaponSound(EMPTY);
			SendWeaponAnim(ACT_VM_DRYFIRE);
			m_flNextPrimaryAttack = 0.2;
		}
		return;
	}

	auto pOwner = ToBasePlayer(GetOwner());

	if (!pOwner)
	{
		Assert(false);
		return;
	}
	else if (m_iClip1 == 0 && !ClientWantsAutoReload(pOwner))
	{
		return;
	}

	if (IsSemiAuto())
	{
		// Do nothing if we hold fire whilst semi auto
		if ((pOwner->m_afButtonLast & IN_ATTACK) &&
			(pOwner->m_nButtons & IN_ATTACK))
		{
			return;
		}
	}

	pOwner->ViewPunchReset();

	if ((gpGlobals->curtime - m_flLastAttackTime) > 0.5f)
	{
		m_nNumShotsFired = 0;
	}
	else
	{
		++m_nNumShotsFired;
	}

	m_flLastAttackTime = gpGlobals->curtime;

	BaseClass::PrimaryAttack();

	m_flAccuracyPenalty += GetAccuracyPenalty();
}
