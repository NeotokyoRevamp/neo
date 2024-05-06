#include "cbase.h"
#include "neo_hud_friendly_marker.h"

#include "iclientmode.h"
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui/IScheme.h>

#include <engine/ivdebugoverlay.h>
#include "ienginevgui.h"

#include "neo_gamerules.h"

#include "c_neo_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using vgui::surface;

ConVar neo_friendly_marker_hud_scale_factor("neo_friendly_marker_hud_scale_factor", "0.5", FCVAR_USERINFO,
	"Friendly player marker HUD element scaling factor", true, 0.01, false, 0);

NEO_HUD_ELEMENT_DECLARE_FREQ_CVAR(FriendlyMarker, 0.01)

CNEOHud_FriendlyMarker::CNEOHud_FriendlyMarker(const char* pElemName, vgui::Panel* parent)
	: CHudElement(pElemName), Panel(parent, pElemName)
{
	m_iPosX = m_iPosY = 0;

	SetAutoDelete(true);

	vgui::HScheme neoscheme = vgui::scheme()->LoadSchemeFromFileEx(
		enginevgui->GetPanel(PANEL_CLIENTDLL), "resource/ClientScheme_Neo.res", "ClientScheme_Neo");
	SetScheme(neoscheme);

	if (parent)
	{
		SetParent(parent);
	}
	else
	{
		SetParent(g_pClientMode->GetViewport());
	}

	int wide, tall;
	surface()->GetScreenSize(wide, tall);
	SetBounds(0, 0, wide, tall);

	// NEO HACK (Rain): this is kind of awkward, we should get the handle on ApplySchemeSettings
	vgui::IScheme *scheme = vgui::scheme()->GetIScheme(neoscheme);
	Assert(scheme);

	m_hFont = scheme->GetFont("NHudOCRSmall", true);

	m_hTex = surface()->CreateNewTextureID();
	Assert(m_hTex > 0);
	surface()->DrawSetTextureFile(m_hTex, "vgui/hud/star", 1, false);

	surface()->DrawGetTextureSize(m_hTex, m_iMarkerTexWidth, m_iMarkerTexHeight);

	SetFgColor(Color(0, 0, 0, 0));
	SetBgColor(Color(0, 0, 0, 0));

	SetVisible(true);
}

void CNEOHud_FriendlyMarker::UpdateStateForNeoHudElementDraw()
{
	int x, y;
	const float scale = neo_friendly_marker_hud_scale_factor.GetFloat();
	const float heightOffset = 48.0f;
	Vector pos;

	auto localPlayer = C_NEO_Player::GetLocalNEOPlayer();

	Assert(localPlayer->m_rvFriendlyPlayerPositions.Count() == MAX_PLAYERS);

	COMPILE_TIME_ASSERT(ARRAYSIZE(m_x0) == MAX_PLAYERS);
	COMPILE_TIME_ASSERT(ARRAYSIZE(m_x1) == MAX_PLAYERS);
	COMPILE_TIME_ASSERT(ARRAYSIZE(m_y0) == MAX_PLAYERS);
	COMPILE_TIME_ASSERT(ARRAYSIZE(m_y1) == MAX_PLAYERS);

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		if ((localPlayer->entindex() == i + 1) || // Skip self
			(localPlayer->m_rvFriendlyPlayerPositions[i] == vec3_origin)) // Skip unused positions
		{
			m_x0[i] = m_x1[i] = m_y0[i] = m_y1[i] = 0;
			continue;
		}

		pos = Vector(
			localPlayer->m_rvFriendlyPlayerPositions[i].x,
			localPlayer->m_rvFriendlyPlayerPositions[i].y,
			localPlayer->m_rvFriendlyPlayerPositions[i].z + heightOffset);

		if (GetVectorInScreenSpace(pos, x, y))
		{
			m_x0[i] = RoundFloatToInt((x - ((m_iMarkerTexWidth * 0.5f) * scale)));
			m_x1[i] = RoundFloatToInt((x + ((m_iMarkerTexWidth * 0.5f) * scale)));
			m_y0[i] = RoundFloatToInt((y - ((m_iMarkerTexHeight * 0.5f) * scale)));
			m_y1[i] = RoundFloatToInt((y + ((m_iMarkerTexHeight * 0.5f) * scale)));
		}
	}
}

void CNEOHud_FriendlyMarker::DrawNeoHudElement()
{
	if (!ShouldDraw())
	{
		return;
	}

	const Color teamColor = (C_NEO_Player::GetLocalNEOPlayer()->GetTeamNumber() == TEAM_NSF) ? COLOR_NSF : COLOR_JINRAI;

	surface()->DrawSetTextFont(m_hFont);
	surface()->DrawSetTextColor(teamColor);

	surface()->DrawSetColor(teamColor);
	surface()->DrawSetTexture(m_hTex);

	auto localPlayer = C_NEO_Player::GetLocalNEOPlayer();

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		// Skip self
		if (localPlayer->entindex() == (i + 1))
		{
			continue;
		}
		// Skip unused
		if ((m_x0[i] == 0) && (m_x1[i] == 0) && (m_y0[i] == 0) && (m_y1[i] == 0))
		{
			continue;
		}

		surface()->DrawTexturedRect(m_x0[i], m_y0[i], m_x1[i], m_y1[i]);
#if(0)
		DevMsg("Drawing %i @ X:%i--%i AND Y:%i--%i. I am %i.\n",
			i, m_x0[i], m_x1[i], m_y0[i], m_y1[i], localPlayer->entindex());
#endif
	}
}

void CNEOHud_FriendlyMarker::Paint()
{
	BaseClass::Paint();
	PaintNeoElement();
}
