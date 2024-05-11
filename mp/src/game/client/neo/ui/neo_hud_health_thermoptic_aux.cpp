#include "cbase.h"
#include "neo_hud_health_thermoptic_aux.h"

#include "c_neo_player.h"

#include "iclientmode.h"
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui/IScheme.h>

#include <engine/ivdebugoverlay.h>
#include "ienginevgui.h"

#include "neo_hud_elements.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

NEO_HUD_ELEMENT_DECLARE_FREQ_CVAR(HTA, 0.00695);

using vgui::surface;

ConVar neo_cl_hud_hta_enabled("neo_cl_hud_hta_enabled", "1", FCVAR_USERINFO,
	"Whether the HUD Health/ThermOptic/AUX module is enabled or not.", true, 0, true, 1);

CNEOHud_HTA::CNEOHud_HTA(const char* pElementName, vgui::Panel* parent)
	: CHudElement(pElementName), Panel(parent, pElementName)
{
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

	surface()->GetScreenSize(m_resX, m_resY);
	SetBounds(0, 0, m_resX, m_resY);

	// NEO HACK (Rain): this is kind of awkward, we should get the handle on ApplySchemeSettings
	vgui::IScheme* scheme = vgui::scheme()->GetIScheme(neoscheme);
	if (!scheme) {
		Assert(scheme);
		Error("CNEOHud_Ammo: Failed to load neoscheme\n");
	}

	m_hFont = scheme->GetFont("NHudOCRSmall");

	SetVisible(neo_cl_hud_hta_enabled.GetBool());

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION);
}

void CNEOHud_HTA::Paint()
{
	PaintNeoElement();

	SetFgColor(COLOR_TRANSPARENT);
	SetBgColor(COLOR_TRANSPARENT);

	BaseClass::Paint();
}

void CNEOHud_HTA::UpdateStateForNeoHudElementDraw()
{
	Assert(C_NEO_Player::GetLocalNEOPlayer());
}

void CNEOHud_HTA::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	surface()->GetScreenSize(m_resX, m_resY);
	SetBounds(0, 0, m_resX, m_resY);
}

