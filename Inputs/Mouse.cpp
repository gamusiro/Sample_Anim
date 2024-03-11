/*===================================================================================
*    Date : 2023/06/28(êÖ)
*        Author		: Gakuto.S
*        File		: Mouse.cpp
*        Detail		:
===================================================================================*/
#include "Mouse.h"

POINT Mouse::m_currently;
POINT Mouse::m_last;
float Mouse::m_delta;

void Mouse::Update()
{
	m_last = m_currently;
	GetCursorPos(&m_currently);
}

void Mouse::SetCursourPos(int x, int y)
{
	SetCursorPos(x, y);
}

void Mouse::SetWheelDelta(float delta)
{
	m_delta = delta;
}
