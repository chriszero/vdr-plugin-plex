#include "hlsPlayerControl.h"

cHlsPlayerControl::cHlsPlayerControl(cHlsPlayer* Player, std::string title) :cControl(Player)
{
	m_pPlayer = Player;
	m_title = title;
}

cHlsPlayerControl::~cHlsPlayerControl()
{
}

cString cHlsPlayerControl::GetHeader(void)
{
	return m_title.c_str();
}

void cHlsPlayerControl::Hide(void)
{
}

void cHlsPlayerControl::Show(void)
{
}
