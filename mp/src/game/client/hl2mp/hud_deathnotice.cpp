//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Draws CSPort's death notices
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "c_playerresource.h"
#include "clientmode_hl2mpnormal.h"
#include <vgui_controls/Controls.h>
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <KeyValues.h>
#include "c_baseplayer.h"
#include "c_team.h"

#ifdef NEO
#include "../neo/ui/neo_hud_elements.h"
#include "../neo/ui/neo_hud_game_event.h"
#include "neo_player_shared.h"
#include "spectatorgui.h"
#include "takedamageinfo.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar hud_deathnotice_time( "hud_deathnotice_time", "6", 0 );

// Player entries in a death notice
struct DeathNoticePlayer
{
	char		szName[MAX_PLAYER_NAME_LENGTH];
	int			iEntIndex;
};

// Contents of each entry in our list of death notices
struct DeathNoticeItem 
{
	DeathNoticePlayer	Killer;
#ifdef NEO
	DeathNoticePlayer	Assists;
#endif
	DeathNoticePlayer   Victim;
	CHudTexture *iconDeath;
	int			iSuicide;
	float		flDisplayTime;
	bool		bHeadshot;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CHudDeathNotice : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudDeathNotice, vgui::Panel );
public:
	CHudDeathNotice( const char *pElementName );

	void Init( void );
	void VidInit( void );
	virtual bool ShouldDraw( void );
	virtual void Paint( void );
	virtual void ApplySchemeSettings( vgui::IScheme *scheme );

	void SetColorForNoticePlayer( int iTeamNumber );
	void RetireExpiredDeathNotices( void );
	
	virtual void FireGameEvent( IGameEvent * event );

private:

	CPanelAnimationVarAliasType( float, m_flLineHeight, "LineHeight", "15", "proportional_float" );

	CPanelAnimationVar( float, m_flMaxDeathNotices, "MaxDeathNotices", "4" );

	CPanelAnimationVar( bool, m_bRightJustify, "RightJustify", "1" );

	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "HudNumbersTimer" );

	// Texture for skull symbol
	CHudTexture		*m_iconD_skull;  
	CHudTexture		*m_iconD_headshot;  

	CUtlVector<DeathNoticeItem> m_DeathNotices;
};

using namespace vgui;

