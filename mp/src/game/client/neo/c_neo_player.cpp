#include "cbase.h"
#include "vcollide_parse.h"
#include "c_neo_player.h"
#include "view.h"
#include "takedamageinfo.h"
#include "neo_gamerules.h"
#include "in_buttons.h"
#include "iviewrender_beams.h"			// flashlight beam
#include "r_efx.h"
#include "dlight.h"

#include "iinput.h"

#include "clientmode_hl2mpnormal.h"
#include <vgui/IScheme.h>
#include <vgui_controls/Panel.h>

#include "hud_crosshair.h"

#include "neo_predicted_viewmodel.h"

#include "game_controls/neo_teammenu.h"

#include "ui/neo_hud_compass.h"
#include "ui/neo_hud_game_event.h"
#include "ui/neo_hud_ghost_marker.h"
#include "ui/neo_hud_friendly_marker.h"
#include "ui/neo_hud_elements.h"

#include "neo/game_controls/neo_loadoutmenu.h"

#include "baseviewmodel_shared.h"

#include "prediction.h"

#include "neo/weapons/weapon_ghost.h"
#include "neo/weapons/weapon_supa7.h"

#include <engine/ivdebugoverlay.h>
#include <engine/IEngineSound.h>

#include <materialsystem/imaterialsystem.h>
#include <materialsystem/itexture.h>
#include "rendertexture.h"

#include "model_types.h"

#include "neo_playeranimstate.h"

// Don't alias here
#if defined( CNEO_Player )
#undef CNEO_Player	
#endif

LINK_ENTITY_TO_CLASS(player, C_NEO_Player);

