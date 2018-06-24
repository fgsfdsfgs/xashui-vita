/*
YesNoMessageBox.h - simple generic yes/no message box
Copyright (C) 2017 a1batross

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "extdll_menu.h"
#include "BaseMenu.h"
#include "Action.h"
#include "PicButton.h"
#include "YesNoMessageBox.h"
#include "Utils.h"

static void ToggleInactiveInternalCb( CMenuBaseItem *pSelf, void *pExtra );

CMenuYesNoMessageBox::CMenuYesNoMessageBox( bool alert ) : BaseClass( "YesNoMessageBox")
{
	bAutoHide = true;
	m_bIsAlert = alert;
	iFlags |= QMF_DIALOG;
	dlgMessage1.iFlags = QMF_INACTIVE|QMF_DROPSHADOW;
	dlgMessage1.eTextAlignment = QM_TOP;

	if( m_bIsAlert )
	{
		yes.SetRect( 298, 204, UI_BUTTONS_WIDTH / 2, UI_BUTTONS_HEIGHT );
	}
	else
	{
		yes.SetRect( 188, 204, UI_BUTTONS_WIDTH / 2, UI_BUTTONS_HEIGHT );
	}
	no.SetRect( 338, 204, UI_BUTTONS_WIDTH / 2, UI_BUTTONS_HEIGHT );

	yes.onActivated.pExtra = no.onActivated.pExtra = this;
	yes.bEnableTransitions = no.bEnableTransitions = false;

	SET_EVENT_MULTI( yes.onActivated,
	{
		CMenuYesNoMessageBox *msgBox = (CMenuYesNoMessageBox*)pExtra;

		if( msgBox->bAutoHide ) msgBox->Hide();
		msgBox->onPositive( msgBox );

	});

	SET_EVENT_MULTI( no.onActivated,
	{
		CMenuYesNoMessageBox *msgBox = (CMenuYesNoMessageBox*)pExtra;

		if( msgBox->bAutoHide ) msgBox->Hide();
		msgBox->onNegative( msgBox );
	});

	m_bSetYes = m_bSetNo = false;
	m_bIsAlert = alert;

	szName = "CMenuYesNoMessageBox";
}

/*
==============
CMenuYesNoMessageBox::Init
==============
*/
void CMenuYesNoMessageBox::_Init( void )
{
	if( !m_bSetYes )
		SetPositiveButton( "Ok", PC_OK );

	if( !m_bSetNo )
		SetNegativeButton( "Cancel", PC_CANCEL );

	if( !(bool)onNegative )
		onNegative = CEventCallback::NoopCb;

	if( !(bool)onPositive )
		onPositive = CEventCallback::NoopCb;

	background.bForceColor = true;
	background.iColor = uiPromptBgColor;
	AddItem( background );
	AddItem( dlgMessage1 );
	AddItem( yes );

	// alert dialog has single OK button
	if( !m_bIsAlert )
		AddItem( no );
}

/*
==============
CMenuYesNoMessageBox::VidInit
==============
*/
void CMenuYesNoMessageBox::_VidInit( void )
{
	SetRect( DLG_X + 192, 256, 640, 256 );
	pos.x += uiStatic.xOffset;
	pos.y += uiStatic.yOffset;
	CalcPosition();
	CalcSizes();

	dlgMessage1.SetRect( 0, 24, 640, 256 - 24 );
	dlgMessage1.SetCharSize( QM_DEFAULTFONT );

}

/*
==============
CMenuYesNoMessageBox::Draw
==============
*/
void CMenuYesNoMessageBox::Draw( void )
{
	UI_FillRect( 0,0, gpGlobals->scrWidth, gpGlobals->scrHeight, 0x40000000 );
	CMenuBaseWindow::Draw();
}

