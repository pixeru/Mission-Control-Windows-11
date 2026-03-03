// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// YOU MAY USE THIS CODE AS YOU WISH FOR PERSONAL PURPOSES ONLY.
// YOU MAY NOT USE THIS CODE IN PART OR IN WHOLE IN A COMMERCIAL PRODUCT
// WITHOUT THE EXPRESS WRITTEN PERMISSION OF EMCEE APP SOFTWARE
// 9622 SANDHILL ROAD, MIDDLETON, WI 53562
// Copyright (c) Emcee App Software. All rights reserved
#pragma unmanaged
#include  "stdafx.h"
#pragma managed
#include <vcclr.h>
#pragma unmanaged
#include "McSettingsEditor.h"
#include "McModernAppMgr.h"
#include "Emcee.h"
#include "McLabelW.h"
#include "McColorTools.h"
#include "McMainW.h"

#pragma managed

System::Void McSettingsEditor::allowHotCornerCB_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
{
	BOOL checked = allowHotCornerCB->Checked == true ? TRUE : FALSE;
	if ( checked )
	{
		if ( data->mouseCornerActivate == 0 )
			data->mouseCornerActivate = MCA_UPPERRIGHT;
	}
	else
		data->mouseCornerActivate = 0;
	displayData();
}	

System::Void McSettingsEditor::customHotKeyCB_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
{
	BOOL checked = customHotKeyCB->Checked == true ? TRUE : FALSE;
	if ( checked )
		hotKeyPanel->Visible = true;
	else
	{
		hotKeyPanel->Visible = false;
		data->hotKeyCode = _HK_DEFAULT;
		displayData();
	}
}
#if MC_DESKTOPS
System::Void McSettingsEditor::showAllDesktops_CheckedChanged( System::Object ^sender, System::EventArgs^ e )
{
	BOOL checked = showAllDesktops->Checked == true ? TRUE : FALSE;
	data->showAllDesktops = checked;
}
#endif
System::Void McSettingsEditor::useAppBar_CheckedChanged( System::Object ^sender, System::EventArgs^ e )
{
	BOOL checked = useAppBar->Checked == true ? TRUE : FALSE;
	data->useAppBar = checked;
}


System::Void McSettingsEditor::multiMonitor_CheckedChanged( System::Object ^sender, System::EventArgs^ e )
{
	if (MC::getMD( ))
	{
		BOOL mm = multiMonitor->Checked == true ? TRUE : FALSE;
		data->multiMonitor = mm;
	}
}

System::Void McSettingsEditor::disableLabelCB_CheckChanged(System::Object^  sender, System::EventArgs^  e)
{
	BOOL checked = disableLabelCB->Checked == true ? TRUE : FALSE;
	data->disableLabels = checked;
}

System::Void McSettingsEditor::autoColorCheckCB_CheckedChanged( System::Object^  sender, System::EventArgs^  e )
{
	BOOL checked = autoColorCheck->Checked == true ? TRUE : FALSE;
	if (checked)
	{
		data->dtAutoColor = TRUE;
	}
	else
		data->dtAutoColor = FALSE;
	displayData( );
}

System::Void McSettingsEditor::colorPick_Click( System::Object^  sender, System::EventArgs^  e )
{

	autoColorCheck->Checked = FALSE;
	
	colorDialog->AllowFullOpen = true;

	Color myColor = Color::FromArgb( 
		 GetRValue( data->dtBgColor ), GetGValue( data->dtBgColor ), GetBValue( data->dtBgColor ) );

	int paletteCount = McModernAppMgr::GetPaletteCount( );
	cli::array<Int32 >^temp0 = gcnew cli::array< Int32 >( paletteCount+1 );
	for (int i = 0; i < paletteCount; i++)
		temp0[i] = McModernAppMgr::GetPaletteColor( i );
	temp0[paletteCount] = data->dtBgColor;

	 colorDialog->CustomColors = temp0;
	 
	 colorDialog->Color = myColor;
	 colorDialog->ShowHelp = true;
	 if ( colorDialog->ShowDialog() == ::System::Windows::Forms::DialogResult::OK )
	 {
		 colorDisplayLabel->BackColor = colorDialog->Color;
		 int rgb = colorDialog->Color.ToArgb();
		 int red = (rgb & 0xFF0000)>>16;
		 int green = (rgb & 0x00FF00)>>8;
		 int blue = rgb & 0x0000FF;
		 data->dtBgColor = RGB( red, green, blue );
		 displayData( );
	 }
}