IMPLEMENT_CLIENTCLASS_DT(C_NEO_Player, DT_NEO_Player, CNEO_Player)
	RecvPropInt(RECVINFO(m_iNeoClass)),
	RecvPropInt(RECVINFO(m_iNeoSkin)),
	RecvPropInt(RECVINFO(m_iNeoStar)),

	RecvPropBool(RECVINFO(m_bShowTestMessage)),
	RecvPropString(RECVINFO(m_pszTestMessage)),

	RecvPropInt(RECVINFO(m_iXP)),
	RecvPropInt(RECVINFO(m_iCapTeam)),
	RecvPropInt(RECVINFO(m_iLoadoutWepChoice)),
	RecvPropInt(RECVINFO(m_iNextSpawnClassChoice)),

	RecvPropVector(RECVINFO(m_vecGhostMarkerPos)),
	RecvPropInt(RECVINFO(m_iGhosterTeam)),
	RecvPropBool(RECVINFO(m_bGhostExists)),
	RecvPropBool(RECVINFO(m_bInThermOpticCamo)),
	RecvPropBool(RECVINFO(m_bLastTickInThermOpticCamo)),
	RecvPropBool(RECVINFO(m_bInVision)),
	RecvPropBool(RECVINFO(m_bHasBeenAirborneForTooLongToSuperJump)),
	RecvPropBool(RECVINFO(m_bInAim)),

	RecvPropTime(RECVINFO(m_flCamoAuxLastTime)),
	RecvPropInt(RECVINFO(m_nVisionLastTick)),

	RecvPropArray(RecvPropVector(RECVINFO(m_rvFriendlyPlayerPositions[0])), m_rvFriendlyPlayerPositions),
	RecvPropArray(RecvPropFloat(RECVINFO(m_rfAttackersScores[0])), m_rfAttackersScores),

	RecvPropInt(RECVINFO(m_NeoFlags)),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA(C_NEO_Player)
	DEFINE_PRED_FIELD(m_rvFriendlyPlayerPositions, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_rfAttackersScores, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_vecGhostMarkerPos, FIELD_VECTOR, FTYPEDESC_INSENDTABLE),

	DEFINE_PRED_FIELD_TOL(m_flCamoAuxLastTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),
	
	DEFINE_PRED_FIELD(m_bInThermOpticCamo, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_bLastTickInThermOpticCamo, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_bInAim, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_bInVision, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),
	DEFINE_PRED_FIELD(m_bHasBeenAirborneForTooLongToSuperJump, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE),

	DEFINE_PRED_FIELD(m_nVisionLastTick, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()

ConVar cl_drawhud_quickinfo("cl_drawhud_quickinfo", "0", 0,
	"Whether to display HL2 style ammo/health info near crosshair.",
	true, 0.0f, true, 1.0f);

class NeoLoadoutMenu_Cb : public ICommandCallback
{
public:
	virtual void CommandCallback(const CCommand& command)
	{
#if DEBUG
		DevMsg("Loadout access cb\n");
#endif

		auto panel = dynamic_cast<vgui::EditablePanel*>(GetClientModeNormal()->
			GetViewport()->FindChildByName(PANEL_NEO_LOADOUT));

		if (!panel)
		{
			Assert(false);
			Warning("Couldn't find weapon loadout panel\n");
			return;
		}

		panel->ApplySchemeSettings(vgui::scheme()->GetIScheme(panel->GetScheme()));

		int panelWide = 960, panelTall = 700, screenWide, screenTall;
		surface()->GetScreenSize(screenWide, screenTall);
		// Resize panel, but make sure it fits the resolution.
		panel->SetSize(Min(screenWide, panelWide), Min(screenTall, panelTall));
		panel->SetPos((screenWide / 2) - (panelWide / 2),
			(screenTall / 2) - (panelTall / 2));

		panel->SetMouseInputEnabled(true);
		panel->SetKeyBoardInputEnabled(true);
		panel->SetCursorAlwaysVisible(true);

		panel->SetControlEnabled("Scout_Button", true);
		panel->SetControlEnabled("Misc2", true);
		panel->SetControlEnabled("Done_Button", true);
		panel->SetControlEnabled("Button1", true);
		panel->SetControlEnabled("Button2", true);
		panel->SetControlEnabled("Button3", true);
		panel->SetControlEnabled("Button4", true);
		panel->SetControlEnabled("Button5", true);
		panel->SetControlEnabled("Button6", true);
		panel->SetControlEnabled("Button7", true);
		panel->SetControlEnabled("Button8", true);
		panel->SetControlEnabled("Button9", true);
		panel->SetControlEnabled("Button10", true);
		panel->SetControlEnabled("Button11", true);
		panel->SetControlEnabled("Button12", true);
		panel->SetControlEnabled("Button13", true);
		panel->SetControlEnabled("Button14", true);

		panel->MoveToFront();

		if (panel->IsKeyBoardInputEnabled())
		{
			panel->RequestFocus();
		}

		panel->SetVisible(true);
		panel->SetEnabled(true);

		auto loadoutPanel = dynamic_cast<CNeoLoadoutMenu*>(panel);
		if (loadoutPanel)
		{
			loadoutPanel->ShowPanel(true);
		}
		else
		{
			Assert(false);
			DevWarning("Cast failed\n");
		}

		surface()->SetMinimized(panel->GetVPanel(), false);
	}
};
NeoLoadoutMenu_Cb neoLoadoutMenu_Cb;

class NeoClassMenu_Cb : public ICommandCallback
{
public:
	virtual void CommandCallback(const CCommand& command)
	{
		vgui::EditablePanel *panel = dynamic_cast<vgui::EditablePanel*>
			(GetClientModeNormal()->GetViewport()->FindChildByName(PANEL_CLASS));

		if (!panel)
		{
			Assert(false);
			Warning("Couldn't find class panel\n");
			return;
		}

		panel->ApplySchemeSettings(vgui::scheme()->GetIScheme(panel->GetScheme()));

		int panelWide = 650, panelTall = 280, screenWide, screenTall;
		surface()->GetScreenSize(screenWide, screenTall);
		// Resize panel, but make sure it fits the resolution.
		panel->SetSize(Min(screenWide, panelWide), Min(screenTall, panelTall));
		panel->SetPos((screenWide / 2) - (panelWide / 2),
			(screenTall / 2) - (panelTall / 2));

		panel->SetMouseInputEnabled(true);
		panel->SetKeyBoardInputEnabled(true);
		panel->SetCursorAlwaysVisible(true);

		panel->SetControlEnabled("Scout_Button", true);
		panel->SetControlEnabled("Assault_Button", true);
		panel->SetControlEnabled("Heavy_Button", true);
		panel->SetControlEnabled("Back_Button", true);

		panel->MoveToFront();

		if (panel->IsKeyBoardInputEnabled())
		{
			panel->RequestFocus();
		}

		panel->SetVisible(true);
		panel->SetEnabled(true);

		surface()->SetMinimized(panel->GetVPanel(), false);
	}
};
NeoClassMenu_Cb neoClassMenu_Cb;

class NeoTeamMenu_Cb : public ICommandCallback
{
public:
	virtual void CommandCallback( const CCommand &command )
	{
		if (!g_pNeoTeamMenu)
		{
			Assert(false);
			DevWarning("CNeoTeamMenu is not ready\n");
			return;
		}

		vgui::EditablePanel *panel = dynamic_cast<vgui::EditablePanel*>
			(GetClientModeNormal()->GetViewport()->FindChildByName(PANEL_TEAM));
		if (!panel)
		{
			Assert(false);
			Warning("Couldn't find team panel\n");
			return;
		}

		panel->ApplySchemeSettings(vgui::scheme()->GetIScheme(panel->GetScheme()));

		int panelWide = 360, panelTall = 215, screenWide, screenTall;
		panelWide *= 1.5; panelTall *= 1.5;
		surface()->GetScreenSize(screenWide, screenTall);
		// Resize panel, but make sure it fits the resolution.
		panel->SetSize(Min(screenWide, panelWide), Min(screenTall, panelTall));
		panel->SetPos((screenWide / 2) - (panelWide / 2),
			(screenTall / 2) - (panelTall / 2));

		panel->SetMouseInputEnabled(true);
		panel->SetKeyBoardInputEnabled(true);
		panel->SetCursorAlwaysVisible(true);

		panel->SetControlEnabled("jinraibutton", true);
		panel->SetControlEnabled("nsfbutton", true);
		panel->SetControlEnabled("specbutton", true);
		panel->SetControlEnabled("autobutton", true);
		panel->SetControlEnabled("CancelButton", true);

		panel->MoveToFront();

		if (panel->IsKeyBoardInputEnabled())
		{
			panel->RequestFocus();
		}

		panel->SetVisible(true);
		panel->SetEnabled(true);

		surface()->SetMinimized(panel->GetVPanel(), false);
	}
};
NeoTeamMenu_Cb neoTeamMenu_Cb;

class VguiCancel_Cb : public ICommandCallback
{
public:
	virtual void CommandCallback(const CCommand& command)
	{
		auto player = C_NEO_Player::GetLocalNEOPlayer();
		Assert(player);
		if (player)
		{
			if (player->GetTeamNumber() <= TEAM_UNASSIGNED)
			{
				engine->ClientCmd("jointeam 0");
			}
		}
	}
};
VguiCancel_Cb vguiCancel_Cb;

ConCommand loadoutmenu("loadoutmenu", &neoLoadoutMenu_Cb, "Open weapon loadout selection menu.", FCVAR_USERINFO);
ConCommand classmenu("classmenu", &neoClassMenu_Cb, "Open class selection menu.", FCVAR_USERINFO);
ConCommand teammenu("teammenu", &neoTeamMenu_Cb, "Open team selection menu.", FCVAR_USERINFO);
ConCommand vguicancel("vguicancel", &vguiCancel_Cb, "Cancel current vgui screen.", FCVAR_USERINFO);

C_NEO_Player::C_NEO_Player()
{
	SetPredictionEligible(true);

	m_iNeoClass = NEO_CLASS_ASSAULT;
	m_iNeoSkin = NEO_SKIN_FIRST;
	m_iNeoStar = NEO_DEFAULT_STAR;

	m_iCapTeam = TEAM_UNASSIGNED;
	m_iLoadoutWepChoice = 0;
	m_iNextSpawnClassChoice = -1;
	m_iGhosterTeam = TEAM_UNASSIGNED;
	m_iXP.GetForModify() = 0;

	m_vecGhostMarkerPos = vec3_origin;
	m_bGhostExists = false;
	m_bShowClassMenu = m_bShowTeamMenu = m_bIsClassMenuOpen = m_bIsTeamMenuOpen = false;
	m_bInThermOpticCamo = m_bInVision = false;
	m_bHasBeenAirborneForTooLongToSuperJump = false;
	m_bInAim = false;

	m_pNeoPanel = NULL;

	m_flCamoAuxLastTime = 0;
	m_nVisionLastTick = 0;
	m_flLastAirborneJumpOkTime = 0;
	m_flLastSuperJumpTime = 0;

	m_bFirstDeathTick = true;
	m_bPreviouslyReloading = false;
	m_bPreviouslyPreparingToHideMsg = false;
	m_bLastTickInThermOpticCamo = false;
	m_bIsAllowedToToggleVision = false;

	m_pPlayerAnimState = CreatePlayerAnimState(this, CreateAnimStateHelpers(this),
		NEO_ANIMSTATE_LEGANIM_TYPE, NEO_ANIMSTATE_USES_AIMSEQUENCES);
}

C_NEO_Player::~C_NEO_Player()
{
	m_pPlayerAnimState->Release();
}

void C_NEO_Player::CheckThermOpticButtons()
{
	m_bLastTickInThermOpticCamo = m_bInThermOpticCamo;

	if ((m_afButtonPressed & IN_THERMOPTIC) && IsAlive())
	{
		if (GetClass() != NEO_CLASS_RECON && GetClass() != NEO_CLASS_ASSAULT)
		{
			return;
		}

		if (m_HL2Local.m_flSuitPower >= CLOAK_AUX_COST)
		{
			m_bInThermOpticCamo = !m_bInThermOpticCamo;
		}
	}

	if (m_bInThermOpticCamo != m_bLastTickInThermOpticCamo)
	{
		PlayCloakSound();
	}
}

void C_NEO_Player::CheckVisionButtons()
{
	if (!m_bIsAllowedToToggleVision)
	{
		return;
	}

	if (m_afButtonPressed & IN_VISION)
	{
		if (IsAlive())
		{
			m_bIsAllowedToToggleVision = false;

			m_bInVision = !m_bInVision;

			if (m_bInVision)
			{
				DevMsg("Playing sound at :%f\n", gpGlobals->curtime);

				C_RecipientFilter filter;
				filter.AddRecipient(this);
				filter.MakeReliable();

				EmitSound_t params;
				params.m_bEmitCloseCaption = false;
				params.m_pOrigin = &GetAbsOrigin();
				params.m_nChannel = CHAN_ITEM;
				params.m_nFlags |= SND_DO_NOT_OVERWRITE_EXISTING_ON_CHANNEL;
				static int visionToggle = CBaseEntity::PrecacheScriptSound("NeoPlayer.VisionOn");
				params.m_hSoundScriptHandle = visionToggle;

				EmitSound(filter, entindex(), params);
			}
		}
	}
}

void C_NEO_Player::ZeroFriendlyPlayerLocArray()
{
	Assert(m_rvFriendlyPlayerPositions.Count() == MAX_PLAYERS);
	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		m_rvFriendlyPlayerPositions.GetForModify(i) = vec3_origin;
	}
}

