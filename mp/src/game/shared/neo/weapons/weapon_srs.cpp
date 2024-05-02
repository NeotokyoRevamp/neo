#include "cbase.h"
#include "weapon_srs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponSRS, DT_WeaponSRS)

BEGIN_NETWORK_TABLE(CWeaponSRS, DT_WeaponSRS)
	DEFINE_NEO_BASE_WEP_NETWORK_TABLE
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA(CWeaponSRS)
	DEFINE_NEO_BASE_WEP_PREDICTION
END_PREDICTION_DATA()
#endif

NEO_IMPLEMENT_ACTTABLE(CWeaponSRS)

LINK_ENTITY_TO_CLASS(weapon_srs, CWeaponSRS);

PRECACHE_WEAPON_REGISTER(weapon_srs);

CWeaponSRS::CWeaponSRS()
{
	m_flSoonestAttack = gpGlobals->curtime;
	m_flAccuracyPenalty = 0;

	m_nNumShotsFired = 0;
}

void CWeaponSRS::DryFire()
{
	WeaponSound(EMPTY);
	SendWeaponAnim(ACT_VM_DRYFIRE);

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

void CWeaponSRS::Spawn()
{
	BaseClass::Spawn();
}

bool CWeaponSRS::Deploy(void)
{
	return BaseClass::Deploy();
}

void CWeaponSRS::UpdatePenaltyTime()
{
	auto owner = ToBasePlayer(GetOwner());

	if (!owner)
	{
		return;
	}

	if ((owner->m_nButtons & IN_ATTACK) == false)
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

void CWeaponSRS::ItemPreFrame()
{
	UpdatePenaltyTime();

	BaseClass::ItemPreFrame();
}

void CWeaponSRS::ItemBusyFrame()
{
	UpdatePenaltyTime();

	BaseClass::ItemBusyFrame();
}

void CWeaponSRS::ItemPostFrame()
{
	BaseClass::ItemPostFrame();
}

Activity CWeaponSRS::GetPrimaryAttackActivity()
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

void CWeaponSRS::AddViewKick()
{
	auto owner = ToBasePlayer(GetOwner());

	if (!owner)
	{
		return;
	}

	QAngle viewPunch;

	viewPunch.x = SharedRandomFloat("srspx", 0.25f, 0.5f);
	viewPunch.y = SharedRandomFloat("srspy", -0.6f, 0.6f);
	viewPunch.z = 0;

	owner->ViewPunch(viewPunch);
}
