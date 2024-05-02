#include "cbase.h"
#include "weapon_zr68l.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponZR68L, DT_WeaponZR68L)

BEGIN_NETWORK_TABLE(CWeaponZR68L, DT_WeaponZR68L)
	DEFINE_NEO_BASE_WEP_NETWORK_TABLE
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponZR68L)
	DEFINE_NEO_BASE_WEP_PREDICTION
END_PREDICTION_DATA()
#endif

NEO_IMPLEMENT_ACTTABLE(CWeaponZR68L)

LINK_ENTITY_TO_CLASS(weapon_zr68l, CWeaponZR68L);

PRECACHE_WEAPON_REGISTER(weapon_zr68l);

CWeaponZR68L::CWeaponZR68L()
{
	m_flSoonestAttack = gpGlobals->curtime;
	m_flAccuracyPenalty = 0;

	m_nNumShotsFired = 0;
}

void CWeaponZR68L::DryFire()
{
	WeaponSound(EMPTY);
	SendWeaponAnim(ACT_VM_DRYFIRE);

	m_flNextPrimaryAttack = gpGlobals->curtime + GetFastestDryRefireTime();
}

void CWeaponZR68L::Spawn()
{
	BaseClass::Spawn();
}

bool CWeaponZR68L::Deploy(void)
{
	return BaseClass::Deploy();
}

void CWeaponZR68L::UpdatePenaltyTime()
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	// Check our penalty time decay
	if ((pOwner->m_nButtons & IN_ATTACK) == false)
	{
		if (m_flSoonestAttack < gpGlobals->curtime)
		{
			m_flAccuracyPenalty -= gpGlobals->frametime;
			m_flAccuracyPenalty = clamp(m_flAccuracyPenalty, 0.0f, GetMaxAccuracyPenalty());
		}
	}
	else
	{
		m_flSoonestAttack = gpGlobals->curtime + GetFireRate();
	}

	if (m_flSoonestAttack > gpGlobals->curtime)
	{
		m_flSoonestAttack -= (gpGlobals->curtime - m_flLastAttackTime);
	}
}

void CWeaponZR68L::ItemPreFrame()
{
	UpdatePenaltyTime();

	BaseClass::ItemPreFrame();
}

void CWeaponZR68L::ItemBusyFrame()
{
	UpdatePenaltyTime();

	BaseClass::ItemBusyFrame();
}

void CWeaponZR68L::ItemPostFrame()
{
	BaseClass::ItemPostFrame();
}

Activity CWeaponZR68L::GetPrimaryAttackActivity()
{
	if (m_nNumShotsFired < 1)
	{
		return ACT_VM_PRIMARYATTACK;
	}

	if (m_nNumShotsFired < 2)
	{
		return ACT_VM_RECOIL1;
	}

	if (m_nNumShotsFired < 3)
	{
		return ACT_VM_RECOIL2;
	}

	return ACT_VM_RECOIL3;
}

void CWeaponZR68L::AddViewKick()
{
	auto owner = ToBasePlayer(GetOwner());

	if (!owner)
	{
		return;
	}

	QAngle viewPunch;

	viewPunch.x = SharedRandomFloat("zr68lpx", 0.25f, 0.5f);
	viewPunch.y = SharedRandomFloat("zr68lpy", -0.6f, 0.6f);
	viewPunch.z = 0;

	owner->ViewPunch(viewPunch);
}