System::Void McSettingsEditor::thumbnailSpacingTB_ValueChanged(System::Object^  sender, System::EventArgs^  e)
{
	double value = thumbnailSpacingTB->Value * 0.0625;
	data->thumbnailSpacing = value;
}

System::Void McSettingsEditor::immersiveThumbHeightTB_ValueChanged(System::Object^  sender, System::EventArgs^  e)
{
	data->immersiveTileHeight = immersiveThumbHeightTB->Value * 0.0625;
}

System::Void McSettingsEditor::animationSpeedTB_ValueChanged(System::Object^  sender, System::EventArgs^  e)
{
	int value = animationSpeedTB->Value;
	data->animationSpeed = value;
}

System::Void McSettingsEditor::labelFontSizeTB_ValueChanged(System::Object^  sender, System::EventArgs^  e)
{
	int value = labelFontSizeTB->Value;
	data->labelFontSize = value;
}

System::Void McSettingsEditor::cancelButton_Click(System::Object^  sender, System::EventArgs^  e)
{
	DialogResult = System::Windows::Forms::DialogResult::Cancel;
	disposition = settingsCancel;
}

System::Void McSettingsEditor::acceptButton_Click(System::Object^ sender, System::EventArgs^ e )
{
	if ( !McHotKeyMgr::testHotKey( data->hotKeyCode ) )
	{
		System::Windows::Forms::DialogResult result = 
			MessageBox::Show( 
				String::Concat
				( 
					"\n\nUnable to set your hot key to ",
					_HK_MODIFIER( data->hotKeyCode ) == MOD_NONE ? "" :
					_HK_MODIFIER( data->hotKeyCode ) == MOD_CONTROL ? "<CTRL>" :
					_HK_MODIFIER( data->hotKeyCode ) == MOD_ALT ? "<ALT>" : 
					_HK_MODIFIER( data->hotKeyCode ) == MOD_SHIFT ? "<SHIFT>" : "<WIN>","<",
					gcnew String( McHotKeyMgr::getStringFromCode( _HK_VKEY( data->hotKeyCode ) ) ),">"
					"-- some other application, or Windows, has reserved it.\n\nPress OK to proceed without changing the hot key, or CANCEL to continue editing the settings.\n\n"
				),
				"WARNING",
				MessageBoxButtons::OKCancel,
				MessageBoxIcon::Warning,
				MessageBoxDefaultButton::Button1 );
		if ( result == System::Windows::Forms::DialogResult::Cancel )
			return;
		data->hotKeyCode = backupData->hotKeyCode;
	}

	if (data->labelFontSize != backupData->labelFontSize)
		McLabelW::NewLabelFontSize( );

	DialogResult = System::Windows::Forms::DialogResult::OK;
	disposition = settingsAccept;
}

System::Void McSettingsEditor::defaultsButton_Click(System::Object^ sender, System::EventArgs^ e )
{
	data->set( backupData );
	displayData();
}

System::Void McSettingsEditor::groupsTab_Enter(System::Object^ sender, System::EventArgs^ e )
{
	clearAppAndGroupEntry();
}