float C_NEO_Player::GetAttackersScores(const int attackerIdx) const
{
	return m_rfAttackersScores.Get(attackerIdx);
}

int C_NEO_Player::DrawModel( int flags )
{
	// Do cloak if cloaked
	if (IsCloaked())
	{
		IMaterial *pass = materials->FindMaterial("dev/toc_cloakpass", TEXTURE_GROUP_CLIENT_EFFECTS);
		Assert(pass && !pass->IsErrorMaterial());

		if (pass && !pass->IsErrorMaterial())
		{
			//const int extraFlags = STUDIO_RENDER | STUDIO_TRANSPARENCY | STUDIO_NOSHADOWS | STUDIO_DRAWTRANSLUCENTSUBMODELS;
			modelrender->ForcedMaterialOverride(pass);
			const int ret = BaseClass::DrawModel(flags /*| extraFlags*/);
			modelrender->ForcedMaterialOverride(NULL);

			return ret;
		}
	}

	int ret = BaseClass::DrawModel(flags);

	auto pLocalPlayer = C_NEO_Player::GetLocalNEOPlayer();
	if (pLocalPlayer && pLocalPlayer->IsInVision())
	{
#define SPEED_BETWEEN_WALK_AND_RUN ((NEO_ASSAULT_WALK_SPEED + NEO_ASSAULT_NORM_SPEED) / 2.0)
		if ((pLocalPlayer->GetClass() == NEO_CLASS_ASSAULT) &&
			(GetAbsVelocity().Length() >= SPEED_BETWEEN_WALK_AND_RUN))
		{
			IMaterial* pass = materials->FindMaterial("dev/motion_third", TEXTURE_GROUP_MODEL);
			Assert(pass && !pass->IsErrorMaterial());

			if (pass && !pass->IsErrorMaterial())
			{
				// Render
				modelrender->ForcedMaterialOverride(pass);
				ret = BaseClass::DrawModel(flags | STUDIO_RENDER | STUDIO_TRANSPARENCY);
				modelrender->ForcedMaterialOverride(NULL);
#if(0)
				// Send to mv buffer
				static int bufferIdx = 0;
				const int numBuffers = 2;
				ITexture* pVM_Buffer = GetMVBuffer(bufferIdx);
				bufferIdx = (bufferIdx + 1) % numBuffers;
				Assert(pVM_Buffer && !pVM_Buffer->IsError());

				ITexture* pSrc = materials->FindTexture("_rt_FullFrameFB", TEXTURE_GROUP_RENDER_TARGET);
				Assert(pSrc && !pSrc->IsError());

				const int nSrcWidth = pSrc->GetActualWidth();
				const int nSrcHeight = pSrc->GetActualHeight();
				Rect_t DestRect{ 0, 0, nSrcWidth, nSrcHeight };

				CMatRenderContextPtr pRenderContext(materials);
				pRenderContext->CopyRenderTargetToTextureEx(pVM_Buffer, 0, &DestRect, NULL);

				// Render without effect
				//ret = BaseClass::DrawModel(flags);
				rendered = false;
#endif
			}
		}
		else if (pLocalPlayer->GetClass() == NEO_CLASS_SUPPORT)
		{
			IMaterial* pass = materials->FindMaterial("dev/thermal_third", TEXTURE_GROUP_MODEL);
			Assert(pass && !pass->IsErrorMaterial());

			if (pass && !pass->IsErrorMaterial())
			{
				// Render
				modelrender->ForcedMaterialOverride(pass);
				ret = BaseClass::DrawModel(flags | STUDIO_RENDER | STUDIO_TRANSPARENCY);
				modelrender->ForcedMaterialOverride(NULL);
			}
		}
	}

	return ret;
}

