#pragma once
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Mission Control for Windows 11 by pixeru
// Source code originally by Emcee (https://sourceforge.net/projects/mcsoft/)
// Extensive development by pixeru due to discontinued development of original project.
#include "stdafx.h"
#include "Emcee.h"
#include "McPropsMgr.h"
#include "McMonitorsMgr.h"

#pragma managed

#define _EMCEE_FORM_BG_ALPHA 0xFF

#include <vcclr.h>

char *wchar2char( WCHAR *wstr, char *cstr );
WCHAR *char2wchar( char *cstr, WCHAR *wstr );

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

#define _TAB_PADDING 42

	enum AAG_STATE
	{
		AAG_IDLE = 0,
		AAG_GROUP = 1,
		AAG_APP = 2
	};

public ref class McSettingsEditor : public System::Windows::Forms::Form
{
	public:
		McSettingsEditor(void)
		{
			haveActivated = FALSE;
			_ignore = FALSE;
			InitializeComponent();
		}

		SETTINGS_DISPOSITION disposition;

	protected:

		/// Clean up any resources being used.

		~McSettingsEditor()
		{
		}

		!McSettingsEditor()
		{
			disposition = settingsOther;
		}

	protected:

	private: 
		
		int dialogWidth, dialogHeight;
		int tabWidth, tabHeight;
		AAG_STATE AAGstate;

	System::Windows::Forms::ColorDialog^  colorDialog;
	System::Windows::Forms::Button^  cancelButton;
	System::Windows::Forms::TabControl^  tabGroup;
	System::Windows::Forms::TabPage^  activationTab;
	System::Windows::Forms::FlowLayoutPanel^ activationPanel;
	System::Windows::Forms::TableLayoutPanel^ actTickBoxPanel;
	System::Windows::Forms::CheckBox^  allowHotCornerCB;
	System::Windows::Forms::CheckBox^  customHotKeyCB;
#if MC_DESKTOPS
	System::Windows::Forms::CheckBox^ showAllDesktops;
#endif
	System::Windows::Forms::CheckBox^ useAppBar;
	System::Windows::Forms::CheckBox^ multiMonitor;
	System::Windows::Forms::CheckBox^  disableLabelCB;
	System::Windows::Forms::CheckBox^  hardwareAccelCB;
	System::Windows::Forms::CheckBox^  animationEffectsCB;
	System::Windows::Forms::CheckBox^  stackWindowsCB;
	System::Windows::Forms::FlowLayoutPanel^	hotKeyPanel;
	System::Windows::Forms::RadioButton^		altButton;
	System::Windows::Forms::RadioButton^		ctrlButton;
	System::Windows::Forms::FlowLayoutPanel^	modifierBox;
	System::Windows::Forms::RadioButton^		shiftButton;
	System::Windows::Forms::RadioButton^		winButton;
	System::Windows::Forms::RadioButton^		nokeyButton;
	System::Windows::Forms::ComboBox^			keyCombo;
	System::Windows::Forms::TabPage^			appearanceTab;
	System::Windows::Forms::FlowLayoutPanel^	appearancePanel;
	System::Windows::Forms::TableLayoutPanel^	sliderPanel;
	System::Windows::Forms::FlowLayoutPanel^	colorPanel;
	System::Windows::Forms::HScrollBar^			immersiveThumbHeightTB;
	System::Windows::Forms::HScrollBar^			animationSpeedTB;
	System::Windows::Forms::HScrollBar^			labelFontSizeTB;
	System::Windows::Forms::HScrollBar^			thumbnailSpacingTB;
	System::Windows::Forms::Label^				labelFontSizeL;
	System::Windows::Forms::RadioButton^	ulButton;
	System::Windows::Forms::RadioButton^	urButton;
	System::Windows::Forms::RadioButton^	lrButton;
	System::Windows::Forms::Label^  animationSpeedL;
	System::Windows::Forms::Label^  thumbnailSpacingL;
	System::Windows::Forms::Label^  immersiveThumbHeightL;
	System::Windows::Forms::CheckBox^  autoColorCheck;
	System::Windows::Forms::Label^  backgroundColorL;
	System::Windows::Forms::Label^  colorDisplayLabel;
	System::Windows::Forms::Label^  colorPaddingL;
	System::Windows::Forms::TabPage^  groupsTab;
	System::Windows::Forms::FlowLayoutPanel^ groupsPanel;
	System::Windows::Forms::TableLayoutPanel^ groupTable;
	System::Windows::Forms::Label^		groupListL;
	System::Windows::Forms::ListBox^	groupListBox;
	System::Windows::Forms::Label^		groupContentsL;
	System::Windows::Forms::ListBox^	groupContentsBox;
	System::Windows::Forms::Button^		deleteAppFromGroupButton;
	System::Windows::Forms::Button^		addAppGroupButton;
	System::Windows::Forms::Button^		addAppToGroupButton;
	System::Windows::Forms::Button^		deleteAppGroupButton;
	System::Windows::Forms::FlowLayoutPanel^ addPanel;
	System::Windows::Forms::TextBox^	addAppOrGroupText;
	System::Windows::Forms::Button^		addAppOrGroupButton;
	System::Windows::Forms::Label^		addActiveAppLabel;
	System::Windows::Forms::ComboBox^	addActiveAppCombo;
	System::Windows::Forms::Button^  acceptButton;
	System::Windows::Forms::Button^  defaultsButton;
	System::Windows::Forms::Label^ copyright;
	System::Windows::Forms::Label^ settings;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>

	void InitializeComponent( void )
	{
		AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
		Load += gcnew System::EventHandler( this, &McSettingsEditor::doActivation );
		Closed += gcnew EventHandler( this, &McSettingsEditor::doDeactivation );
		copyright = (gcnew System::Windows::Forms::Label( ));
		settings = (gcnew System::Windows::Forms::Label( ));
		colorDialog = (gcnew System::Windows::Forms::ColorDialog( ));
		cancelButton = (gcnew System::Windows::Forms::Button( ));
		tabGroup = (gcnew System::Windows::Forms::TabControl( ));
		activationTab = (gcnew System::Windows::Forms::TabPage( ));
		activationPanel = (gcnew System::Windows::Forms::FlowLayoutPanel( ));
		actTickBoxPanel = (gcnew System::Windows::Forms::TableLayoutPanel( ));
		allowHotCornerCB = (gcnew System::Windows::Forms::CheckBox( ));
		customHotKeyCB = (gcnew System::Windows::Forms::CheckBox( ));
#if MC_DESKTOPS
		showAllDesktops = (gcnew System::Windows::Forms::CheckBox( ));
#endif
		useAppBar = (gcnew System::Windows::Forms::CheckBox( ));
		if (MC::getMD( ))
			multiMonitor = (gcnew System::Windows::Forms::CheckBox( ));
		disableLabelCB = (gcnew System::Windows::Forms::CheckBox( ));
			hardwareAccelCB = (gcnew System::Windows::Forms::CheckBox( ));
			animationEffectsCB = (gcnew System::Windows::Forms::CheckBox( ));
			stackWindowsCB = (gcnew System::Windows::Forms::CheckBox( ));
		hotKeyPanel = (gcnew System::Windows::Forms::FlowLayoutPanel( ));
		keyCombo = (gcnew System::Windows::Forms::ComboBox( ));
		modifierBox = (gcnew System::Windows::Forms::FlowLayoutPanel( ));
		ctrlButton = (gcnew System::Windows::Forms::RadioButton( ));
		shiftButton = (gcnew System::Windows::Forms::RadioButton( ));
		winButton = (gcnew System::Windows::Forms::RadioButton( ));
		nokeyButton = (gcnew System::Windows::Forms::RadioButton( ));
		altButton = (gcnew System::Windows::Forms::RadioButton( ));
		ulButton = (gcnew System::Windows::Forms::RadioButton( ));
		urButton = (gcnew System::Windows::Forms::RadioButton( ));
		lrButton = (gcnew System::Windows::Forms::RadioButton( ));
		appearanceTab = (gcnew System::Windows::Forms::TabPage( ));
		appearancePanel = (gcnew System::Windows::Forms::FlowLayoutPanel( ));
		sliderPanel = (gcnew System::Windows::Forms::TableLayoutPanel( ));
		colorPanel = (gcnew System::Windows::Forms::FlowLayoutPanel( ));
		colorDisplayLabel = (gcnew System::Windows::Forms::Label( ));
		colorPaddingL = (gcnew System::Windows::Forms::Label( ));
		labelFontSizeL = (gcnew System::Windows::Forms::Label( ));
		thumbnailSpacingTB = (gcnew System::Windows::Forms::HScrollBar( ));
		animationSpeedL = (gcnew System::Windows::Forms::Label( ));
		thumbnailSpacingL = (gcnew System::Windows::Forms::Label( ));
		immersiveThumbHeightL = (gcnew System::Windows::Forms::Label( ));
		immersiveThumbHeightTB = (gcnew System::Windows::Forms::HScrollBar( ));
		animationSpeedTB = (gcnew System::Windows::Forms::HScrollBar( ));
		labelFontSizeTB = (gcnew System::Windows::Forms::HScrollBar( ));
		autoColorCheck = (gcnew System::Windows::Forms::CheckBox( ));
		backgroundColorL = (gcnew System::Windows::Forms::Label( ));
		groupsTab = (gcnew System::Windows::Forms::TabPage( ));
		groupsPanel = (gcnew System::Windows::Forms::FlowLayoutPanel( ));
		groupTable = (gcnew System::Windows::Forms::TableLayoutPanel( ));
		addPanel = (gcnew System::Windows::Forms::FlowLayoutPanel( ));
		addActiveAppLabel = (gcnew System::Windows::Forms::Label( ));
		addAppToGroupButton = (gcnew System::Windows::Forms::Button( ));
		deleteAppFromGroupButton = (gcnew System::Windows::Forms::Button( ));
		groupContentsL = (gcnew System::Windows::Forms::Label( ));
		groupListL = (gcnew System::Windows::Forms::Label( ));
		groupContentsBox = (gcnew System::Windows::Forms::ListBox( ));
		addActiveAppCombo = (gcnew System::Windows::Forms::ComboBox( ));
		addAppOrGroupText = (gcnew System::Windows::Forms::TextBox( ));
		addAppOrGroupButton = (gcnew System::Windows::Forms::Button( ));
		addAppGroupButton = (gcnew System::Windows::Forms::Button( ));
		deleteAppGroupButton = (gcnew System::Windows::Forms::Button( ));
		groupListBox = (gcnew System::Windows::Forms::ListBox( ));
		acceptButton = (gcnew System::Windows::Forms::Button( ));
		defaultsButton = (gcnew System::Windows::Forms::Button( ));
		SuspendLayout( );
		activationTab->SuspendLayout( );
		activationPanel->SuspendLayout( );
		actTickBoxPanel->SuspendLayout( );
		hotKeyPanel->SuspendLayout( );
		modifierBox->SuspendLayout( );
		appearanceTab->SuspendLayout( );
		appearancePanel->SuspendLayout( );
		sliderPanel->SuspendLayout( );
		colorPanel->SuspendLayout( );
		groupsTab->SuspendLayout( );
		groupsPanel->SuspendLayout( );
		groupTable->SuspendLayout( );
		addPanel->SuspendLayout( );
		McRect *mdR = McMonitorsMgr::getMainMonitor( )->getMonitorSize( );
		dialogWidth = mdR->getWidth( );
		tabWidth = max( 600, (int) floor( dialogWidth / 1.75 ) );
		dialogHeight = max( tabWidth / 2, mdR->getHeight( ) / 3 );
		ClientSize = System::Drawing::Size( dialogWidth, dialogHeight );
		Top = 0;
		Left = 0;
		ControlBox = false;
		Controls->Add( settings );
		Controls->Add( copyright );
		Controls->Add( defaultsButton );
		Controls->Add( acceptButton );
		Controls->Add( cancelButton );
		Controls->Add( tabGroup );
		FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
		StartPosition = FormStartPosition::Manual;
		MaximizeBox = false;
		MinimizeBox = false;
		Name = L"SettingsEditor";
		ShowInTaskbar = false;
		Text = L"Emcee Settings";
		TopMost = true;
		float scale = McMonitorsMgr::getMainMonitor( )->getScale( );
		// acceptButton
		acceptButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb( _EMCEE_FORM_BG_ALPHA, 218, 42, 42 );
		acceptButton->FlatAppearance->MouseDownBackColor = acceptButton->FlatAppearance->MouseOverBackColor;
		acceptButton->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
		acceptButton->Font = (gcnew System::Drawing::Font( L"Segoe UI", 12.0F*scale, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		acceptButton->Name = L"acceptButton";
		acceptButton->AutoSize = true;
		acceptButton->TabIndex = 2;
		acceptButton->Text = L"Accept Changes";
		acceptButton->Click += gcnew System::EventHandler( this, &McSettingsEditor::acceptButton_Click );
		// defaultsButton
		defaultsButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb( _EMCEE_FORM_BG_ALPHA, 50, 143, 245 );
		defaultsButton->FlatAppearance->MouseDownBackColor = defaultsButton->FlatAppearance->MouseOverBackColor;
		defaultsButton->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
		defaultsButton->Font = (gcnew System::Drawing::Font( L"Segoe UI", 12.0F * scale, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		defaultsButton->Name = L"defaultsButton";
		defaultsButton->AutoSize = true;
		defaultsButton->TabIndex = 4;
		defaultsButton->Text = L"Undo Changes";
		defaultsButton->Click += gcnew System::EventHandler( this, &McSettingsEditor::defaultsButton_Click );
		// cancelButton
		cancelButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb( _EMCEE_FORM_BG_ALPHA, 17, 191, 47 );
		cancelButton->FlatAppearance->MouseDownBackColor = cancelButton->FlatAppearance->MouseOverBackColor;
		cancelButton->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
		cancelButton->Font = (gcnew System::Drawing::Font( L"Segoe UI", 12.0F * scale, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		cancelButton->Name = L"cancelButton";
		cancelButton->AutoSize = true;
		cancelButton->TabIndex = 3;
		cancelButton->Text = L"Cancel";
		cancelButton->Click += gcnew System::EventHandler( this, &McSettingsEditor::cancelButton_Click );
		// tabGroup
		tabHeight = dialogHeight - cancelButton->Height - 45;
		tabGroup->Appearance = TabAppearance::Normal;
		tabGroup->Controls->Add( activationTab );
		tabGroup->Controls->Add( appearanceTab );
		tabGroup->Controls->Add( groupsTab );
		tabGroup->Font = (gcnew System::Drawing::Font( L"Segoe UI", 11.25F*scale, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		tabGroup->Name = L"tabGroup";
		tabGroup->SelectedIndex = 0;
		tabGroup->Size = System::Drawing::Size( tabWidth, tabHeight );
		tabGroup->SizeMode = System::Windows::Forms::TabSizeMode::Normal;
		tabGroup->Top = 5;
		tabGroup->TabIndex = 1;
		System::Drawing::Size clientS = tabGroup->ClientSize;
		int tabw = clientS.Width;
		int tabh = clientS.Height;
		// activationTab
		activationTab->TabIndex = 0;
		activationTab->Text = L"Activation & Start-up  ";
		activationTab->BorderStyle = System::Windows::Forms::BorderStyle::None;
		activationTab->Controls->Add( activationPanel );
		activationTab->Controls->Add( urButton );
		activationTab->Controls->Add( lrButton );
		activationTab->Controls->Add( ulButton );
		activationTab->Name = L"activationTab";
		activationTab->Size = System::Drawing::Size( tabw, tabh );
		activationTab->TabIndex = 0;
		activationTab->Text = L"Activation & Start-up  ";
		// activationPanel
		activationPanel->FlowDirection = FlowDirection::TopDown;
		activationPanel->AutoSize = true;
		activationPanel->Controls->Add( actTickBoxPanel );
		activationPanel->Controls->Add( hotKeyPanel );
		activationPanel->Anchor = AnchorStyles::None;
		// activationLayout
		actTickBoxPanel->RowCount = 3;
		actTickBoxPanel->ColumnCount = 2;
		actTickBoxPanel->AutoSize = TRUE;
		actTickBoxPanel->Anchor = AnchorStyles::None;
		actTickBoxPanel->Controls->Add( useAppBar );
#if MC_DESKTOPS
		actTickBoxPanel->Controls->Add( showAllDesktops );
#endif
		actTickBoxPanel->Controls->Add( allowHotCornerCB );
		actTickBoxPanel->Controls->Add( disableLabelCB );
			actTickBoxPanel->Controls->Add( hardwareAccelCB );
			actTickBoxPanel->Controls->Add( animationEffectsCB );
			actTickBoxPanel->Controls->Add( stackWindowsCB );
		if (MC::getMD( ))
		{
			actTickBoxPanel->Controls->Add( multiMonitor );
			actTickBoxPanel->Controls->Add( customHotKeyCB );
		}
		actTickBoxPanel->Controls->Add( customHotKeyCB );
		// disableLabelCB
		disableLabelCB->AutoSize = true;
		disableLabelCB->Font = (gcnew System::Drawing::Font( L"Segoe UI", 11.75F*scale/*9.75*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		disableLabelCB->Name = L"disableLabelCB";
		disableLabelCB->TabIndex = 1;
		disableLabelCB->Text = L"Disable Window Labels";
		disableLabelCB->CheckedChanged += gcnew System::EventHandler( this, &McSettingsEditor::disableLabelCB_CheckChanged );
			// hardwareAccelCB
			hardwareAccelCB->AutoSize = true;
			hardwareAccelCB->Font = (gcnew System::Drawing::Font( L"Segoe UI", 11.75F*scale/*9.75*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0) ));
			hardwareAccelCB->Name = L"hardwareAccelCB";
			hardwareAccelCB->TabIndex = 3;
			hardwareAccelCB->Text = L"Enable Hardware Acceleration";
			hardwareAccelCB->CheckedChanged += gcnew System::EventHandler( this, &McSettingsEditor::hardwareAccelCB_CheckedChanged );

			// animationEffectsCB
			animationEffectsCB->AutoSize = true;
			animationEffectsCB->Font = hardwareAccelCB->Font;
			animationEffectsCB->Name = L"animationEffectsCB";
			animationEffectsCB->TabIndex = 24;
			animationEffectsCB->Text = L"Enable Animation Effects";
			animationEffectsCB->CheckedChanged += gcnew System::EventHandler( this, &McSettingsEditor::animationEffectsCB_CheckedChanged );
			// stackWindowsCB
			stackWindowsCB->AutoSize = true;
			stackWindowsCB->Font = (gcnew System::Drawing::Font( L"Segoe UI", 11.75F*scale/*9.75*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0) ));
			stackWindowsCB->Name = L"stackWindowsCB";
			stackWindowsCB->TabIndex = 4;
			stackWindowsCB->Text = L"Stack Windows";
			stackWindowsCB->CheckedChanged += gcnew System::EventHandler( this, &McSettingsEditor::stackWindowsCB_CheckedChanged );
		// allowHotCornerCB
		allowHotCornerCB->AutoSize = true;
		allowHotCornerCB->Font = (gcnew System::Drawing::Font( L"Segoe UI", 11.75F*scale/*9.75*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		allowHotCornerCB->Name = L"allowHotCornerCB";
		allowHotCornerCB->TabIndex = 0;
		allowHotCornerCB->Text = L"Allow Hot Corner Activation";
		allowHotCornerCB->CheckedChanged += gcnew System::EventHandler( this, &McSettingsEditor::allowHotCornerCB_CheckedChanged );
		// customHotKeyCB
		customHotKeyCB->AutoSize = true;
		customHotKeyCB->Font = (gcnew System::Drawing::Font( L"Segoe UI", 11.75F*scale/*9.75*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		customHotKeyCB->Name = L"customHotKeyCB";
		customHotKeyCB->TabIndex = 2;
		customHotKeyCB->Text = L"Customize Activation Hot Key";
		customHotKeyCB->CheckedChanged += gcnew System::EventHandler( this, &McSettingsEditor::customHotKeyCB_CheckedChanged );
		// showAllDesktops
#if MC_DESKTOPS
		showAllDesktops->AutoSize = true;
		showAllDesktops->Font = (gcnew System::Drawing::Font( L"Segoe UI", 11.75F*scale/*9.75*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		showAllDesktops->Name = L"showAllDesktops";
		showAllDesktops->TabIndex = 2;
		showAllDesktops->Text = L"Show Windows From All Desktops";
		showAllDesktops->CheckedChanged += gcnew System::EventHandler( this, &McSettingsEditor::showAllDesktops_CheckedChanged );
		if (MC::inTabletMode( )) showAllDesktops->Enabled = false;
#endif
		// useAppBar
		useAppBar->AutoSize = true;
		useAppBar->Font = (gcnew System::Drawing::Font( L"Segoe UI", 11.75F*scale/*9.75*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		useAppBar->Name = L"useAppBar";
		useAppBar->TabIndex = 2;
		useAppBar->Text = L"Full Screen \"Modern\" Apps on App Bar";
		useAppBar->CheckedChanged += gcnew System::EventHandler( this, &McSettingsEditor::useAppBar_CheckedChanged );
		if (MC::inTabletMode( )) useAppBar->Enabled = false;
		if (MC::getMD( ))
		{
			multiMonitor->AutoSize = true;
			multiMonitor->Font = (gcnew System::Drawing::Font( L"Segoe UI", 11.75F*scale/*9.75*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0) ));
			multiMonitor->Name = L"multiMonitor";
			multiMonitor->TabIndex = 2;
			multiMonitor->Text = L"Multi-Monitor Layout";
			multiMonitor->CheckedChanged += gcnew System::EventHandler( this, &McSettingsEditor::multiMonitor_CheckedChanged );
		}
		// hotKeyPanel
		hotKeyPanel->Anchor = AnchorStyles::None;
		hotKeyPanel->FlowDirection = FlowDirection::LeftToRight;
		hotKeyPanel->Font = (gcnew System::Drawing::Font( L"Segoe UI", 11.75F*scale/*9.75*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		hotKeyPanel->Name = L"hotKeyPanel";
		hotKeyPanel->AutoSize = true;
		hotKeyPanel->TabIndex = 0;
		hotKeyPanel->Controls->Add( modifierBox );
		hotKeyPanel->Controls->Add( keyCombo );
		// keyCombo
		keyCombo->FormattingEnabled = true;
		keyCombo->Items->AddRange( gcnew cli::array< System::Object^  >( 92 ) {
			"BkSpace", "Tab", "Enter", "Space", "PgUp",
				"PgDown", "End", "Home", "L Arrow", "Up Arrow",
				"R Arrow", "Dn Arrow", "Select", "Print", "Execute",
				"Insert", "Del", "Help", "0", "1",
				"2", "3", "4", "5", "6",
				"7", "8", "9", "A", "B",
				"C", "D", "E", "F", "G",
				"H", "I", "J", "K", "L",
				"M", "N", "O", "P", "Q",
				"R", "S", "T", "U", "V",
				"W", "X", "Y", "Z",
				"`~", "[{", "]}", "'\"", "\\|",
				"/?", ";:", "+=", "-_", ".>", ",<",
				"NumPad 0",
				"NumPad 1", "NumPad 2", "NumPad 3", "NumPad 4", "NumPad 5",
				"NumPad 6", "NumPad 7", "NumPad 8", "NumPad 9",
				"NumPad /", "NumPad *", "NumPad -", "NumPad +", "NumPad .",
				"F1", "F2", "F3", "F4", "F5",
				"F6", "F7", "F8", "F9", "F10",
				"F11", "F12"
		} );
		keyCombo->Name = L"keyCombo";
		keyCombo->AutoSize = true;
		keyCombo->TabIndex = 7;
		keyCombo->Font = (gcnew System::Drawing::Font( L"Segoe UI", 8.75F*scale/*7.75F*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		keyCombo->SelectedValueChanged += gcnew System::EventHandler( this, &McSettingsEditor::hotKey_change );
		keyCombo->DropDownStyle = ComboBoxStyle::DropDownList;
		// modifierBox
		modifierBox->FlowDirection = FlowDirection::LeftToRight;
		modifierBox->Controls->Add( ctrlButton );
		modifierBox->Controls->Add( shiftButton );
		modifierBox->Controls->Add( altButton );
		modifierBox->Controls->Add( winButton );
		modifierBox->Controls->Add( nokeyButton );
		modifierBox->Name = L"modifierBox";
		modifierBox->AutoSize = true;
		modifierBox->TabIndex = 4;
		modifierBox->TabStop = false;
		modifierBox->Text = L"";
		modifierBox->ForeColor = System::Drawing::Color::White;
		// ctrlButton
		ctrlButton->AutoSize = true;
		ctrlButton->Name = L"ctrlButton";
		ctrlButton->TabIndex = 0;
		ctrlButton->TabStop = true;
		ctrlButton->Text = L"Ctrl";
		ctrlButton->UseVisualStyleBackColor = true;
		ctrlButton->CheckedChanged += gcnew System::EventHandler( this, &McSettingsEditor::hotKey_change );
		// altButton
		altButton->AutoSize = true;
		altButton->Name = L"altButton";
		altButton->TabIndex = 2;
		altButton->TabStop = true;
		altButton->Text = L"Alt";
		altButton->UseVisualStyleBackColor = true;
		altButton->CheckedChanged += gcnew System::EventHandler( this, &McSettingsEditor::hotKey_change );
		// noKeyButton
		nokeyButton->AutoSize = true;
		nokeyButton->Name = L"nokeyButton";
		nokeyButton->TabIndex = 2;
		nokeyButton->TabStop = true;
		nokeyButton->Text = L"No Mod";
		nokeyButton->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
		nokeyButton->UseVisualStyleBackColor = true;
		nokeyButton->CheckedChanged += gcnew System::EventHandler( this, &McSettingsEditor::hotKey_change );
		// shiftButton
		shiftButton->AutoSize = true;
		shiftButton->Name = L"shiftButton";
		shiftButton->TabIndex = 3;
		shiftButton->TabStop = true;
		shiftButton->Text = L"Shift";
		shiftButton->UseVisualStyleBackColor = true;
		shiftButton->CheckedChanged += gcnew System::EventHandler( this, &McSettingsEditor::hotKey_change );
		// winButton
		winButton->AutoSize = true;
		winButton->Name = L"winButton";
		winButton->TabIndex = 2;
		winButton->TabStop = true;
		winButton->Text = L"Win";
		winButton->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
		winButton->UseVisualStyleBackColor = true;
		winButton->CheckedChanged += gcnew System::EventHandler( this, &McSettingsEditor::hotKey_change );
		// ulButton
		ulButton->AutoSize = true;
		ulButton->Font = (gcnew System::Drawing::Font( L"Segoe UI", 9.75F*scale/*7.75F*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		ulButton->Name = L"ulButton";
		ulButton->TabIndex = 3;
		ulButton->TabStop = true;
		ulButton->Text = L"Upper Left";
		ulButton->UseVisualStyleBackColor = true;
		ulButton->TextAlign = System::Drawing::ContentAlignment::MiddleLeft;
		ulButton->CheckedChanged += gcnew System::EventHandler( this, &McSettingsEditor::hotCorner_change );
		// urButton
		urButton->AutoSize = true;
		urButton->Font = (gcnew System::Drawing::Font( L"Segoe UI", 9.75F*scale/*7.75F*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		urButton->Name = L"urButton";
		urButton->TabIndex = 3;
		urButton->TabStop = true;
		urButton->Text = L"Upper Right";
		urButton->UseVisualStyleBackColor = true;
		urButton->CheckAlign = ContentAlignment::MiddleRight;
		urButton->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
		urButton->CheckedChanged += gcnew System::EventHandler( this, &McSettingsEditor::hotCorner_change );
		// lrButton
		lrButton->AutoSize = true;
		lrButton->Font = (gcnew System::Drawing::Font( L"Segoe UI", 9.75F*scale/*7.75F*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		lrButton->Name = L"lrButton";
		lrButton->TabIndex = 3;
		lrButton->TabStop = true;
		lrButton->Text = L"Lower Right";
		lrButton->CheckAlign = ContentAlignment::MiddleRight;
		lrButton->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
		lrButton->CheckedChanged += gcnew System::EventHandler( this, &McSettingsEditor::hotCorner_change );
		modifierBox->ResumeLayout( false );
		modifierBox->PerformLayout( );
		hotKeyPanel->ResumeLayout( false );
		hotKeyPanel->PerformLayout( );
		actTickBoxPanel->ResumeLayout( false );
		actTickBoxPanel->PerformLayout( );
		activationPanel->ResumeLayout( false );
		activationPanel->PerformLayout( );
		activationTab->ResumeLayout( false );
		activationTab->PerformLayout( );
		activationPanel->Left = (activationTab->DisplayRectangle.Width - activationPanel->Width) / 2;
		activationPanel->Top = (activationTab->DisplayRectangle.Height - activationPanel->Height) / 2;
		appearanceTab->TabIndex = 1;
		appearanceTab->Text = L"  Appearance  ";
		// appearanceTab///////////////////////////////////////////////////////////////////////
		appearanceTab->BorderStyle = System::Windows::Forms::BorderStyle::None;
		appearanceTab->Controls->Add( appearancePanel );
		appearanceTab->Name = L"appearanceTab";
		appearanceTab->Size = System::Drawing::Size( tabw, tabh );
		// appearancePanel
		appearancePanel->AutoSize = true;
		appearancePanel->FlowDirection = FlowDirection::TopDown;
		appearancePanel->BackColor = System::Drawing::Color::Transparent;
		appearancePanel->Controls->Add( sliderPanel );
		appearancePanel->Controls->Add( colorPanel );
		appearancePanel->Anchor = AnchorStyles::None;
		// sliderPanel
		sliderPanel->AutoSize = true;
		sliderPanel->RowCount = 4;
		sliderPanel->ColumnCount = 2;
		sliderPanel->Anchor = AnchorStyles::None;
		sliderPanel->Controls->Add( labelFontSizeTB );
		sliderPanel->Controls->Add( thumbnailSpacingTB );
		sliderPanel->Controls->Add( labelFontSizeL );
		sliderPanel->Controls->Add( thumbnailSpacingL );
		sliderPanel->Controls->Add( immersiveThumbHeightTB );
		sliderPanel->Controls->Add( animationSpeedTB );
		sliderPanel->Controls->Add( immersiveThumbHeightL );
		sliderPanel->Controls->Add( animationSpeedL );
		// labelFontSizeL
		labelFontSizeL->AutoSize = true;
		labelFontSizeL->Font = (gcnew System::Drawing::Font( L"Segoe UI", 11.75F*scale/*9.75*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		labelFontSizeL->Name = L"labelFontSizeL";
		labelFontSizeL->TabIndex = 8;
		labelFontSizeL->Text = L"Small - Label Font Size - Large";
		labelFontSizeL->TextAlign = System::Drawing::ContentAlignment::TopCenter;
		labelFontSizeL->Anchor = AnchorStyles::Right | AnchorStyles::Left;
		// animationSpeedL
		animationSpeedL->AutoSize = true;
		animationSpeedL->Font = (gcnew System::Drawing::Font( L"Segoe UI", 11.75F*scale/*9.75*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		animationSpeedL->Name = L"animationSpeedL";
		animationSpeedL->TabIndex = 6;
		animationSpeedL->TextAlign = System::Drawing::ContentAlignment::TopCenter;
		animationSpeedL->Anchor = AnchorStyles::Right | AnchorStyles::Left;
		animationSpeedL->Text = L"Slow - Animation Speed - Fast";
		// thumbnailSpacingL
		thumbnailSpacingL->AutoSize = true;
		thumbnailSpacingL->Font = (gcnew System::Drawing::Font( L"Segoe UI", 11.75F*scale/*9.75*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		thumbnailSpacingL->Name = L"thumbnailSpacingL";
		thumbnailSpacingL->TabIndex = 5;
		thumbnailSpacingL->TextAlign = System::Drawing::ContentAlignment::TopCenter;
		thumbnailSpacingL->Anchor = AnchorStyles::Right | AnchorStyles::Left;
		thumbnailSpacingL->Text = L"Low - Thumbnail Spacing - High";
		// immersiveThumbHeightL
		immersiveThumbHeightL->AutoSize = true;
		immersiveThumbHeightL->Font = (gcnew System::Drawing::Font( L"Segoe UI", 11.75F*scale/*9.75*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		immersiveThumbHeightL->Name = L"immersiveThumbHeightL";
		immersiveThumbHeightL->TabIndex = 7;
		immersiveThumbHeightL->TextAlign = System::Drawing::ContentAlignment::TopCenter;
		immersiveThumbHeightL->Anchor = AnchorStyles::Right | AnchorStyles::Left;
		immersiveThumbHeightL->Text = L"Small - App Bar Thumbnail Size - Large";
		// immersiveThumbHeightTB
		immersiveThumbHeightTB->Maximum = 48;
		immersiveThumbHeightTB->Minimum = 4;
		immersiveThumbHeightTB->Name = L"immersiveThumbHeightTB";
		immersiveThumbHeightTB->TabIndex = 13;
		immersiveThumbHeightTB->Value = 16;
		immersiveThumbHeightTB->SmallChange = 1;
		immersiveThumbHeightTB->LargeChange = 1;
		immersiveThumbHeightTB->Margin = System::Windows::Forms::Padding( 10 );
		immersiveThumbHeightTB->Anchor = AnchorStyles::Right | AnchorStyles::Left;
		immersiveThumbHeightTB->ValueChanged += gcnew System::EventHandler( this, &McSettingsEditor::immersiveThumbHeightTB_ValueChanged );
		// animationSpeedTB
		animationSpeedTB->Maximum = 9;
		animationSpeedTB->Minimum = 1;
		animationSpeedTB->Name = L"animationSpeedTB";
		animationSpeedTB->TabIndex = 12;
		animationSpeedTB->Value = 5;
		animationSpeedTB->SmallChange = 1;
		animationSpeedTB->LargeChange = 1;
		animationSpeedTB->Margin = System::Windows::Forms::Padding( 10 );
		animationSpeedTB->Anchor = AnchorStyles::Right | AnchorStyles::Left;
		animationSpeedTB->ValueChanged += gcnew System::EventHandler( this, &McSettingsEditor::animationSpeedTB_ValueChanged );
		// labelFontSizeTB
		labelFontSizeTB->Maximum = 42;
		labelFontSizeTB->Minimum = 6;
		labelFontSizeTB->Name = L"labelFontSizeTB";
		labelFontSizeTB->TabIndex = 11;
		labelFontSizeTB->Value = 12;
		labelFontSizeTB->SmallChange = 2;
		labelFontSizeTB->LargeChange = 2;
		labelFontSizeTB->Margin = System::Windows::Forms::Padding( 10 );
		labelFontSizeTB->Anchor = AnchorStyles::Right | AnchorStyles::Left;
		labelFontSizeTB->ValueChanged += gcnew System::EventHandler( this, &McSettingsEditor::labelFontSizeTB_ValueChanged );
		// thumbnailSpacingTB
		thumbnailSpacingTB->Maximum = 8;
		thumbnailSpacingTB->Minimum = 2;
		thumbnailSpacingTB->Name = L"thumbnailSpacingTB";
		thumbnailSpacingTB->TabIndex = 16;
		thumbnailSpacingTB->Value = 3;
		thumbnailSpacingTB->Anchor = AnchorStyles::Right | AnchorStyles::Left;
		thumbnailSpacingTB->SmallChange = 1;
		thumbnailSpacingTB->LargeChange = 1;
		thumbnailSpacingTB->Margin = System::Windows::Forms::Padding( 10 );
		thumbnailSpacingTB->ValueChanged += gcnew System::EventHandler( this, &McSettingsEditor::thumbnailSpacingTB_ValueChanged );
		// colorPanel
		colorPanel->AutoSize = true;
		colorPanel->FlowDirection = FlowDirection::LeftToRight;
		colorPanel->Anchor = AnchorStyles::None;
		colorPanel->Controls->Add( backgroundColorL );
		colorPanel->Controls->Add( colorDisplayLabel );
		colorPanel->Controls->Add( colorPaddingL );
		colorPanel->Controls->Add( autoColorCheck );
		// colorDisplayLabel
		colorDisplayLabel->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
		colorDisplayLabel->Name = L"colorDisplayLabel";
		colorDisplayLabel->Height = 50;
		colorDisplayLabel->TabIndex = 23;
		colorDisplayLabel->Anchor = AnchorStyles::None;
		colorDisplayLabel->Click += gcnew System::EventHandler( this, &McSettingsEditor::colorPick_Click );
		// colorPadding
		colorPaddingL->Width = 20;
		// autoColorCheck
		autoColorCheck->AutoSize = true;
		backgroundColorL->Anchor = AnchorStyles::None;
		autoColorCheck->Font = (gcnew System::Drawing::Font( L"Segoe UI", 12.75F*scale/*9.75*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		autoColorCheck->Name = L"autoColorCheckCB";
		autoColorCheck->TabIndex = 15;
		autoColorCheck->Text = L" Select Color\n Automatically";
		autoColorCheck->CheckedChanged += gcnew System::EventHandler( this, &McSettingsEditor::autoColorCheckCB_CheckedChanged );
		// backgroundColorL
		backgroundColorL->AutoSize = true;
		backgroundColorL->Font = (gcnew System::Drawing::Font( L"Segoe UI Semibold", 12.0F * scale, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		backgroundColorL->Name = L"backgroundColorL";
		backgroundColorL->TabIndex = 9;
		backgroundColorL->Text = L"Desktop Background Color";
		backgroundColorL->Anchor = AnchorStyles::None;
		backgroundColorL->Padding = System::Windows::Forms::Padding( 10 );
		autoColorCheck->Height = colorDisplayLabel->Height;
		// colorDialog// 
		colorDialog->AnyColor = true;
		colorPanel->ResumeLayout( false );
		colorPanel->PerformLayout( );
		sliderPanel->ResumeLayout( false );
		sliderPanel->PerformLayout( );
		appearancePanel->ResumeLayout( false );
		appearancePanel->PerformLayout( );
		appearanceTab->ResumeLayout( false );
		appearanceTab->PerformLayout( );
		thumbnailSpacingTB->Width = immersiveThumbHeightTB->Width;
		animationSpeedTB->Width = immersiveThumbHeightTB->Width;
		appearancePanel->Left = (appearanceTab->DisplayRectangle.Width - appearancePanel->Width) / 2;
		appearancePanel->Top = (appearanceTab->DisplayRectangle.Height - appearancePanel->Height) / 2;
		// groupsTab// 
		groupsTab->TabIndex = 2;
		groupsTab->Text = L"  App Groups & Exclusions  ";
		groupsTab->BorderStyle = System::Windows::Forms::BorderStyle::None;
		groupsTab->Controls->Add( groupsPanel );
		groupsTab->Name = L"groupsTab";
		groupsTab->Size = System::Drawing::Size( tabw, tabh );
		groupsTab->Enter += gcnew System::EventHandler( this, &McSettingsEditor::groupsTab_Enter );
		// groupsPanel
		groupsPanel->AutoSize = true;
		groupsPanel->FlowDirection = FlowDirection::LeftToRight;
		groupsPanel->BackColor = System::Drawing::Color::Transparent;
		groupsPanel->Controls->Add( groupTable );
		groupsPanel->Controls->Add( addPanel );
		groupsPanel->Anchor = AnchorStyles::None;
		// groupTable
		groupTable->AutoSize = true;
		groupTable->RowCount = 3;
		groupTable->ColumnCount = 4;
		groupTable->Controls->Add( groupListL );
		groupTable->SetColumnSpan( groupListL, 2 );
		groupTable->Controls->Add( groupContentsL );
		groupTable->SetColumnSpan( groupContentsL, 2 );
		groupTable->Controls->Add( groupListBox );
		groupTable->SetColumnSpan( groupListBox, 2 );
		groupTable->Controls->Add( groupContentsBox );
		groupTable->SetColumnSpan( groupContentsBox, 2 );
		groupTable->Controls->Add( deleteAppGroupButton );
		groupTable->Controls->Add( addAppGroupButton );
		groupTable->Controls->Add( deleteAppFromGroupButton );
		groupTable->Controls->Add( addAppToGroupButton );
		// groupListL
		groupListL->AutoSize = true;
		groupListL->Name = L"groupListL";
		groupListL->TabIndex = 8;
		groupListL->Text = L"App Groups";
		groupListL->Anchor = AnchorStyles::Left | AnchorStyles::Right;
		groupListL->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
		groupListL->Font = (gcnew System::Drawing::Font( L"Segoe UI semibold", 12 * scale, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		// groupContentsL
		groupContentsL->AutoSize = true;
		groupContentsL->Name = L"groupContentsL";
		groupContentsL->TabIndex = 9;
		groupContentsL->Text = L"Group Contents";
		groupContentsL->Anchor = AnchorStyles::Left | AnchorStyles::Right;
		groupContentsL->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
		groupContentsL->Font = (gcnew System::Drawing::Font( L"Segoe UI semibold", 12 * scale, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		// groupListBox
		groupListBox->AutoSize = true;
		groupListBox->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
		groupListBox->Font = (gcnew System::Drawing::Font( L"Segoe UI", 9.00F*scale, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		groupListBox->FormattingEnabled = false;
		groupListBox->Name = L"groupListBox";
		groupListBox->AutoSize = false;
		groupListBox->TabIndex = 0;
		groupListBox->Anchor = AnchorStyles::Left | AnchorStyles::Right;
		groupListBox->SelectionMode = SelectionMode::One;
		groupListBox->SelectedValueChanged += gcnew EventHandler( this, &McSettingsEditor::groupListBox_SelectedValueChanged );
		// groupContentsBox
		groupContentsBox->AutoSize = true;
		groupContentsBox->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
		groupContentsBox->Font = (gcnew System::Drawing::Font( L"Segoe UI", 9.00F*scale, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		groupContentsBox->FormattingEnabled = false;
		groupContentsBox->Name = L"groupContentsBox";
		groupContentsBox->RightToLeft = System::Windows::Forms::RightToLeft::No;
		groupContentsBox->AutoSize = false;
		groupContentsBox->TabIndex = 7;
		groupContentsBox->Anchor = AnchorStyles::Left | AnchorStyles::Right;
		groupContentsBox->SelectedValueChanged += gcnew EventHandler( this, &McSettingsEditor::groupContentsBox_SelectedValueChanged );
		groupListBox->Width = groupContentsBox->Width;
		// deleteAppGroupButton
		deleteAppGroupButton->AutoSize = true;
		deleteAppGroupButton->FlatAppearance->BorderSize = 1;
		deleteAppGroupButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb( _EMCEE_FORM_BG_ALPHA, 218, 42, 42 );
		deleteAppGroupButton->FlatAppearance->MouseDownBackColor = deleteAppGroupButton->FlatAppearance->MouseOverBackColor;
		deleteAppGroupButton->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
		deleteAppGroupButton->Font = (gcnew System::Drawing::Font( L"Segoe UI Bold", 11 * scale/*9*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		deleteAppGroupButton->Name = L"deleteAppGroupButton";
		deleteAppGroupButton->TabIndex = 2;
		deleteAppGroupButton->Text = L"Delete\nApp\nGroup";
		deleteAppGroupButton->Anchor = AnchorStyles::Left | AnchorStyles::Right | AnchorStyles::Top | AnchorStyles::Bottom;
		deleteAppGroupButton->Click += gcnew System::EventHandler( this, &McSettingsEditor::deleteAppGroupButton_Click );
		// addAppGroupButton
		addAppGroupButton->AutoSize = true;
		addAppGroupButton->FlatAppearance->BorderSize = 1;
		addAppGroupButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb( _EMCEE_FORM_BG_ALPHA, 17, 191, 47 );
		addAppGroupButton->FlatAppearance->MouseDownBackColor = addAppGroupButton->FlatAppearance->MouseOverBackColor;
		addAppGroupButton->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
		addAppGroupButton->Font = (gcnew System::Drawing::Font( L"Segoe UI Bold", 11.0F*scale/*9*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		addAppGroupButton->Name = L"addAppGroupButton";
		addAppGroupButton->TabIndex = 3;
		addAppGroupButton->Text = L"Add App\nGroup";
		addAppGroupButton->Anchor = AnchorStyles::Left | AnchorStyles::Right | AnchorStyles::Top | AnchorStyles::Bottom;
		addAppGroupButton->Click += gcnew System::EventHandler( this, &McSettingsEditor::addAppGroupButton_Click );
		// deleteAppFromGroupButton
		deleteAppFromGroupButton->AutoSize = true;
		deleteAppFromGroupButton->FlatAppearance->BorderSize = 1;
		deleteAppFromGroupButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb( _EMCEE_FORM_BG_ALPHA, 218, 42, 42 );
		deleteAppFromGroupButton->FlatAppearance->MouseDownBackColor = deleteAppFromGroupButton->FlatAppearance->MouseOverBackColor;
		deleteAppFromGroupButton->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
		deleteAppFromGroupButton->Font = (gcnew System::Drawing::Font( L"Segoe UI Bold", 11 * scale/*9*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		deleteAppFromGroupButton->Name = L"deleteAppFromGroupButton";
		deleteAppFromGroupButton->TabIndex = 11;
		deleteAppFromGroupButton->Text = L"Remove\nApp\nFrom Group";
		deleteAppFromGroupButton->Anchor = AnchorStyles::Left | AnchorStyles::Right | AnchorStyles::Top | AnchorStyles::Bottom;
		deleteAppFromGroupButton->Click += gcnew System::EventHandler( this, &McSettingsEditor::deleteAppFromGroup_Click );
		// addAppToGroupButton
		addAppToGroupButton->AutoSize = true;
		addAppToGroupButton->FlatAppearance->BorderSize = 1;
		addAppToGroupButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb( _EMCEE_FORM_BG_ALPHA, 17, 191, 47 );
		addAppToGroupButton->FlatAppearance->MouseDownBackColor = addAppToGroupButton->FlatAppearance->MouseOverBackColor;
		addAppToGroupButton->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
		addAppToGroupButton->Font = (gcnew System::Drawing::Font( L"Segoe UI Bold", 11 * scale/*9*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		addAppToGroupButton->Name = L"addAppToGroupButton";
		addAppToGroupButton->TabIndex = 12;
		addAppToGroupButton->Text = L"Add App\nto Group";
		addAppToGroupButton->Anchor = AnchorStyles::Left | AnchorStyles::Right | AnchorStyles::Top | AnchorStyles::Bottom;
		addAppToGroupButton->Click += gcnew System::EventHandler( this, &McSettingsEditor::addAppToGroup_Click );
		// addPanel
		addPanel->BackColor = Color::Transparent;
		addPanel->AutoSize = true;
		addPanel->FlowDirection = FlowDirection::TopDown;
		//addPanel->Padding = System::Windows::Forms::Padding(20);
		addPanel->Controls->Add( addAppOrGroupText );
		addPanel->Controls->Add( addAppOrGroupButton );
		addPanel->Controls->Add( addActiveAppLabel );
		addPanel->Controls->Add( addActiveAppCombo );
		// addActiveAppLabel// 
		addActiveAppLabel->AutoSize = true;
		addActiveAppLabel->Font = (gcnew System::Drawing::Font( L"Segoe UI", 11.0F*scale/*9*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		addActiveAppLabel->Name = L"addActiveAppLabel";
		addActiveAppLabel->TabIndex = 14;
		addActiveAppLabel->Text = L"or Select Active\nApp Below";
		addActiveAppLabel->Anchor = AnchorStyles::Left | AnchorStyles::Right;
		addActiveAppLabel->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
		addActiveAppLabel->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
		// addActiveAppCombo
		addActiveAppCombo->AutoSize = true;
		addActiveAppCombo->Anchor = AnchorStyles::Left | AnchorStyles::Right;
		addActiveAppCombo->Font = (gcnew System::Drawing::Font( L"Segoe UI", 9.00F*scale, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		addActiveAppCombo->FormattingEnabled = false;
		addActiveAppCombo->Name = L"addActiveAppCombo";
		addActiveAppCombo->TabIndex = 5;
		addActiveAppCombo->Text = L"Active Apps";
		addActiveAppCombo->SelectedValueChanged += gcnew System::EventHandler( this, &McSettingsEditor::addActiveAppCombo_SelectedValueChanged );
		// addAppOrGroupText
		addAppOrGroupText->AutoSize = true;
		addAppOrGroupText->Anchor = AnchorStyles::Left | AnchorStyles::Right;
		addAppOrGroupText->Font = (gcnew System::Drawing::Font( L"Segoe UI", 11.75F*scale/*9.75*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		addAppOrGroupText->Cursor = System::Windows::Forms::Cursors::IBeam;
		addAppOrGroupText->Name = L"addAppOrGroupText";
		addAppOrGroupText->Size = System::Drawing::Size( 128, 27 );
		addAppOrGroupText->TabIndex = 4;
		// addAppOrGroupButton
		addAppOrGroupButton->AutoSize = true;
		addAppOrGroupButton->FlatAppearance->BorderSize = 1;
		addAppOrGroupButton->FlatAppearance->MouseOverBackColor = System::Drawing::Color::FromArgb( _EMCEE_FORM_BG_ALPHA, 17, 191, 47 );
		addAppOrGroupButton->FlatAppearance->MouseDownBackColor = addAppOrGroupButton->FlatAppearance->MouseOverBackColor;
		addAppOrGroupButton->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
		addAppOrGroupButton->Font = (gcnew System::Drawing::Font( L"Segoe UI", 11.0F * scale/*9*/, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		//addAppOrGroupButton->Location = System::Drawing::Point(374, 72);
		addAppOrGroupButton->Name = L"addAppOrGroupButton";
		addAppOrGroupButton->Size = System::Drawing::Size( 128, 55 );
		addAppOrGroupButton->TabIndex = 15;
		addAppOrGroupButton->Text = L"Type Group Name\nAbove and\nThen Click Here";
		addAppOrGroupButton->Click += gcnew System::EventHandler( this, &McSettingsEditor::addAppOrGroupButton_Click );
		addPanel->ResumeLayout( false );
		addPanel->PerformLayout( );
		groupTable->ResumeLayout( false );
		groupTable->PerformLayout( );
		groupsPanel->ResumeLayout( false );
		groupsPanel->PerformLayout( );
		groupsTab->ResumeLayout( false );
		groupsTab->PerformLayout( );
		groupListBox->Height += groupsTab->ClientRectangle.Height - groupsPanel->Height - 10 - _TAB_PADDING;
		groupContentsBox->Height = groupListBox->Height;
		groupsPanel->Left = (groupsTab->DisplayRectangle.Width - groupsPanel->Width) / 2;
		groupsPanel->Top = (groupsTab->DisplayRectangle.Height - groupsPanel->Height) / 2;
		copyright->BorderStyle = System::Windows::Forms::BorderStyle::None;
		copyright->BackColor = Color::Transparent;
		copyright->Font = (gcnew
			System::Drawing::Font(
			L"Segoe UI", 9.75F*scale,
			System::Drawing::FontStyle::Regular,
			System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		copyright->Name = L"copyright";
		copyright->AutoSize = true;
		copyright->TabStop = false;
		char vbuff[1000];
		char bbuff[100] = { 0 };
		sprintf_s( bbuff, "%s Mode\r\n", MC::inTabletMode( ) ? "Tablet" : "Desktop"  );
		sprintf_s( vbuff, 1000,
			"%s\r\n%s\r\n"
			"Emcee Desktop Organizer\r\n%s"
			"\r\nCopyright  2012 - 2018\r\nEmcee App Software\r\nAll Rights Reserved",
			MC::getComputerName(),bbuff,_EMCEE_VERSION);
		copyright->Text = gcnew String( vbuff );
		settings->AutoSize = true;
		settings->Font = (gcnew System::Drawing::Font( L"Segoe UI Light", 18.75F*scale,
			System::Drawing::FontStyle::Bold,
			System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0) ));
		settings->Name = L"settings";
		settings->Text = L"Settings";
		tabGroup->ResumeLayout( false );
		tabGroup->PerformLayout( );
		ResumeLayout( false );
		PerformLayout( );
		tabHeight = dialogHeight - cancelButton->Height - 30;
		tabGroup->Size = System::Drawing::Size( tabWidth, tabHeight );
	}

#pragma endregion

private:

	System::Void thumbnailSpacingTB_ValueChanged(System::Object^  sender, System::EventArgs^  e);
	System::Void immersiveThumbHeightTB_ValueChanged(System::Object^  sender, System::EventArgs^  e);
	System::Void animationSpeedTB_ValueChanged(System::Object^  sender, System::EventArgs^  e);
	System::Void labelFontSizeTB_ValueChanged(System::Object^  sender, System::EventArgs^  e);
	
	System::Void customHotKeyCB_CheckedChanged(System::Object^  sender, System::EventArgs^  e);
#if MC_DESKTOPS
	System::Void showAllDesktops_CheckedChanged( System::Object^ sender, System::EventArgs^ e );
#endif
	System::Void useAppBar_CheckedChanged( System::Object^ sender, System::EventArgs^ e );
	System::Void multiMonitor_CheckedChanged( System::Object^ sender, System::EventArgs^ e );
	System::Void allowHotCornerCB_CheckedChanged(System::Object^  sender, System::EventArgs^  e);
	System::Void disableLabelCB_CheckChanged(System::Object^  sender, System::EventArgs^  e);
	System::Void hardwareAccelCB_CheckedChanged(System::Object^  sender, System::EventArgs^  e);
	System::Void animationEffectsCB_CheckedChanged(System::Object^  sender, System::EventArgs^  e);
	System::Void stackWindowsCB_CheckedChanged(System::Object^  sender, System::EventArgs^  e);

	System::Void autoColorCheckCB_CheckedChanged( System::Object^  sender, System::EventArgs^  e );
	System::Void colorPick_Click( System::Object^  sender, System::EventArgs^  e );

	System::Void cancelButton_Click(System::Object^  sender, System::EventArgs^  e);
	System::Void acceptButton_Click(System::Object^ sender, System::EventArgs^ e );
	System::Void defaultsButton_Click(System::Object^ sender, System::EventArgs^ e );
	System::Void groupListBox_SelectedValueChanged(System::Object^ sender, System::EventArgs^ e );
	System::Void groupContentsBox_SelectedValueChanged(System::Object^ sender, System::EventArgs^ e );
	System::Void addAppGroupButton_Click(System::Object^  sender, System::EventArgs^  e);
	System::Void deleteAppGroupButton_Click(System::Object^  sender, System::EventArgs^  e);
	System::Void addAppToGroup_Click(System::Object^ sender, System::EventArgs^ e );
	System::Void deleteAppFromGroup_Click(System::Object^ sender, System::EventArgs^ e );
	System::Void addAppOrGroupButton_Click(System::Object^ sender, System::EventArgs^ e );
	System::Void addActiveAppCombo_SelectedValueChanged(System::Object^ sender, System::EventArgs^ e );
	System::Void groupsTab_Enter(System::Object^ sender, System::EventArgs^ e );

	System::Void hotKey_change(System::Object^  sender, System::EventArgs^  e);
	System::Void hotCorner_change(System::Object^ sender, System::EventArgs^ e);

	System::Void doActivation( System::Object^ sender, System::EventArgs^ e );
	System::Void doDeactivation( System::Object^ sender, System::EventArgs^ e );;


	//////////////////////////////////////////////////////////////////////////////////
	////////////////  HELPER FUNCTIONS PUBLIC AND PRIVATE ////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////

	public:

	BOOL doActivate( McPropsMgr *, list<string> * );	// Activate using props.  TRUE means props changed

	private:

	BOOL haveActivated;
	BOOL _ignore;

	int oldImmersiveValue;

	McPropData *data;
	McPropData *backupData;

	void clearAppAndGroupEntry();
	void displayData();

	void addAppGroup( String ^ );
	void removeAppGroup( String ^ );
	void addAppToGroup( String ^, String^ );
	void removeAppFromGroup( String ^, String^ );

	void removeAppFromList( char *, list<string>* );


};