System::Void McSettingsEditor::groupListBox_SelectedValueChanged(System::Object^ sender, System::EventArgs^ e )
{
	clearAppAndGroupEntry();
	if ( groupListBox->SelectedIndex == -1 )
		return;

	String ^selected = groupListBox->SelectedItem->ToString();

	groupContentsBox->Items->Clear();

	if ( selected == L"<Excluded Apps>" )
	{
		for ( list<string>::iterator st = data->excludeList.begin(); st != data->excludeList.end(); st++ )
				groupContentsBox->Items->Add(gcnew String( (*st).data() ) );
	}
	else for ( list<McGroup *>::iterator it = data->masterList.begin(); it != data->masterList.end(); it++ )
	{
		McGroup *g = *it;
		if ( 0 == String::Compare( selected, gcnew String( g->groupName.data() ) ) )
		{
			for ( list<string>::iterator st = g->stringList.begin(); st != g->stringList.end(); st++ )
				groupContentsBox->Items->Add(gcnew String( (*st).data() ) );
		}
	}
}

System::Void McSettingsEditor::groupContentsBox_SelectedValueChanged( System::Object^ sender, System::EventArgs^ e )
{
	clearAppAndGroupEntry();
}

System::Void McSettingsEditor::addAppGroupButton_Click(System::Object^  sender, System::EventArgs^  e)
{
	if ( AAGstate != AAG_IDLE )
		clearAppAndGroupEntry();
	addAppOrGroupText->Text = "";
	addPanel->Visible = true;
	addAppOrGroupText->Visible = true;
	addAppOrGroupText->Focus();
	addAppOrGroupButton->Text = L"Type Group Name\nAbove and\nThen Click Here";
	addAppOrGroupButton->Visible = true;
	AAGstate = AAG_GROUP;
}

System::Void McSettingsEditor::deleteAppGroupButton_Click(System::Object^  sender, System::EventArgs^  e)
{
	if ( groupListBox->SelectedIndex < 0 ) 
	{
		MessageBox::Show( L"No Group is Selected", L"Error", MessageBoxButtons::OK,
				MessageBoxIcon::Error,
				MessageBoxDefaultButton::Button1 );
		return;
	}
	String ^item = groupListBox->SelectedItem->ToString();
	if ( item == L"<Excluded Apps>" )
	{
		MessageBox::Show( L"You can't delete the <Excluded Apps> group.", L"", MessageBoxButtons::OK,
				MessageBoxIcon::Error,
				MessageBoxDefaultButton::Button1 );
		return;
	}
	System::Windows::Forms::DialogResult result = 
		MessageBox::Show( String::Concat( "Do you really want to delete the group ", item, "?" ),
		"Confirmation Needed", MessageBoxButtons::YesNo,
				MessageBoxIcon::Information,
				MessageBoxDefaultButton::Button1 );
	if ( result == ::DialogResult::Yes )

		removeAppGroup( item );
}

System::Void McSettingsEditor::addAppToGroup_Click(System::Object^ sender, System::EventArgs^ e )
{
	if ( AAGstate != AAG_IDLE )
		clearAppAndGroupEntry();
	int selection = groupListBox->SelectedIndex;
	if ( selection < 0 ) 
	{
		MessageBox::Show( L"No Group is Selected", L"Error", MessageBoxButtons::OK,
				MessageBoxIcon::Error,
				MessageBoxDefaultButton::Button1 );
		addAppToGroupButton->Update();
		return;
	}
	addPanel->Visible = true;
	addAppOrGroupText->Visible = true;
	addAppOrGroupText->Focus();
	addAppOrGroupButton->Text = L"Type App Name\nAbove and\nThen Click Here";
	addAppOrGroupButton->Visible = true;
	addActiveAppLabel->Visible = true;
	addActiveAppCombo->Visible = true;
	addActiveAppCombo->Text = "Select One";

	AAGstate = AAG_APP;
}

