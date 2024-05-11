#include "cbase.h"
#include "neo_scoreboard.h"

#include "hud.h"
#include "cdll_util.h"
#include "c_team.h"
#include "c_playerresource.h"
#include "c_neo_player.h"
#include "neo_gamerules.h"

#include <KeyValues.h>
#include "ienginevgui.h"
#include "voice_status.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/SectionedListPanel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

enum EScoreboardSections
{
	SCORESECTION_JINRAI = 1,
	SCORESECTION_NSF,
	SCORESECTION_SPECTATOR,
};

CNEOScoreBoard::CNEOScoreBoard(IViewPort *pViewPort)
	: CHL2MPClientScoreBoardDialog(pViewPort)
{
	vgui::HScheme neoscheme = vgui::scheme()->LoadSchemeFromFileEx(
		enginevgui->GetPanel(PANEL_CLIENTDLL), "resource/ClientScheme.res", "ClientScheme");
	SetScheme(neoscheme);
}

CNEOScoreBoard::~CNEOScoreBoard()
{
}

void CNEOScoreBoard::InitScoreboardSections()
{
	BaseClass::InitScoreboardSections();
}

// Purpose: Update the scoreboard rows with currently connected players' info
void CNEOScoreBoard::UpdatePlayerInfo()
{
	if (!g_PR)
	{
		return;
	}

	CBasePlayer *player = C_BasePlayer::GetLocalPlayer();
	Assert(player);

	const Color test = Color(255, 0, 0, 255);
	surface()->DrawSetTextColor(test);

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		int itemId = FindItemIDForPlayerIndex(i);

		if (g_PR->IsConnected(i))
		{
			KeyValues *playerData = new KeyValues("data");

			GetPlayerScoreInfo(i, playerData);
			int sectionId = GetSectionFromTeamNumber(g_PR->GetTeam(i));

			// We aren't in the scoreboard yet
			if (itemId == -1)
			{
				itemId = m_pPlayerList->AddItem(sectionId, playerData);
			}
			else
			{
				m_pPlayerList->ModifyItem(itemId, sectionId, playerData);
			}

			// Highlight the row if this is the local player
			if (player && i == player->entindex())
			{
				Assert(itemId != -1);
				m_pPlayerList->SetSelectedItem(itemId);
				m_pPlayerList->SetFgColor(test);
			}

			playerData->deleteThis();
		}
		// We have itemId for unconnected player, remove it
		else if (itemId != -1)
		{
			m_pPlayerList->RemoveItem(itemId);
		}
	}

	m_pPlayerList->SetVisible(true);
}

void CNEOScoreBoard::UpdateTeamInfo()
{
	BaseClass::UpdateTeamInfo();
}

bool CNEOScoreBoard::GetPlayerScoreInfo(int playerIndex, KeyValues *outPlayerInfo)
{
	return BaseClass::GetPlayerScoreInfo(playerIndex, outPlayerInfo);
}

void CNEOScoreBoard::PaintBackground()
{
	BaseClass::PaintBackground();
}

void CNEOScoreBoard::PaintBorder()
{
	BaseClass::PaintBorder();
}

void CNEOScoreBoard::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

#define ADD_COL(sectionId, columnName, columnText, columnFlags, width, fallbackFont) \
	m_pPlayerList->AddColumnToSection(sectionId, columnName, columnText, columnFlags, \
	scheme()->GetProportionalScaledValueEx(GetScheme(), width), fallbackFont)
#define COL_RIGHT SectionedListPanel::COLUMN_RIGHT
#define COL_IMAGE SectionedListPanel::COLUMN_IMAGE

void CNEOScoreBoard::AddHeader()
{
	HFont hFallbackFont = scheme()->GetIScheme(GetScheme())->
		GetFont("DefaultVerySmallFallBack", false);

	m_pPlayerList->AddSection(0, "");
	m_pPlayerList->SetSectionAlwaysVisible(0);
	ADD_COL(0, "name", "", 0, NEO_NAME_WIDTH, hFallbackFont);
	ADD_COL(0, "class", "", 0, NEO_CLASS_WIDTH, hFallbackFont);
	ADD_COL(0, "rank", "Rank", COL_RIGHT, NEO_NAME_WIDTH / 4, hFallbackFont);
	ADD_COL(0, "xp", "XP", COL_RIGHT, NEO_SCORE_WIDTH, hFallbackFont);
	ADD_COL(0, "deaths", "#PlayerDeath", COL_RIGHT, NEO_DEATH_WIDTH, hFallbackFont);
	ADD_COL(0, "ping", "#PlayerPing", COL_RIGHT, NEO_PING_WIDTH, hFallbackFont);
	ADD_COL(0, "status", "Status", COL_RIGHT, NEO_STATUS_WIDTH, hFallbackFont);
	//ADD_COL(0, "voice", "#PlayerVoice", COL_IMAGE, NEO_VOICE_WIDTH, hFallbackFont);
	//ADD_COL(0, "tracker", "#PlayerTracker", COL_IMAGE, NEO_FRIENDS_WIDTH, hFallbackFont);
}

void CNEOScoreBoard::AddSection(int teamType, int teamNumber)
{
	BaseClass::AddSection(teamType, teamNumber);
}

int CNEOScoreBoard::GetSectionFromTeamNumber(int teamNumber)
{
	if (teamNumber == TEAM_JINRAI) { return SCORESECTION_JINRAI; }
	else if (teamNumber == TEAM_NSF) { return SCORESECTION_NSF; }
	else if (teamNumber == TEAM_SPECTATOR) { return SCORESECTION_SPECTATOR; }
	
	// We shouldn't ever fall through
	Assert(false);

	return SCORESECTION_SPECTATOR;
}

void CNEOScoreBoard::SetVisible(bool state)
{
	// Temp fix to show team scores etc when opening scoreboard.
	if (C_NEO_Player::GetLocalNEOPlayer()->IsAlive())
	{
		gViewPortInterface->ShowPanel(gViewPortInterface->FindPanelByName(PANEL_SPECGUI), state);
	}

	BaseClass::SetVisible(state);
}