void CNEOHud_HTA::DrawHTA() const
{
	auto player = C_NEO_Player::GetLocalNEOPlayer();
	Assert(player);

	const Color textColor = COLOR_WHITE;

	char value_Integrity[4]{ '\0' };
	char value_ThermOptic[4]{ '\0' };
	char value_Aux[4]{ '\0' };

	wchar_t unicodeValue_Integrity[4]{ L'\0' };
	wchar_t unicodeValue_ThermOptic[4]{ L'\0' };
	wchar_t unicodeValue_Aux[4]{ L'\0' };

	const int health = player->GetHealth();
	const int thermopticValue = static_cast<int>(roundf(player->m_HL2Local.m_cloakPower));
	const float thermopticPercent = player->CloakPower_CurrentVisualPercentage();
	const int aux = player->m_HL2Local.m_flSuitPower;
	const bool playerIsNotSupport = (player->GetClass() != NEO_CLASS_SUPPORT);

	inttostr(value_Integrity, 10, health);
	if (playerIsNotSupport)
	{
		inttostr(value_ThermOptic, 10, thermopticValue);
		inttostr(value_Aux, 10, aux);
	}

	const int valLen_Integrity = V_strlen(value_Integrity);
	const int valLen_ThermOptic = V_strlen(value_ThermOptic);
	const int valLen_Aux = V_strlen(value_Aux);

	g_pVGuiLocalize->ConvertANSIToUnicode(value_Integrity, unicodeValue_Integrity, sizeof(unicodeValue_Integrity));
	if (playerIsNotSupport)
	{
		g_pVGuiLocalize->ConvertANSIToUnicode(value_ThermOptic, unicodeValue_ThermOptic, sizeof(unicodeValue_ThermOptic));
		g_pVGuiLocalize->ConvertANSIToUnicode(value_Aux, unicodeValue_Aux, sizeof(unicodeValue_Aux));
	}

	int fontWidth, fontHeight;
	surface()->GetTextSize(m_hFont, L"THERM-OPTIC", fontWidth, fontHeight);

	// These are the constant res based scalings of the NT ammo/health box dimensions.
	const int xBoxWidth = m_resX * 0.2375;
	const int yBoxHeight = m_resY * (0.1 / 1.5);

	const int margin = neo_cl_hud_ammo_enabled.GetInt();
	DrawNeoHudRoundedBox(margin, m_resY - yBoxHeight - margin, xBoxWidth + margin, m_resY - margin);

	const int xPadding = 5; // TODO (Rain): make this relative to resolution scaling

	surface()->DrawSetTextFont(m_hFont);
	surface()->DrawSetTextColor(textColor);
	surface()->DrawSetTextPos(xPadding + margin, m_resY - yBoxHeight - (fontHeight / 1.5) + fontHeight * 1 - margin);
	surface()->DrawPrintText(L"INTEGRITY", 9);
	if (playerIsNotSupport)
	{
		surface()->DrawSetTextPos(xPadding + margin, m_resY - yBoxHeight - (fontHeight / 1.5) + fontHeight * 2 - margin);
		surface()->DrawPrintText(L"THERM-OPTIC", 11);
		surface()->DrawSetTextPos(xPadding + margin, m_resY - yBoxHeight - (fontHeight / 1.5) + fontHeight * 3 - margin);
		surface()->DrawPrintText(L"AUX", 3);
	}

	surface()->DrawSetTextPos(xBoxWidth - xPadding * 7 + margin, m_resY - yBoxHeight - (fontHeight / 1.5) + fontHeight * 1 - margin);
	surface()->DrawPrintText(unicodeValue_Integrity, valLen_Integrity);
	if (playerIsNotSupport)
	{
		surface()->DrawSetTextPos(xBoxWidth - xPadding * 7 + margin, m_resY - yBoxHeight - (fontHeight / 1.5) + fontHeight * 2 - margin);
		surface()->DrawPrintText(unicodeValue_ThermOptic, valLen_ThermOptic);
		surface()->DrawSetTextPos(xBoxWidth - xPadding * 7 + margin, m_resY - yBoxHeight - (fontHeight / 1.5) + fontHeight * 3 - margin);
		surface()->DrawPrintText(unicodeValue_Aux, valLen_Aux);
	}

	surface()->DrawSetColor(COLOR_WHITE);

	const int x_from = xPadding * 2 + fontWidth;
	const int x_to = xBoxWidth - xPadding * 8;
	const int x_len = x_to - x_from;

	// Integrity progress bar
	surface()->DrawFilledRect(
		x_from + margin,
		1.0 * (m_resY - yBoxHeight - (fontHeight / 1.5) + fontHeight * 1) - margin,
		x_to - x_len * (1 - health / 100.0) + margin,
		m_resY - yBoxHeight - (fontHeight / 1.5) + fontHeight * 1.75 - margin);

	if (playerIsNotSupport)
	{
		// ThermOptic progress bar
		surface()->DrawFilledRect(
			x_from + margin,
			1.0 * (m_resY - yBoxHeight - (fontHeight / 1.5) + fontHeight * 2) - margin,
			x_to - x_len * (1 - thermopticPercent / 100.0) + margin,
			m_resY - yBoxHeight - (fontHeight / 1.5) + fontHeight * 2.75 - margin);

		// AUX progress bar
		surface()->DrawFilledRect(
			x_from + margin,
			1.0 * (m_resY - yBoxHeight - (fontHeight / 1.5) + fontHeight * 3) - margin,
			x_to - x_len * (1 - aux / 100.0) + margin,
			m_resY - yBoxHeight - (fontHeight / 1.5) + fontHeight * 3.75 - margin);
	}
}

void CNEOHud_HTA::DrawNeoHudElement()
{
	if (!ShouldDraw())
	{
		return;
	}

	if (neo_cl_hud_hta_enabled.GetBool())
	{
		DrawHTA();
	}
}