System::Void McSettingsEditor::deleteAppFromGroup_Click(System::Object^ sender, System::EventArgs^ e )
{
	if ( groupContentsBox->SelectedIndex < 0 ) 
	{
		MessageBox::Show( L"No App is Selected", L"Error", MessageBoxButtons::OK,
				MessageBoxIcon::Error,
				MessageBoxDefaultButton::Button1 );
		return;
	}

	if ( groupListBox->SelectedIndex < 0 )
	{
		MessageBox::Show( L"No Group is Selected", L"Error", MessageBoxButtons::OK,
				MessageBoxIcon::Error,
				MessageBoxDefaultButton::Button1 );
		return;
	}

	String ^item = groupContentsBox->SelectedItem->ToString();
	String ^gitem = groupListBox->SelectedItem->ToString();

	System::Windows::Forms::DialogResult result = 
		MessageBox::Show( String::Concat( "Do you really want to remove the App ", item, " from group ",
			gitem, "?" ),
		"Confirmation Needed", MessageBoxButtons::YesNo,
				MessageBoxIcon::Information,
				MessageBoxDefaultButton::Button1 );
	if ( result == ::DialogResult::Yes )
		removeAppFromGroup( item, gitem );
}

System::Void McSettingsEditor::addAppOrGroupButton_Click(System::Object^ sender, System::EventArgs^ e )
{
	String^ value = gcnew String(addAppOrGroupText->Text->ToString()->Trim());
	if ( value->Length > 0 )
		switch( AAGstate )
		{
		case AAG_GROUP:
			addAppGroup( value );
			break;
		case AAG_APP:
			addAppToGroup( value, groupListBox->SelectedItem->ToString() );
			break;
		default:
			break;
		}
	clearAppAndGroupEntry();
}

System::Void McSettingsEditor::addActiveAppCombo_SelectedValueChanged(System::Object^ sender, System::EventArgs^ e )
{
	String^ value = gcnew String(addActiveAppCombo->Text->ToString()->Trim());
	if ( AAGstate == AAG_APP && value->Length > 0 )
		addAppToGroup( value, groupListBox->SelectedItem->ToString() );
	clearAppAndGroupEntry();
}

void McSettingsEditor::clearAppAndGroupEntry()
{
	addPanel->Visible = false;
	addAppOrGroupText->Visible = false;
	addAppOrGroupText->Text="";
	addAppOrGroupButton->Visible = false;
	addActiveAppLabel->Visible = false;
	addActiveAppCombo->Visible = false;
	addActiveAppCombo->Text="";
	AAGstate = AAG_IDLE;
}