void C_NEO_Player::AddEntity( void )
{
	BaseClass::AddEntity();
}

ShadowType_t C_NEO_Player::ShadowCastType( void ) 
{
	return BaseClass::ShadowCastType();
}

C_BaseAnimating *C_NEO_Player::BecomeRagdollOnClient()
{
	return BaseClass::BecomeRagdollOnClient();
}

const QAngle& C_NEO_Player::GetRenderAngles()
{
	return BaseClass::GetRenderAngles();
}

bool C_NEO_Player::ShouldDraw( void )
{
	return BaseClass::ShouldDraw();
}

void C_NEO_Player::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged(type);
}

float C_NEO_Player::GetFOV( void )
{
	return BaseClass::GetFOV();
}

CStudioHdr *C_NEO_Player::OnNewModel( void )
{
	return BaseClass::OnNewModel();
}

void C_NEO_Player::TraceAttack( const CTakeDamageInfo &info,
	const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator )
{
	BaseClass::TraceAttack(info, vecDir, ptr, pAccumulator);
}

void C_NEO_Player::ItemPreFrame( void )
{
	BaseClass::ItemPreFrame();

	if (m_afButtonPressed & IN_DROP)
	{
		auto neoWep = dynamic_cast<CNEOBaseCombatWeapon*>(GetActiveWeapon());
		if (neoWep)
		{
			Weapon_Drop(neoWep);
		}
	}
}

void C_NEO_Player::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();
}

float C_NEO_Player::GetMinFOV() const
{
	return BaseClass::GetMinFOV();
}

Vector C_NEO_Player::GetAutoaimVector( float flDelta )
{
	return BaseClass::GetAutoaimVector(flDelta);
}

void C_NEO_Player::NotifyShouldTransmit( ShouldTransmitState_t state )
{
	BaseClass::NotifyShouldTransmit(state);
}

void C_NEO_Player::CreateLightEffects(void)
{
	BaseClass::CreateLightEffects();
}

bool C_NEO_Player::ShouldReceiveProjectedTextures( int flags )
{
	return BaseClass::ShouldReceiveProjectedTextures(flags);
}

void C_NEO_Player::PostDataUpdate( DataUpdateType_t updateType )
{
	BaseClass::PostDataUpdate(updateType);

	CNEOPredictedViewModel *vm = (CNEOPredictedViewModel*)GetViewModel();
	if (vm)
	{
		SetNextThink(CLIENT_THINK_ALWAYS);
	}
}

void C_NEO_Player::PlayStepSound( Vector &vecOrigin,
	surfacedata_t *psurface, float fvol, bool force )
{
	BaseClass::PlayStepSound(vecOrigin, psurface, fvol, force);
}

extern ConVar sv_infinite_aux_power;