DECLARE_HUDELEMENT( CHudDeathNotice );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudDeathNotice::CHudDeathNotice( const char *pElementName ) :
	CHudElement( pElementName ), BaseClass( NULL, "HudDeathNotice" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_iconD_headshot = NULL;
	m_iconD_skull = NULL;

	SetHiddenBits( HIDEHUD_MISCSTATUS );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDeathNotice::ApplySchemeSettings( IScheme *scheme )
{
	BaseClass::ApplySchemeSettings( scheme );
	SetPaintBackgroundEnabled( false );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDeathNotice::Init( void )
{
	ListenForGameEvent( "player_death" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDeathNotice::VidInit( void )
{
	m_iconD_skull = gHUD.GetIcon( "d_skull" );
	m_DeathNotices.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: Draw if we've got at least one death notice in the queue
//-----------------------------------------------------------------------------
bool CHudDeathNotice::ShouldDraw( void )
{
	return ( CHudElement::ShouldDraw() && ( m_DeathNotices.Count() ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDeathNotice::SetColorForNoticePlayer( int iTeamNumber )
{
	surface()->DrawSetTextColor( GameResources()->GetTeamColor( iTeamNumber ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudDeathNotice::Paint()
{
	if (!m_iconD_skull)
	{
		Assert(false);
		return;
	}

#ifdef NEO
	// TODO (nullsystem): Just create this once rather than every paint
	wchar_t assistsPlusSign[64];
	g_pVGuiLocalize->ConvertANSIToUnicode(" + ", assistsPlusSign, sizeof(assistsPlusSign));
	const int assistsPlusSignWidth = UTIL_ComputeStringWidth(m_hTextFont, assistsPlusSign);

	if (g_hFontKillfeed != vgui::INVALID_FONT)
	{
		m_hTextFont = g_hFontKillfeed;
	}
	else
	{
		Assert(false);
	}
#endif

	int yStart = GetClientModeHL2MPNormal()->GetDeathMessageStartHeight();
#ifdef NEO
	if (g_pSpectatorGUI->IsVisible())
	{
		yStart += g_pSpectatorGUI->GetTopBarHeight();
	}
#endif

	surface()->DrawSetTextFont( m_hTextFont );
	surface()->DrawSetTextColor( GameResources()->GetTeamColor( 0 ) );

	int iCount = m_DeathNotices.Count();
	for ( int i = 0; i < iCount; i++ )
	{
#ifdef NEO
		int yNeoIconOffset = 0;
#endif

		CHudTexture *icon = m_DeathNotices[i].iconDeath;
		if ( !icon )
			continue;

		wchar_t victim[ 256 ];
		wchar_t killer[ 256 ];

		// Get the team numbers for the players involved
		int iKillerTeam = 0;
		int iVictimTeam = 0;

		if( g_PR )
		{
			iKillerTeam = g_PR->GetTeam( m_DeathNotices[i].Killer.iEntIndex );
			iVictimTeam = g_PR->GetTeam( m_DeathNotices[i].Victim.iEntIndex );
		}

		g_pVGuiLocalize->ConvertANSIToUnicode( m_DeathNotices[i].Victim.szName, victim, sizeof( victim ) );
		g_pVGuiLocalize->ConvertANSIToUnicode( m_DeathNotices[i].Killer.szName, killer, sizeof( killer ) );

#ifdef NEO
		wchar_t assists[256];
		int iAssistsTeam = 0;
		const bool hasAssists = m_DeathNotices[i].Assists.iEntIndex > 0;
		if (g_PR)
		{
			iAssistsTeam = g_PR->GetTeam(m_DeathNotices[i].Assists.iEntIndex);
		}
		g_pVGuiLocalize->ConvertANSIToUnicode(m_DeathNotices[i].Assists.szName, assists, sizeof(assists));
#endif

		// Get the local position for this notice
		int len = UTIL_ComputeStringWidth( m_hTextFont, victim );
		int y = yStart + (m_flLineHeight * i);

		int iconWide;
		int iconTall;

		if( icon->bRenderUsingFont )
		{
			iconWide = surface()->GetCharacterWidth( icon->hFont, icon->cCharacterInFont );
			iconTall = surface()->GetFontTall( icon->hFont );
		}
		else
		{
#ifdef NEO
			surface()->DrawGetTextureSize(icon->textureId, iconWide, iconTall);

			const float scale = ((float)ScreenHeight() / 480.0f);	//scale based on 640x480
			const float neoIconScale = 0.22;

			iconWide = (int)(scale * iconWide * neoIconScale);
			iconTall = (int)(scale * iconTall * neoIconScale);

			// Center the icon vertically in relation to the killfeed text.
			yNeoIconOffset = (int)(surface()->GetFontTall(m_hTextFont) * -0.25);
#else
			float scale = ((float)ScreenHeight() / 480.0f);	//scale based on 640x480
			iconWide = (int)(scale * (float)icon->Width());
			iconTall = (int)(scale * (float)icon->Height());
#endif
		}

		int x;
		if ( m_bRightJustify )
		{
			x =	GetWide() - len - iconWide;
		}
		else
		{
			x = 0;
		}
		
#ifdef NEO
		// Only draw killers name if it wasn't a suicide or has an assists in it
		if (!m_DeathNotices[i].iSuicide || hasAssists)
#else
		// Only draw killers name if it wasn't a suicide
		if ( !m_DeathNotices[i].iSuicide )
#endif
		{
			if ( m_bRightJustify )
			{
				x -= UTIL_ComputeStringWidth( m_hTextFont, killer );
#ifdef NEO
				if (hasAssists)
				{
					x -= (UTIL_ComputeStringWidth(m_hTextFont, assists) + assistsPlusSignWidth);
				}
#endif
			}

			SetColorForNoticePlayer( iKillerTeam );

			// Draw killer's name
			surface()->DrawSetTextPos( x, y );
			surface()->DrawSetTextFont(m_hTextFont);
			surface()->DrawUnicodeString( killer );
			surface()->DrawGetTextPos( x, y );

#ifdef NEO
			if (hasAssists)
			{
				SetColorForNoticePlayer(iAssistsTeam);

				// Draw +
				surface()->DrawSetTextPos(x, y);
				surface()->DrawSetTextFont(m_hTextFont);
				surface()->DrawUnicodeString(assistsPlusSign);
				surface()->DrawGetTextPos(x, y);

				// Draw assists's name
				surface()->DrawSetTextPos(x, y);
				surface()->DrawSetTextFont(m_hTextFont);
				surface()->DrawUnicodeString(assists);
				surface()->DrawGetTextPos(x, y);
			}
#endif
		}

		// Draw death weapon
		//If we're using a font char, this will ignore iconTall and iconWide
#ifdef NEO
		icon->DrawSelf(x, y + yNeoIconOffset, iconWide, iconTall,
			(m_DeathNotices[i].iSuicide ? COLOR_NEO_ORANGE : COLOR_NEO_WHITE));
#else
		Color iconColor(255, 80, 0, 255);
		icon->DrawSelf(x, y, iconWide, iconTall, iconColor);
#endif
		x += iconWide;

		SetColorForNoticePlayer( iVictimTeam );

		// Draw victims name
		surface()->DrawSetTextPos( x, y );
		// reset the font, draw icon can change it
		surface()->DrawSetTextFont(m_hTextFont);
		surface()->DrawUnicodeString( victim );
	}

	// Now retire any death notices that have expired
	RetireExpiredDeathNotices();
}

//-----------------------------------------------------------------------------
// Purpose: This message handler may be better off elsewhere
//-----------------------------------------------------------------------------
void CHudDeathNotice::RetireExpiredDeathNotices( void )
{
	// Loop backwards because we might remove one
	int iSize = m_DeathNotices.Size();
	for ( int i = iSize-1; i >= 0; i-- )
	{
		if ( m_DeathNotices[i].flDisplayTime < gpGlobals->curtime )
		{
			m_DeathNotices.Remove(i);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Server's told us that someone's died
//-----------------------------------------------------------------------------
void CHudDeathNotice::FireGameEvent(IGameEvent* event)
{
	if (!g_PR)
		return;

	if (hud_deathnotice_time.GetFloat() == 0)
		return;

	// the event should be "player_death"
	int killer = engine->GetPlayerForUserID(event->GetInt("attacker"));
	int victim = engine->GetPlayerForUserID(event->GetInt("userid"));
#ifdef NEO
	const int assistsInt = event->GetInt("assists");
	int assists = engine->GetPlayerForUserID(assistsInt);
	bool hasAssists = assists > 0;
#endif
	const char* killedwith = event->GetString("weapon");

	char fullkilledwith[128];
	if (killedwith && *killedwith)
	{
		Q_snprintf(fullkilledwith, sizeof(fullkilledwith), "death_%s", killedwith);
	}
	else
	{
		fullkilledwith[0] = 0;
	}

	// Do we have too many death messages in the queue?
	if (m_DeathNotices.Count() > 0 &&
		m_DeathNotices.Count() >= (int)m_flMaxDeathNotices)
	{
		// Remove the oldest one in the queue, which will always be the first
		m_DeathNotices.Remove(0);
	}

	// Get the names of the players
	const char* killer_name = g_PR->GetPlayerName(killer);
	const char* victim_name = g_PR->GetPlayerName(victim);
#ifdef NEO
	const char* assists_name = g_PR->GetPlayerName(assists);
	if (!assists_name)
		assists_name = "";
#endif

	if (!killer_name)
		killer_name = "";
	if (!victim_name)
		victim_name = "";

	// Make a new death notice
	DeathNoticeItem deathMsg;
	deathMsg.Killer.iEntIndex = killer;
	deathMsg.Victim.iEntIndex = victim;
	Q_strncpy(deathMsg.Killer.szName, killer_name, MAX_PLAYER_NAME_LENGTH);
	Q_strncpy(deathMsg.Victim.szName, victim_name, MAX_PLAYER_NAME_LENGTH);
	deathMsg.flDisplayTime = gpGlobals->curtime + hud_deathnotice_time.GetFloat();
	deathMsg.iSuicide = (!killer || killer == victim);

#ifdef NEO
	deathMsg.Assists.iEntIndex = assists;
	Q_strncpy(deathMsg.Assists.szName, assists_name, MAX_PLAYER_NAME_LENGTH);

	// NEO TODO (Rain): we're passing an event->"icon" as short here.
	// Only headshot's icon is implemented thus far, but we should
	// implement them all and use them to determine what to draw here
	// instead of doing all these test cases.

	if (deathMsg.iSuicide)
	{
		deathMsg.iconDeath = gHUD.GetIcon("neo_bus");
	}
	else
	{
		// Explosives
		if (V_stristr(killedwith, "gre") || V_stristr(killedwith, "det"))
		{
			deathMsg.bHeadshot = false;
			deathMsg.iconDeath = gHUD.GetIcon("neo_boom");
		}
		// Guns
		else
		{
			deathMsg.bHeadshot = (event->GetInt("icon") == NEO_DEATH_EVENT_ICON_HEADSHOT);
			deathMsg.iconDeath = gHUD.GetIcon(deathMsg.bHeadshot ? "neo_hs" : "neo_gun");
		}
	}

	if (!deathMsg.iconDeath)
	{
		Assert(false);
		// Can't find it, so use the default skull & crossbones icon
		deathMsg.iconDeath = m_iconD_skull;
	}
#else
	// Try and find the death identifier in the icon list
	deathMsg.iconDeath = gHUD.GetIcon(fullkilledwith);

	if (!deathMsg.iconDeath || deathMsg.iSuicide)
	{
		// Can't find it, so use the default skull & crossbones icon
		deathMsg.iconDeath = m_iconD_skull;
	}
#endif

	// Add it to our list of death notices
	m_DeathNotices.AddToTail(deathMsg);

	char sDeathMsg[512];

	// Record the death notice in the console
	if (deathMsg.iSuicide)
	{
		if (!strcmp(fullkilledwith, "d_worldspawn"))
		{
#ifdef NEO
			Q_snprintf(sDeathMsg, sizeof(sDeathMsg), "%s died.", deathMsg.Victim.szName);
#else
			Q_snprintf(sDeathMsg, sizeof(sDeathMsg), "%s died.\n", deathMsg.Victim.szName);
#endif
		}
		else	//d_world
		{
#ifdef NEO
			Q_snprintf(sDeathMsg, sizeof(sDeathMsg), "%s suicided.", deathMsg.Victim.szName);
#else
			Q_snprintf(sDeathMsg, sizeof(sDeathMsg), "%s suicided.\n", deathMsg.Victim.szName);
#endif
		}
	}
	else
	{
#ifdef NEO
		// These phrases are the same as in original NT to provide plugin/stats tracking website/etc compatibility.
#define HEADSHOT_LINGO "headshot'd"
#define NON_HEADSHOT_LINGO "killed"
#endif

		Q_snprintf(sDeathMsg, sizeof(sDeathMsg), "%s %s %s",
			deathMsg.Killer.szName,
#ifdef NEO
			(deathMsg.bHeadshot ? HEADSHOT_LINGO : NON_HEADSHOT_LINGO),
#else
			"killed"
#endif
			deathMsg.Victim.szName);

		if ((*fullkilledwith != NULL) && (*fullkilledwith > 13))
		{
#ifdef NEO
			Q_strncat(sDeathMsg, VarArgs(" with %s.", fullkilledwith + 6), sizeof(sDeathMsg), COPY_ALL_CHARACTERS);
#else
			Q_strncat(sDeathMsg, VarArgs(" with %s.\n", fullkilledwith + 6), sizeof(sDeathMsg), COPY_ALL_CHARACTERS);
#endif
		}
	}

#ifdef NEO
	if (hasAssists)
	{
		Q_strncat(sDeathMsg, VarArgs(" Assists: %s.", assists_name), sizeof(sDeathMsg), COPY_ALL_CHARACTERS);
	}
	Msg("%s\n", sDeathMsg);
#else
	Msg( "%s", sDeathMsg );
#endif

}