BOOL McSettingsEditor::doActivate( McPropsMgr *_props, list<string> *apps )
{
	McPropData myData;
	data = &myData;
	backupData = _props->getData();
	data->set( _props->getData() );
	displayData();

	addActiveAppCombo->Items->Clear();
	if ( apps )
		for ( list<string>::iterator it = apps->begin(); it != apps->end(); it++ )
			addActiveAppCombo->Items->Add( gcnew String( (*it).data() ) );

	COLORREF settingsColor = McModernAppMgr::GetSettingsColor();

	int r = GetRValue( settingsColor );
	int g = GetGValue( settingsColor );
	int b = GetBValue( settingsColor );

	BackColor = Color::FromArgb( 0xFF000000 | 
		(( r << 16 ) + (g << 8) + b ) );
	copyright->BackColor = BackColor;
	tabGroup->BackColor = BackColor;
	appearanceTab->BackColor = BackColor;
	activationTab->BackColor = BackColor;
	groupsTab->BackColor = BackColor;

	ForeColor = Color::FromArgb( 0xFFFFFFFF );

	copyright->ForeColor = ForeColor;
	appearanceTab->ForeColor = ForeColor;
	activationTab->ForeColor = ForeColor;
	groupsTab->ForeColor = ForeColor;
	thumbnailSpacingTB->ForeColor = ForeColor;
	thumbnailSpacingTB->BackColor = BackColor;
	animationSpeedTB->BackColor = BackColor;
	animationSpeedTB->ForeColor = ForeColor;
	labelFontSizeTB->BackColor = BackColor;
	labelFontSizeTB->ForeColor = ForeColor;
	immersiveThumbHeightTB->BackColor = BackColor;
	immersiveThumbHeightTB->ForeColor = ForeColor;

	if ( !haveActivated )
	{
		McRect *mdR = McMonitorsMgr::getMainMonitor( )->getMonitorSize( );
		int left;
		int screenWidth = mdR->getWidth( );
		int screenHeight = mdR->getHeight( );

		int barHeight = Size.Height;
		int formLeft = (screenWidth-tabGroup->Size.Width)/2;

		ClientSize = System::Drawing::Size( screenWidth, barHeight );
		Location = System::Drawing::Point( 
			0, 0 );

		tabGroup->Location = System::Drawing::Point( formLeft, 8 );

		int top	= Height - 10 - acceptButton->Height;
		acceptButton->Location = System::Drawing::Point( formLeft, top );

		left = (screenWidth-formLeft) - defaultsButton->Size.Width;
		defaultsButton->Location = System::Drawing::Point( left, top );

		left = (screenWidth - cancelButton->Size.Width)/2;
		cancelButton->Location = System::Drawing::Point( left, top );
		int h = Height;
		int ch = copyright->Height;
		copyright->Location = System::Drawing::Point( 10, Height-copyright->Height - 5 );
		copyright->Visible = true;

		settings->Location = System::Drawing::Point( screenWidth - settings->Width-16, 10 );
		ulButton->Location = System::Drawing::Point(10,1);

		urButton->Location = System::Drawing::Point(
			activationTab->Width - urButton->Width - 15, 
			1 );
		lrButton->Location = System::Drawing::Point( 
			activationTab->Width  - lrButton->Width  - 15, 
			activationPanel->Bottom - 20 );

		haveActivated = TRUE;
	}

	System::Windows::Forms::DialogResult result = ShowDialog();

	if ( result == System::Windows::Forms::DialogResult::OK )
		_props->setData( data );

	return (result == System::Windows::Forms::DialogResult::OK);
}

void McSettingsEditor::displayData( )
{
	_ignore = TRUE;
	autoColorCheck->Checked = data->dtAutoColor ? true : false;
	allowHotCornerCB->Checked = data->mouseCornerActivate ? true : false;
	disableLabelCB->Checked = data->disableLabels ? true : false;
#if MC_DESKTOPS
	showAllDesktops->Checked = data->showAllDesktops ? true : false;
#endif
	useAppBar->Checked = data->useAppBar ? true : false;
	if (MC::getMD( ))
		multiMonitor->Checked = data->multiMonitor ? true : false;
	long hotKeyCode = data->hotKeyCode;
	customHotKeyCB->Checked = (hotKeyCode == _HK_DEFAULT) ? false : true;
	hotKeyPanel->Visible = customHotKeyCB->Checked;
	keyCombo->SelectedIndex = McHotKeyMgr::getIndexFromCode( _HK_VKEY( hotKeyCode ) );
	UINT mod = _HK_MODIFIER( hotKeyCode );
	if (mod == MOD_NONE)
		nokeyButton->Checked = true;
	else if (mod & MOD_CONTROL)
		ctrlButton->Checked = true;
	else if (mod & MOD_ALT)
		altButton->Checked = true;
	else if (mod & MOD_SHIFT)
		shiftButton->Checked = true;
	else if (mod & MOD_WIN)
		winButton->Checked = true;

	keyCombo->SelectedIndex = McHotKeyMgr::getIndexFromCode( _HK_VKEY( hotKeyCode ) );

	if (data->mouseCornerActivate)
	{
		ulButton->Visible = true;
		urButton->Visible = true;
		lrButton->Visible = true;
		if (data->mouseCornerActivate & MCA_UPPERLEFT)
			ulButton->Checked = true;
		else if ( data->mouseCornerActivate & MCA_UPPERRIGHT )
			urButton->Checked = true;
		else if ( data->mouseCornerActivate & MCA_LOWERRIGHT )
			lrButton->Checked = true;
	}
	else
	{
		ulButton->Visible = false;
		urButton->Visible = false;
		lrButton->Visible = false;
	}

	int val; 
		
	val = (int) floor( 0.5 + data->immersiveTileHeight / 0.0625 );48;

	immersiveThumbHeightTB->Value =  min( 48, max( val, 8 ) );

	oldImmersiveValue = immersiveThumbHeightTB->Value;

	val = (int)floor( 0.5 + data->thumbnailSpacing / 0.0625 );
	thumbnailSpacingTB->Value = min( 8, max( 2, val ) );
	animationSpeedTB->Value = min( 9, max( 1, data->animationSpeed ) );
	labelFontSizeTB->Value = min( 42, max( 6, data->labelFontSize ) );

	COLORREF bgColor = data->dtAutoColor ? McModernAppMgr::GetAutoColor( ) : data->dtBgColor ;

	colorDisplayLabel->BackColor =  Color::FromArgb( _EMCEE_FORM_BG_ALPHA, 
		 GetRValue( bgColor ), GetGValue( bgColor ), GetBValue( bgColor ) );

	groupListBox->Items->Clear();
	groupContentsBox->Items->Clear();

	for ( list<McGroup *>::iterator it = data->masterList.begin(); it != data->masterList.end(); it++ )
	{
		McGroup *group = *it;
		groupListBox->Items->Add( gcnew String( group->groupName.data() ) );
	}

	groupListBox->Items->Add( "<Excluded Apps>" );
	_ignore = FALSE;
}