/*
==============
CMenuYesNoMessageBox::Key
==============
*/
const char *CMenuYesNoMessageBox::Key(int key, int down)
{
	if( UI::Key::IsEscape( key ) && down )
	{
		Hide();
		onNegative( this );

		return uiSoundNull;
	}
	else
	{
		return CMenuBaseWindow::Key( key, down );
	}
}

/*
==============
CMenuYesNoMessageBox::SetMessage
==============
*/
void CMenuYesNoMessageBox::SetMessage( const char *msg )
{
	dlgMessage1.szName = ( msg );
}

/*
==============
CMenuYesNoMessageBox::SetPositiveButton
==============
*/
void CMenuYesNoMessageBox::SetPositiveButton( const char *msg, EDefaultBtns buttonPic, int extrawidth )
{
	m_bSetYes = true;
	yes.szName = msg;
	yes.SetPicture( buttonPic );
	yes.SetRect(  (m_bIsAlert?298:188) - extrawidth / 2, 204, UI_BUTTONS_WIDTH / 2 + extrawidth, UI_BUTTONS_HEIGHT );
}

/*
==============
CMenuYesNoMessageBox::SetNegativeButton
==============
*/
void CMenuYesNoMessageBox::SetNegativeButton( const char *msg, EDefaultBtns buttonPic, int extrawidth )
{
	m_bSetNo = true;
	no.szName = msg;
	no.SetPicture( buttonPic );
	no.SetRect( 338 + extrawidth / 2, 204, UI_BUTTONS_WIDTH / 2 + extrawidth, UI_BUTTONS_HEIGHT );
}

/*
==============
CMenuYesNoMessageBox::HighlightChoice
==============
*/
void CMenuYesNoMessageBox::HighlightChoice( EHighlight yesno )
{
	if( yesno == NO_HIGHLIGHT )
	{
		yes.bPulse = no.bPulse = false;
	}
	else
	{
		yes.bPulse = yesno == HIGHLIGHT_YES ? true : false;
		no.bPulse = yesno == HIGHLIGHT_NO ? true : false;
	}
}

CEventCallback CMenuYesNoMessageBox::MakeOpenEvent()
{
	return CEventCallback( OpenCb, this );
}


/*
==============
CMenuYesNoMessageBox::ToggleInactiveCb
==============
*/
void CMenuYesNoMessageBox::OpenCb( CMenuBaseItem *, void *pExtra )
{
	ToggleInactiveInternalCb( (CMenuBaseItem*)pExtra, NULL );
}

/*
==============
CMenuYesNoMessageBox::ToggleInactiveCb
==============
*/
static void ToggleInactiveInternalCb( CMenuBaseItem *pSelf, void * )
{
	pSelf->ToggleVisibility();
}

void CMenuYesNoMessageBox::UI_ShowMessageBox( void )
{
	static char msg[1024];
	static CMenuYesNoMessageBox msgBox( true );

	Q_strncpy( msg, EngFuncs::CmdArgv(1), sizeof( msg ) );

	if( !UI_IsVisible() )
	{
		UI_Main_Menu();
		UI_SetActiveMenu( TRUE );
	}

	if( strstr( msg, "m_ignore") || strstr( msg, "touch_enable" ) || strstr( msg, "joy_enable" ) )
	{
		static CMenuYesNoMessageBox msgBoxInputDev( false );
		static bool init;

		if( !init )
		{
			msgBoxInputDev.SetPositiveButton("Ok", PC_OK, 100 );
			msgBoxInputDev.SetNegativeButton("Config", PC_CONFIG, -20 );
			msgBoxInputDev.onNegative = UI_InputDevices_Menu;
			msgBoxInputDev.yes.SetCoord( 200, 204 );

			init = true;
		}

		msgBoxInputDev.SetMessage( msg );
		msgBoxInputDev.Show();
		msgBoxInputDev.yes.SetCoord( 200, 204 );
		return;
	}

	msgBox.SetMessage( msg );
	msgBox.Show();
}
ADD_COMMAND( menu_showmessagebox, CMenuYesNoMessageBox::UI_ShowMessageBox );