void C_NEO_Player::PreThink( void )
{
	BaseClass::PreThink();

	if ((!GetActiveWeapon() && IsAlive()) ||
		// Whether or not we move backwards affects max speed
		((m_afButtonPressed | m_afButtonReleased) & IN_BACK))
	{
		if (GetFlags() & FL_DUCKING)
		{
			SetMaxSpeed(GetCrouchSpeed());
		}
		else if (IsWalking())
		{
			SetMaxSpeed(GetWalkSpeed());
		}
		else if (IsSprinting())
		{
			SetMaxSpeed(GetSprintSpeed());
		}
		else
		{
			SetMaxSpeed(GetNormSpeed());
		}
	}

	CheckThermOpticButtons();
	CheckVisionButtons();

	if (m_bInThermOpticCamo)
	{
		if (m_flCamoAuxLastTime == 0)
		{
			// NEO TODO (Rain): add server style interface for accessor,
			// so we can share code
			if (m_HL2Local.m_flSuitPower >= CLOAK_AUX_COST)
			{
				m_flCamoAuxLastTime = gpGlobals->curtime;
			}
		}
		else
		{
			const float deltaTime = gpGlobals->curtime + gpGlobals->interpolation_amount - m_flCamoAuxLastTime;
			if (deltaTime >= 1)
			{
				// NEO TODO (Rain): add interface for predicting this
				//SuitPower_Drain(deltaTime * CLOAK_AUX_COST);

				const float auxToDrain = deltaTime * CLOAK_AUX_COST;
				if (m_HL2Local.m_flSuitPower <= auxToDrain)
				{
					m_HL2Local.m_flSuitPower = 0;
				}

				if (m_HL2Local.m_flSuitPower < CLOAK_AUX_COST)
				{
					m_bInThermOpticCamo = false;

					m_HL2Local.m_flSuitPower = 0;
					m_flCamoAuxLastTime = 0;
				}
				else
				{
					m_flCamoAuxLastTime = gpGlobals->curtime;
				}
			}
		}
	}
	else
	{
		m_flCamoAuxLastTime = 0;
	}

	Lean();

	// Eek. See rationale for this thing in CNEO_Player::PreThink
	if (IsAirborne())
	{
		m_flLastAirborneJumpOkTime = gpGlobals->curtime;
		const float deltaTime = gpGlobals->curtime - m_flLastAirborneJumpOkTime;
		const float leeway = 0.5f;
		if (deltaTime > leeway)
		{
			m_bHasBeenAirborneForTooLongToSuperJump = false;
			m_flLastAirborneJumpOkTime = gpGlobals->curtime;
		}
		else
		{
			m_bHasBeenAirborneForTooLongToSuperJump = true;
		}
	}
	else
	{
		m_bHasBeenAirborneForTooLongToSuperJump = false;
	}

	if (m_iNeoClass == NEO_CLASS_RECON)
	{
		if ((m_afButtonPressed & IN_JUMP) && (m_nButtons & IN_SPEED))
		{
			// If player holds both forward + back, only use up AUX power.
			// This movement trick replaces the original NT's trick of
			// sideways-superjumping with the intent of dumping AUX for a
			// jump setup that requires sprint jumping without the superjump.
			if (IsAllowedToSuperJump())
			{
				if (!((m_nButtons & IN_FORWARD) && (m_nButtons & IN_BACK)))
				{
					SuperJump();
				}
			}
		}
	}

	if (m_bShowTeamMenu && !m_bIsTeamMenuOpen)
	{
		m_bIsTeamMenuOpen = true;
		engine->ClientCmd(teammenu.GetName());
	}
	else if (m_bShowClassMenu && !m_bIsClassMenuOpen)
	{
		m_bIsClassMenuOpen = true;
		engine->ClientCmd(classmenu.GetName());
	}
	else if (m_bShowTeamMenu && m_bShowClassMenu)
	{
		m_bShowClassMenu = false;
		m_bIsTeamMenuOpen = true;
		m_bIsClassMenuOpen = false;
		engine->ClientCmd(teammenu.GetName());
	}

	// NEO TODO (Rain): marker should be responsible for its own vis control instead
	CNEOHud_GhostMarker *ghostMarker = NULL;
	if (m_pNeoPanel)
	{
		m_pNeoPanel->SetLastUpdater(this);

		ghostMarker = m_pNeoPanel->GetGhostMarker();

		if (ghostMarker)
		{
			if (!m_bGhostExists)
			{
				ghostMarker->SetVisible(false);

				//m_pGhostMarker->SetVisible(false);
			}
			else
			{
				const float distance = METERS_PER_INCH *
					GetAbsOrigin().DistTo(m_vecGhostMarkerPos);

				// NEO HACK (Rain): We should test if we're holding a ghost
				// instead of relying on a distance check.
				if (m_iGhosterTeam != GetTeamNumber() || distance > 0.2)
				{
					ghostMarker->SetVisible(true);

					int ghostMarkerX, ghostMarkerY;
					GetVectorInScreenSpace(m_vecGhostMarkerPos, ghostMarkerX, ghostMarkerY);

					ghostMarker->SetScreenPosition(ghostMarkerX, ghostMarkerY);
					ghostMarker->SetGhostingTeam(m_iGhosterTeam);


					ghostMarker->SetGhostDistance(distance);
				}
				else
				{
					ghostMarker->SetVisible(false);
				}
			}
		}
		else
		{
			Warning("Couldn't find ghostMarker\n");
		}

		auto indicator = m_pNeoPanel->GetGameEventIndicator();

		if (indicator)
		{
			if (m_bShowTestMessage)
			{
				indicator->SetMessage(m_pszTestMessage, sizeof(m_pszTestMessage));
			}

			if (indicator->IsVisible() != m_bShowTestMessage)
			{
				indicator->SetVisible(m_bShowTestMessage);
			}
		}
		else
		{
			Warning("Couldn't find GameEventIndicator\n");
		}
	}
}

void C_NEO_Player::Lean(void)
{
	auto vm = static_cast<CNEOPredictedViewModel*>(GetViewModel());
	if (vm)
	{
		Assert(GetBaseAnimating());
		GetBaseAnimating()->SetBoneController(0, vm->lean(this));

		//debugoverlay->AddTextOverlay(this->GetAbsOrigin() + GetViewOffset(), 0.001, "client view offset");
	}
}

void C_NEO_Player::ClientThink(void)
{
	BaseClass::ClientThink();
}

static ConVar neo_this_client_speed("neo_this_client_speed", "0", FCVAR_SPONLY);