void McSettingsEditor::addAppGroup( String ^newGroupName )
{
	char *c =  (char*)(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(newGroupName)).ToPointer();
	BOOL doAdd = TRUE;
	for ( list<McGroup *>::iterator it = data->masterList.begin(); it != data->masterList.end(); it++ )
	{
		McGroup *g = *it;
		if ( !_stricmp( c, g->groupName.c_str() ) )
		{
			MessageBox::Show( "The new group name matches and existing app group.", "Error", MessageBoxButtons::OK,
				MessageBoxIcon::Error,
				MessageBoxDefaultButton::Button1 );
			doAdd = FALSE;
			break;
		}	
	}

	if ( doAdd )
	{
		data->masterList.push_back( new McGroup( c ) );
		groupListBox->Items->Insert( 0, newGroupName );
	}

	System::Runtime::InteropServices::Marshal::FreeHGlobal(IntPtr((void*)c));
}

void McSettingsEditor::removeAppGroup( String ^oldGroupName )
{
	if ( groupListBox->SelectedItem == oldGroupName )
	{
		groupListBox->ClearSelected();
		groupContentsBox->Items->Clear();
	}
	char *c =  (char*)(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(oldGroupName)).ToPointer();

	groupListBox->Items->Remove( oldGroupName );

	for ( list<McGroup *>::iterator it = data->masterList.begin(); it != data->masterList.end(); it++ )
	{
		McGroup *g = *it;
		if ( !_stricmp( c, g->groupName.c_str() ) )
		{
			data->masterList.remove( g );
			delete g;
			break;
		}
	}
	System::Runtime::InteropServices::Marshal::FreeHGlobal(IntPtr((void*)c));
}

void McSettingsEditor::addAppToGroup( String^ AppName, String^ GroupName )
{
	char *appName =  (char*)(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(AppName)).ToPointer();
	char *groupName =  (char*)(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(GroupName)).ToPointer();

	removeAppFromList( appName, &data->excludeList );
	if ( !_stricmp( groupName, "<Excluded Apps>" ) )
	{
		string newAppName( appName );
		data->excludeList.push_back( newAppName );
	}
	for ( list<McGroup *>::iterator it = data->masterList.begin(); it != data->masterList.end(); it++ )
	{
		McGroup *g = *it;
		removeAppFromList( appName, &g->stringList );
		if ( !_stricmp( groupName, g->groupName.c_str() ) )
		{
			string newAppName( appName );
			g->stringList.push_back( newAppName );
		}
	}

	if ( false == groupContentsBox->Items->Contains( AppName ) )
		groupContentsBox->Items->Add( AppName );

	System::Runtime::InteropServices::Marshal::FreeHGlobal(IntPtr((void*)groupName));
	System::Runtime::InteropServices::Marshal::FreeHGlobal(IntPtr((void*)appName));
}

