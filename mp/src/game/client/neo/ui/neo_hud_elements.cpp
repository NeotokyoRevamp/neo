#include "cbase.h"
#include "neo_hud_elements.h"

#include "GameEventListener.h"

#include "neo_hud_compass.h"
#include "neo_hud_friendly_marker.h"
#include "neo_hud_game_event.h"
#include "neo_hud_ghost_marker.h"
#include "neo_hud_round_state.h"

#include "vgui/ISurface.h"

#include "c_neo_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define UI_ELEMENT_NAME_COMPASS "neo_compass"
#define UI_ELEMENT_NAME_IFF "neo_iff"
#define UI_ELEMENT_GAME_EVENT "neo_game_event_indicator"
#define UI_ELEMENT_NAME_GHOST_MARKER "neo_ghost_marker"
#define UI_ELEMENT_NAME_GHOST_BEACON "neo_ghost_beacon"
#define UI_ELEMENT_ROUND_STATE "neo_round_state"

using namespace vgui;

CNeoHudElements::CNeoHudElements(IViewPort *pViewPort)
	: EditablePanel(NULL, PANEL_NEO_HUD)
{
	m_iPlayerIndexSymbol = KeyValuesSystem()->GetSymbolForString("playerIndex");

	m_pViewPort = pViewPort;

	SetProportional(true);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	// Set the scheme before any child control is created
	SetScheme("ClientScheme");

	// We need to update IFF if these happen
	ListenForGameEvent("player_death");
	ListenForGameEvent("player_team");

	m_pCompass = NULL;
	m_pFriendlyMarker = NULL;
	m_pGameEvent = NULL;
	m_pRoundState = NULL;
}

CNeoHudElements::~CNeoHudElements()
{
	FreePanelChildren();
}

void CNeoHudElements::OnThink()
{
	BaseClass::OnThink();
}

void CNeoHudElements::FreePanelChildren()
{
	if (m_pCompass)
	{
		m_pCompass->DeletePanel();
		m_pCompass = NULL;
	}

	if (m_pFriendlyMarker)
	{
		m_pFriendlyMarker->DeletePanel();
		m_pFriendlyMarker = NULL;
	}

	if (m_pGameEvent)
	{
		m_pGameEvent->DeletePanel();
		m_pGameEvent = NULL;
	}

	if (m_pRoundState)
	{
		m_pRoundState->DeletePanel();
		m_pRoundState = NULL;
	}

	for (CNEOHud_GhostMarker *marker : m_vecGhostMarkers)
	{
		if (marker)
		{
			marker->DeletePanel();
		}
		else
		{
			// We should be never carrying a null pointer
			Assert(false);
		}
	}
	m_vecGhostMarkers.RemoveAll();
}

void CNeoHudElements::Reset()
{
	m_fNextUpdateTime = 0;

	FreePanelChildren();

	InitHud();
}

void CNeoHudElements::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	int sizex, sizey;
	surface()->GetScreenSize(sizex, sizey);
	SetBounds(0, 0, sizex, sizey);

	PostApplySchemeSettings(pScheme);
}

void CNeoHudElements::PostApplySchemeSettings(vgui::IScheme* pScheme)
{
	SetBgColor(Color(255, 255, 0, 0));
}

void CNeoHudElements::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
	{
		return;
	}

	if (bShow)
	{
		Reset();
		Update();
		SetVisible(true);
		MoveToFront();
	}
	else
	{
		BaseClass::SetVisible(false);
	}
}

void CNeoHudElements::FireGameEvent(IGameEvent* event)
{
}

bool CNeoHudElements::NeedsUpdate()
{
	return (m_fNextUpdateTime < gpGlobals->curtime);
}

void CNeoHudElements::Update()
{
	FillIFFs();

	m_fNextUpdateTime = gpGlobals->curtime + 1.0f;
}

void CNeoHudElements::UpdatePlayerIFF(int playerIndex, KeyValues* kv)
{
	
}

void CNeoHudElements::FillIFFs()
{
	auto localPlayer = C_BasePlayer::GetLocalPlayer();

	if (!localPlayer)
	{
		return;
	}

	int localTeam = localPlayer->GetTeamNumber();

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		auto otherPlayer = UTIL_PlayerByIndex(i);

		if (!otherPlayer || otherPlayer == localPlayer)
		{
			continue;
		}

		if (otherPlayer->GetTeamNumber() != localTeam)
		{
			continue;
		}

		KeyValues *kv = NULL;
		UpdatePlayerIFF(i, kv);
	}
}

int CNeoHudElements::FindIFFItemIDForPlayerIndex(int playerIndex)
{
	return 0;
}

void CNeoHudElements::InitHud()
{
	InitCompass();
	InitFriendlyMarker();
	InitGameEventIndicator();
	InitGhostMarkers();
	InitRoundState();
}

void CNeoHudElements::InitCompass()
{
	m_pCompass = new CNEOHud_Compass(UI_ELEMENT_NAME_COMPASS, this);
}

void CNeoHudElements::InitFriendlyMarker()
{
	m_pFriendlyMarker = new CNEOHud_FriendlyMarker(UI_ELEMENT_NAME_IFF, this);
}

void CNeoHudElements::InitGameEventIndicator()
{
	m_pGameEvent = new CNEOHud_GameEvent(UI_ELEMENT_GAME_EVENT, this);
}

void CNeoHudElements::InitGhostMarkers()
{
	const int numGhosts = 1;

	for (int i = 0; i < numGhosts; i++)
	{
		auto marker = new CNEOHud_GhostMarker(UI_ELEMENT_NAME_GHOST_MARKER, this);

		m_vecGhostMarkers.AddToTail(marker);
	}
}

void CNeoHudElements::InitRoundState()
{
	m_pRoundState = new CNEOHud_RoundState(UI_ELEMENT_ROUND_STATE, this);
}


CNEOHud_Compass *CNeoHudElements::GetCompass()
{
	return m_pCompass;
}

CNEOHud_GameEvent* CNeoHudElements::GetGameEventIndicator()
{
	return m_pGameEvent;
}

CNEOHud_GhostMarker* CNeoHudElements::GetGhostMarker()
{
	// Just return first, for now
	for (auto marker : m_vecGhostMarkers)
	{
		Assert(marker);
		return marker;
	}

	return NULL;
}

CNEOHud_FriendlyMarker* CNeoHudElements::GetIFF()
{
	return m_pFriendlyMarker;
}