void C_NEO_Player::PostThink(void)
{
	BaseClass::PostThink();

	if (GetLocalNEOPlayer() == this)
	{
		neo_this_client_speed.SetValue(MIN(GetAbsVelocity().Length2D() / GetNormSpeed(), 1.0f));
	}

	//DevMsg("Roll: %f\n", m_angEyeAngles[2]);

	bool preparingToHideMsg = (m_iCapTeam != TEAM_UNASSIGNED);

	if (!preparingToHideMsg && m_bPreviouslyPreparingToHideMsg)
	{
		if (m_pNeoPanel && m_pNeoPanel->GetGameEventIndicator())
		{
			m_pNeoPanel->GetGameEventIndicator()->SetVisible(false);
			m_bPreviouslyPreparingToHideMsg = false;
		}
		else
		{
			Assert(false);
		}
	}
	else
	{
		m_bPreviouslyPreparingToHideMsg = preparingToHideMsg;
	}

	if (!IsAlive())
	{
		// Undo aim zoom if just died
		if (m_bFirstDeathTick)
		{
			m_bFirstDeathTick = false;

			Weapon_SetZoom(false);
			m_bInVision = false;
		}

		return;
	}
	else
	{
		if (!m_bFirstDeathTick)
		{
			m_bFirstDeathTick = true;
		}
	}

	C_BaseCombatWeapon *pWep = GetActiveWeapon();

	if (pWep)
	{
		if (pWep->m_bInReload && !m_bPreviouslyReloading)
		{
			Weapon_SetZoom(false);
		}
		else if (m_afButtonPressed & IN_SPEED)
		{
			Weapon_SetZoom(false);
		}
		else if ((m_afButtonReleased & IN_AIM) && (!(m_nButtons & IN_SPEED)))
		{
			Weapon_AimToggle(pWep);
		}

#if !defined( NO_ENTITY_PREDICTION )
		// Can't do aim zoom in prediction, because we can't know
		// server's reload state for our weapon with certainty.
		if (!GetPredictable() || !prediction->InPrediction())
		{
#else
		if (true) {
#endif
			m_bPreviouslyReloading = pWep->m_bInReload;
		}
	}

	Vector eyeForward;
	this->EyeVectors(&eyeForward, NULL, NULL);
	Assert(eyeForward.IsValid());
	m_pPlayerAnimState->Update(eyeForward[YAW], eyeForward[PITCH]);
}

bool C_NEO_Player::IsAllowedToSuperJump(void)
{
	if (IsCarryingGhost())
		return false;

	if (GetMoveParent())
		return false;

	// Can't superjump whilst airborne (although it is kind of cool)
	if (m_bHasBeenAirborneForTooLongToSuperJump)
		return false;

	// Only superjump if we have a reasonable jump direction in mind
	// NEO TODO (Rain): should we support sideways superjumping?
	if ((m_nButtons & (IN_FORWARD | IN_BACK)) == 0)
	{
		return false;
	}

	// The suit check is for prediction only, actual power drain happens serverside
	if (m_HL2Local.m_flSuitPower < SUPER_JMP_COST)
		return false;

	if (SUPER_JMP_DELAY_BETWEEN_JUMPS > 0)
	{
		m_flLastSuperJumpTime = gpGlobals->curtime;
		const float deltaTime = gpGlobals->curtime - m_flLastSuperJumpTime;
		if (deltaTime > SUPER_JMP_DELAY_BETWEEN_JUMPS)
			return false;

		m_flLastSuperJumpTime = gpGlobals->curtime;
	}

	return true;
}

// This is applied for prediction purposes. It should match CNEO_Player's method.
void C_NEO_Player::SuperJump(void)
{
	Vector forward;
	AngleVectors(EyeAngles(), &forward);

	// We don't give an upwards boost aside from regular jump
	forward.z = 0;

	ApplyAbsVelocityImpulse(forward * neo_recon_superjump_intensity.GetFloat());
}

void C_NEO_Player::Spawn( void )
{
	BaseClass::Spawn();

	m_bLastTickInThermOpticCamo = m_bInThermOpticCamo = false;
	m_flCamoAuxLastTime = 0;

	m_bInVision = false;
	m_nVisionLastTick = 0;

	for (int i = 0; i < m_rfAttackersScores.Count(); ++i)
	{
		m_rfAttackersScores.Set(i, 0.0f);
	}

	Weapon_SetZoom(false);

	SetViewOffset(VEC_VIEW_NEOSCALE(this));

	if (GetTeamNumber() == TEAM_UNASSIGNED)
	{
		m_bShowTeamMenu = true;
	}

	// NEO TODO (Rain): UI elements should do this themselves
	if (!m_pNeoPanel)
	{
		m_pNeoPanel = dynamic_cast<CNeoHudElements*>
			(GetClientModeNormal()->GetViewport()->FindChildByName(PANEL_NEO_HUD));

		if (!m_pNeoPanel)
		{
			Assert(false);
			Warning("Couldn't find CNeoHudElements panel\n");
		}
		else
		{
			m_pNeoPanel->ShowPanel(true);
		}
	}

#if(0)
	// We could support crosshair customization/colors etc this way.
	auto cross = GET_HUDELEMENT(CHudCrosshair);
	Color color = Color(255, 255, 255, 255);
	cross->SetCrosshair(NULL, color);
#endif
}

void C_NEO_Player::DoImpactEffect( trace_t &tr, int nDamageType )
{
	BaseClass::DoImpactEffect(tr, nDamageType);
}

IRagdoll* C_NEO_Player::GetRepresentativeRagdoll() const
{
	return BaseClass::GetRepresentativeRagdoll();
}

void C_NEO_Player::CalcView( Vector &eyeOrigin, QAngle &eyeAngles,
	float &zNear, float &zFar, float &fov )
{
	BaseClass::CalcView(eyeOrigin, eyeAngles, zNear, zFar, fov);
}

const QAngle &C_NEO_Player::EyeAngles()
{
	return BaseClass::EyeAngles();
}

// Whether to draw the HL2 style quick health/ammo info around the crosshair.
// Clients can control their preference with a ConVar that gets polled here.
bool C_NEO_Player::ShouldDrawHL2StyleQuickHud(void)
{
	return cl_drawhud_quickinfo.GetBool();
}

void C_NEO_Player::Weapon_Drop(C_NEOBaseCombatWeapon *pWeapon)
{
	Weapon_SetZoom(false);

	if (pWeapon->IsGhost())
	{
		Assert(dynamic_cast<C_WeaponGhost*>(pWeapon));
		static_cast<C_WeaponGhost*>(pWeapon)->HandleGhostUnequip();
	}
	else if (pWeapon->GetNeoWepBits() & NEO_WEP_SUPA7)
	{
		Assert(dynamic_cast<C_WeaponSupa7*>(pWeapon));
		static_cast<C_WeaponSupa7*>(pWeapon)->ClearDelayedInputs();
	}
}

void C_NEO_Player::StartSprinting(void)
{
	if (m_HL2Local.m_flSuitPower < 10)
	{
		return;
	}

	if (IsCarryingGhost())
	{
		return;
	}

	SetMaxSpeed(GetSprintSpeed());
}

void C_NEO_Player::StopSprinting(void)
{
	SetMaxSpeed(GetNormSpeed());

	m_fIsSprinting = false;
}

bool C_NEO_Player::CanSprint(void)
{
	if (m_iNeoClass == NEO_CLASS_SUPPORT)
	{
		return false;
	}

	return BaseClass::CanSprint();
}

void C_NEO_Player::StartWalking(void)
{
	SetMaxSpeed(GetWalkSpeed());

	m_fIsWalking = true;
}

void C_NEO_Player::StopWalking(void)
{
	SetMaxSpeed(GetNormSpeed());

	m_fIsWalking = true;
}

float C_NEO_Player::GetCrouchSpeed_WithActiveWepEncumberment(void) const
{
	return GetCrouchSpeed() * GetActiveWeaponSpeedScale();
}

float C_NEO_Player::GetNormSpeed_WithActiveWepEncumberment(void) const
{
	return GetNormSpeed() * GetActiveWeaponSpeedScale();
}

float C_NEO_Player::GetWalkSpeed_WithActiveWepEncumberment(void) const
{
	return GetWalkSpeed() * GetActiveWeaponSpeedScale();
}

float C_NEO_Player::GetSprintSpeed_WithActiveWepEncumberment(void) const
{
	return GetSprintSpeed() * GetActiveWeaponSpeedScale();
}

float C_NEO_Player::GetCrouchSpeed_WithWepEncumberment(CNEOBaseCombatWeapon* pNeoWep) const
{
	Assert(pNeoWep);
	return GetCrouchSpeed() * pNeoWep->GetSpeedScale();
}

float C_NEO_Player::GetNormSpeed_WithWepEncumberment(CNEOBaseCombatWeapon* pNeoWep) const
{
	Assert(pNeoWep);
	return GetNormSpeed() * pNeoWep->GetSpeedScale();
}

float C_NEO_Player::GetWalkSpeed_WithWepEncumberment(CNEOBaseCombatWeapon* pNeoWep) const
{
	Assert(pNeoWep);
	return GetWalkSpeed() * pNeoWep->GetSpeedScale();
}

float C_NEO_Player::GetSprintSpeed_WithWepEncumberment(CNEOBaseCombatWeapon* pNeoWep) const
{
	Assert(pNeoWep);
	return GetSprintSpeed() * pNeoWep->GetSpeedScale();
}

float C_NEO_Player::GetCrouchSpeed(void) const
{
	switch (m_iNeoClass)
	{
	case NEO_CLASS_RECON:
		return NEO_RECON_CROUCH_SPEED * GetBackwardsMovementPenaltyScale();
	case NEO_CLASS_ASSAULT:
		return NEO_ASSAULT_CROUCH_SPEED * GetBackwardsMovementPenaltyScale();
	case NEO_CLASS_SUPPORT:
		return NEO_SUPPORT_CROUCH_SPEED * GetBackwardsMovementPenaltyScale();
	default:
		return NEO_BASE_CROUCH_SPEED * GetBackwardsMovementPenaltyScale();
	}
}

float C_NEO_Player::GetNormSpeed(void) const
{
	switch (m_iNeoClass)
	{
	case NEO_CLASS_RECON:
		return NEO_RECON_NORM_SPEED * GetBackwardsMovementPenaltyScale();
	case NEO_CLASS_ASSAULT:
		return NEO_ASSAULT_NORM_SPEED * GetBackwardsMovementPenaltyScale();
	case NEO_CLASS_SUPPORT:
		return NEO_SUPPORT_NORM_SPEED * GetBackwardsMovementPenaltyScale();
	default:
		return NEO_BASE_NORM_SPEED * GetBackwardsMovementPenaltyScale();
	}
}

float C_NEO_Player::GetWalkSpeed(void) const
{
	switch (m_iNeoClass)
	{
	case NEO_CLASS_RECON:
		return NEO_RECON_WALK_SPEED * GetBackwardsMovementPenaltyScale();
	case NEO_CLASS_ASSAULT:
		return NEO_ASSAULT_WALK_SPEED * GetBackwardsMovementPenaltyScale();
	case NEO_CLASS_SUPPORT:
		return NEO_SUPPORT_WALK_SPEED * GetBackwardsMovementPenaltyScale();
	default:
		return NEO_BASE_WALK_SPEED * GetBackwardsMovementPenaltyScale();
	}
}

float C_NEO_Player::GetSprintSpeed(void) const
{
	switch (m_iNeoClass)
	{
	case NEO_CLASS_RECON:
		return NEO_RECON_SPRINT_SPEED * GetBackwardsMovementPenaltyScale();
	case NEO_CLASS_ASSAULT:
		return NEO_ASSAULT_SPRINT_SPEED * GetBackwardsMovementPenaltyScale();
	case NEO_CLASS_SUPPORT:
		return NEO_SUPPORT_SPRINT_SPEED * GetBackwardsMovementPenaltyScale();
	default:
		return NEO_BASE_SPRINT_SPEED * GetBackwardsMovementPenaltyScale();
	}
}

float C_NEO_Player::GetActiveWeaponSpeedScale() const
{
	// NEO TODO (Rain): change to static cast once all weapons are guaranteed to derive from the class
	auto pWep = dynamic_cast<CNEOBaseCombatWeapon*>(GetActiveWeapon());
	return (pWep ? pWep->GetSpeedScale() : 1.0f);
}

void C_NEO_Player::Weapon_AimToggle(C_BaseCombatWeapon *pWep)
{
	// NEO TODO/HACK: Not all neo weapons currently inherit
	// through a base neo class, so we can't static_cast!!
	auto neoCombatWep = dynamic_cast<C_NEOBaseCombatWeapon*>(pWep);
	if (!neoCombatWep)
	{
		return;
	}

	// This implies the wep ptr is valid, so we don't bother checking
	if (IsAllowedToZoom(neoCombatWep))
	{
		if (neoCombatWep->IsReadyToAimIn())
		{
			const bool showCrosshair = (m_Local.m_iHideHUD & HIDEHUD_CROSSHAIR) == HIDEHUD_CROSSHAIR;
			Weapon_SetZoom(showCrosshair);
		}
		else
		{
			Weapon_SetZoom(false);
		}
	}
}

void C_NEO_Player::Weapon_SetZoom(const bool bZoomIn)
{
	float zoomSpeedSecs = 0.25f;

#if(0)
#if !defined( NO_ENTITY_PREDICTION )
	if (!prediction->InPrediction() && IsLocalPlayer())
	{
		if (GetPredictable())
		{
			zoomSpeedSecs *= gpGlobals->interpolation_amount;
		}
	}
#endif
#endif

	if (bZoomIn)
	{
		m_Local.m_iHideHUD &= ~HIDEHUD_CROSSHAIR;

		const int zoomAmount = 30;
		SetFOV((CBaseEntity*)this, GetDefaultFOV() - zoomAmount, zoomSpeedSecs);
	}
	else
	{
		m_Local.m_iHideHUD |= HIDEHUD_CROSSHAIR;

		SetFOV((CBaseEntity*)this, GetDefaultFOV(), zoomSpeedSecs);
	}

	m_bInAim = bZoomIn;
}

bool C_NEO_Player::IsCarryingGhost(void) const
{
	return GetNeoWepWithBits(this, NEO_WEP_GHOST) != NULL;
}

const Vector C_NEO_Player::GetPlayerMins(void) const
{
	return VEC_DUCK_HULL_MIN_SCALED(this);
}

const Vector C_NEO_Player::GetPlayerMaxs(void) const
{
	return VEC_DUCK_HULL_MAX_SCALED(this);
}

void C_NEO_Player::PlayCloakSound(void)
{
	C_RecipientFilter filter;
	filter.AddRecipient(this);
	filter.MakeReliable();

	static int tocOn = CBaseEntity::PrecacheScriptSound("NeoPlayer.ThermOpticOn");
	static int tocOff = CBaseEntity::PrecacheScriptSound("NeoPlayer.ThermOpticOff");

	EmitSound_t params;
	params.m_bEmitCloseCaption = false;
	params.m_hSoundScriptHandle = (m_bInThermOpticCamo ? tocOn : tocOff);
	params.m_pOrigin = &GetAbsOrigin();
	params.m_nChannel = CHAN_VOICE;

	EmitSound(filter, entindex(), params);
}

void C_NEO_Player::PreDataUpdate(DataUpdateType_t updateType)
{
	if (updateType == DATA_UPDATE_DATATABLE_CHANGED)
	{
		if (gpGlobals->tickcount - m_nVisionLastTick < TIME_TO_TICKS(0.1f))
		{
			return;
		}
		else
		{
			m_bIsAllowedToToggleVision = true;
		}
	}

	BaseClass::PreDataUpdate(updateType);
}

void C_NEO_Player::SetAnimation(PLAYER_ANIM playerAnim)
{
	PlayerAnimEvent_t animEvent;
	if (!PlayerAnimToPlayerAnimEvent(playerAnim, animEvent))
	{
		DevWarning("CLI Tried to get unknown PLAYER_ANIM %d\n", playerAnim);
	}
	else
	{
		m_pPlayerAnimState->DoAnimationEvent(animEvent);
	}
}

extern ConVar sv_neo_wep_dmg_modifier;

// NEO NOTE (Rain): doesn't seem to be implemented at all clientside?
// Don't need to do this, unless we want it for prediction with proper implementation later.
// Keeping it for now.
void C_NEO_Player::ModifyFireBulletsDamage(CTakeDamageInfo* dmgInfo)
{
	dmgInfo->SetDamage(dmgInfo->GetDamage() * sv_neo_wep_dmg_modifier.GetFloat());
}