void McSettingsEditor::removeAppFromGroup( String ^AppName, String^ GroupName )
{
	char *appName =  (char*)(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(AppName)).ToPointer();
	char *groupName =  (char*)(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(GroupName)).ToPointer();

	if ( !_stricmp( groupName, "<Excluded Apps>" ) )
		removeAppFromList( appName, &data->excludeList );
	else
		for ( list<McGroup *>::iterator it = data->masterList.begin(); it != data->masterList.end(); it++ )
		{
			McGroup *g = *it;
			if ( !_stricmp( groupName, g->groupName.c_str() ) )
			{
				string s( appName );
				g->stringList.remove( s );
				break;
			}
		}

	groupContentsBox->Items->Remove( AppName );
	
	System::Runtime::InteropServices::Marshal::FreeHGlobal(IntPtr((void*)groupName));
	System::Runtime::InteropServices::Marshal::FreeHGlobal(IntPtr((void*)appName));
}

void McSettingsEditor::removeAppFromList( char *_iApp, list<string>* _list )
{
	for ( list<string>::iterator it = _list->begin(); it!=_list->end(); it++ )
		if ( !_stricmp( _iApp, (*it).c_str() ) )
		{
			string s( _iApp );
			_list->remove( *it );
			return;
		}
}

static LRESULT CALLBACK _windowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( uMsg == WM_PAINT )
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint( hwnd, &ps );
		FillRect( hdc, &ps.rcPaint, (HBRUSH)GetStockObject( BLACK_BRUSH ) );
		EndPaint( hwnd, &ps );
	}

	return DefWindowProc( hwnd, uMsg, wParam, lParam );
}

SETTINGS_DISPOSITION doSettings( list<string>* dtList )
{
	SETTINGS_DISPOSITION disposition;
	Application::EnableVisualStyles( );
	{
		McSettingsEditor settings;
		settings.doActivate( MC::getProperties( ), dtList );
		disposition = settings.disposition;
	}
	System::GC::Collect( );
	return disposition;
}

System::Void McSettingsEditor::hotKey_change( System::Object^ sender, System::EventArgs^ e )
{
	UINT key = McHotKeyMgr::getCodeByIndex(keyCombo->SelectedIndex);
	UINT modifier = MOD_NONE;
	if ( ctrlButton->Checked )
		modifier = MOD_CONTROL;
	else if ( altButton->Checked == true )
		modifier = MOD_ALT;
	else if ( shiftButton->Checked == true )
		modifier = MOD_SHIFT;
	else if ( winButton->Checked == true )
		modifier = MOD_WIN;
	data->hotKeyCode = _HK_COMPRESS( modifier, key );
}

System::Void McSettingsEditor::hotCorner_change( System::Object^sender, System::EventArgs^ e )
{
	int corner = 0;	
	if ( urButton->Checked == true )
		corner = MCA_UPPERRIGHT;
	else if ( lrButton->Checked == true )
		corner = MCA_LOWERRIGHT;
	if (ulButton->Checked == true)
		corner = MCA_UPPERLEFT;

	data->mouseCornerActivate = corner;

}

System::Void McSettingsEditor::doActivation( System::Object^ sender, System::EventArgs^ e )
{
	System::Object^t = this;
	HWND hwnd = static_cast<HWND>(Handle.ToPointer( ));
	AnimateWindow( hwnd, 200, AW_ACTIVATE | AW_BLEND );
	
}

System::Void McSettingsEditor::doDeactivation( System::Object^ sender, System::EventArgs^ e )
{
	AnimateWindow( static_cast<HWND>(Handle.ToPointer( )), 100, AW_HIDE | AW_SLIDE ); 
}
