/*****************************************************************************
 * PokerTraining - THNL training software, based on the PokerTH GUI          *
 * Copyright (C) 2013 Marc Ennaji                                            *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU Affero General Public License as            *
 * published by the Free Software Foundation, either version 3 of the        *
 * License, or (at your option) any later version.                           *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU Affero General Public License for more details.                       *
 *                                                                           *
 * You should have received a copy of the GNU Affero General Public License  *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/
#include "settingsdialogimpl.h"
#include <qt/settingsdialog/selectavatardialog/myavatarlistitem.h>
#include <qt/settingsdialog/mystylelistitem.h>
#include <qt/styles/gametablestylereader.h>
#include <qt/styles/carddeckstylereader.h>
#include <qt/mymessagebox/mymessagebox.h>

#include <configfile.h>
#include <qt/gametable/log/guilog.h>
//#include <net/socket_startup.h>
#include <QSet>
#include <QtWidgets/QFileDialog>

enum StyleType { POKERTRAINING_DISTRIBUTED_STYLE, ADDITIONAL_STYLE };

using namespace std;

settingsDialogImpl::settingsDialogImpl(QWidget *parent, ConfigFile *c, selectAvatarDialogImpl *s, Log *l)
	: QDialog(parent), myConfig(c), mySelectAvatarDialogImpl(s), myLog(l)
{
#ifdef __APPLE__
	setWindowModality(Qt::ApplicationModal);
	setWindowFlags(Qt::WindowSystemMenuHint | Qt::CustomizeWindowHint | Qt::Dialog);
#endif
	setupUi(this);

#ifdef ANDROID
    listWidget->takeItem(1);
    listWidget->takeItem(1);
#endif


	myManualBlindsOrderDialog = new manualBlindsOrderDialogImpl;

	myAppDataPath = QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str());

	pushButton_openFlipsidePicture->setIcon(QIcon(QPixmap(myAppDataPath+"gfx/gui/misc/fileopen16.png")));

	comboBox_switchLanguage->addItem(QString(tr("English")+" "+QString::fromUtf8("(English)")),"en");
	comboBox_switchLanguage->addItem(QString(tr("French")+" "+QString::fromUtf8("(français)")),"fr");

	connect( buttonBox, SIGNAL( accepted() ), this, SLOT( isAccepted() ) );
	connect( pushButton_openFlipsidePicture, SIGNAL( clicked() ), this, SLOT( setFlipsidePicFileName()) );
	connect( pushButton_editManualBlindsOrder, SIGNAL( clicked() ), this, SLOT( callManualBlindsOrderDialog()) );
	connect( pushButton_resetHumanPlayerStatistics, SIGNAL( clicked() ), this, SLOT( resetHumanPlayerStatistics()) );
	connect( pushButton_resetAllPlayersStatistics, SIGNAL( clicked() ), this, SLOT( resetAllPlayersStatistics()) );

	connect( radioButton_manualBlindsOrder, SIGNAL( toggled(bool) ), this, SLOT( setFirstSmallBlindMargin() ));

	connect( comboBox_switchLanguage, SIGNAL( currentIndexChanged (int) ), this, SLOT ( setLanguageChanged(int) ));

	connect( treeWidget_gameTableStyles, SIGNAL( currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*) ), this, SLOT ( showCurrentGameTableStylePreview() ));
	connect( pushButton_addGameTableStyle, SIGNAL( clicked() ), this, SLOT( addGameTableStyle()) );
	connect( pushButton_removeGameTableStyle, SIGNAL( clicked() ), this, SLOT( removeGameTableStyle()) );

	connect( treeWidget_cardDeckStyles, SIGNAL( currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*) ), this, SLOT ( showCurrentCardDeckStylePreview() ));
	connect( pushButton_addCardDeckStyle, SIGNAL( clicked() ), this, SLOT( addCardDeckStyle()) );
	connect( pushButton_removeCardDeckStyle, SIGNAL( clicked() ), this, SLOT( removeCardDeckStyle()) );

#ifdef GUI_800x480
    //make the scrollbar touchable for mobile guis
    comboBox_switchLanguage->setStyleSheet("QObject {font: 30px} QScrollBar:vertical { border: 1px solid grey; background: white; width: 60px; margin: 0px -1px 0px 0px; } QScrollBar::handle:vertical { border-radius: 3px; border: 2px solid grey; background: LightGrey ; min-height: 60px; } QScrollBar::add-line:vertical { background: none; } QScrollBar::sub-line:vertical { background: none; } QScrollBar:up-arrow:vertical, QScrollBar::down-arrow:vertical { background: none; } QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }");
#endif

}

settingsDialogImpl::~settingsDialogImpl()
{
}

void settingsDialogImpl::prepareDialog()
{
	playerNickIsChanged = false;

	//Player Nicks
	lineEdit_HumanPlayerName->setText(QString::fromUtf8(myConfig->readConfigString("MyName").c_str()));
	pushButton_HumanPlayerAvatar->setLink(QString::fromUtf8(myConfig->readConfigString("MyAvatar").c_str()));
	lineEdit_Opponent1Name->setText(QString::fromUtf8(myConfig->readConfigString("Opponent1Name").c_str()));
	pushButton_Opponent1Avatar->setLink(QString::fromUtf8(myConfig->readConfigString("Opponent1Avatar").c_str()));
	lineEdit_Opponent2Name->setText(QString::fromUtf8(myConfig->readConfigString("Opponent2Name").c_str()));
	pushButton_Opponent2Avatar->setLink(QString::fromUtf8(myConfig->readConfigString("Opponent2Avatar").c_str()));
	lineEdit_Opponent3Name->setText(QString::fromUtf8(myConfig->readConfigString("Opponent3Name").c_str()));
	pushButton_Opponent3Avatar->setLink(QString::fromUtf8(myConfig->readConfigString("Opponent3Avatar").c_str()));
	lineEdit_Opponent4Name->setText(QString::fromUtf8(myConfig->readConfigString("Opponent4Name").c_str()));
	pushButton_Opponent4Avatar->setLink(QString::fromUtf8(myConfig->readConfigString("Opponent4Avatar").c_str()));
	lineEdit_Opponent5Name->setText(QString::fromUtf8(myConfig->readConfigString("Opponent5Name").c_str()));
	pushButton_Opponent5Avatar->setLink(QString::fromUtf8(myConfig->readConfigString("Opponent5Avatar").c_str()));
	lineEdit_Opponent6Name->setText(QString::fromUtf8(myConfig->readConfigString("Opponent6Name").c_str()));
	pushButton_Opponent6Avatar->setLink(QString::fromUtf8(myConfig->readConfigString("Opponent6Avatar").c_str()));
	lineEdit_Opponent7Name->setText(QString::fromUtf8(myConfig->readConfigString("Opponent7Name").c_str()));
	pushButton_Opponent7Avatar->setLink(QString::fromUtf8(myConfig->readConfigString("Opponent7Avatar").c_str()));
	lineEdit_Opponent8Name->setText(QString::fromUtf8(myConfig->readConfigString("Opponent8Name").c_str()));
	pushButton_Opponent8Avatar->setLink(QString::fromUtf8(myConfig->readConfigString("Opponent8Avatar").c_str()));
	lineEdit_Opponent9Name->setText(QString::fromUtf8(myConfig->readConfigString("Opponent9Name").c_str()));
	pushButton_Opponent9Avatar->setLink(QString::fromUtf8(myConfig->readConfigString("Opponent9Avatar").c_str()));
	//disable player nicks page if the dialogs is called ingame to prevent changes during game
	this->page_3->setDisabled(calledIngame);

	//refresh AvatarIcons
	pushButton_HumanPlayerAvatar->setIcon(QIcon(pushButton_HumanPlayerAvatar->getLink()));
	pushButton_Opponent1Avatar->setIcon(QIcon(pushButton_Opponent1Avatar->getLink()));
	pushButton_Opponent2Avatar->setIcon(QIcon(pushButton_Opponent2Avatar->getLink()));
	pushButton_Opponent3Avatar->setIcon(QIcon(pushButton_Opponent3Avatar->getLink()));
	pushButton_Opponent4Avatar->setIcon(QIcon(pushButton_Opponent4Avatar->getLink()));
	pushButton_Opponent5Avatar->setIcon(QIcon(pushButton_Opponent5Avatar->getLink()));
	pushButton_Opponent6Avatar->setIcon(QIcon(pushButton_Opponent6Avatar->getLink()));
	pushButton_Opponent7Avatar->setIcon(QIcon(pushButton_Opponent7Avatar->getLink()));
	pushButton_Opponent8Avatar->setIcon(QIcon(pushButton_Opponent8Avatar->getLink()));
	pushButton_Opponent9Avatar->setIcon(QIcon(pushButton_Opponent9Avatar->getLink()));

	// Game Settings
	spinBox_quantityPlayers->setValue(myConfig->readConfigInt("NumberOfPlayers"));
	spinBox_startCash->setValue(myConfig->readConfigInt("StartCash"));
	spinBox_firstSmallBlind->setValue(myConfig->readConfigInt("FirstSmallBlind"));
	radioButton_raiseBlindsAtHands->setChecked(myConfig->readConfigInt("RaiseBlindsAtHands"));
	radioButton_raiseBlindsAtMinutes->setChecked(myConfig->readConfigInt("RaiseBlindsAtMinutes"));
	spinBox_raiseSmallBlindEveryHands->setValue(myConfig->readConfigInt("RaiseSmallBlindEveryHands"));
	spinBox_raiseSmallBlindEveryMinutes->setValue(myConfig->readConfigInt("RaiseSmallBlindEveryMinutes"));
	radioButton_alwaysDoubleBlinds->setChecked(myConfig->readConfigInt("AlwaysDoubleBlinds"));
	radioButton_manualBlindsOrder->setChecked(myConfig->readConfigInt("ManualBlindsOrder"));
	spinBox_gameSpeed->setValue(myConfig->readConfigInt("GameSpeed"));
	checkBox_pauseBetweenHands->setChecked(myConfig->readConfigInt("PauseBetweenHands"));
	checkBox_showGameSettingsDialogOnNewGame->setChecked(myConfig->readConfigInt("ShowGameSettingsDialogOnNewGame"));


	//Interface
	comboBox_switchLanguage->setCurrentIndex(comboBox_switchLanguage->findData(QString::fromUtf8(myConfig->readConfigString("Language").c_str()).section('_', 0, 0)));
	checkBox_showFadeOutCardsAnimation->setChecked(myConfig->readConfigInt("ShowFadeOutCardsAnimation"));
	checkBox_showFlipCardsAnimation->setChecked(myConfig->readConfigInt("ShowFlipCardsAnimation"));
	checkBox_showBlindButtons->setChecked(myConfig->readConfigInt("ShowBlindButtons"));
	checkBox_antiPeekMode->setChecked(myConfig->readConfigInt("AntiPeekMode"));
	checkBox_dontTranslatePokerStrings->setChecked(myConfig->readConfigInt("DontTranslateInternationalPokerStringsFromStyle"));
	
	//S t y l e
	//TABLE

#ifdef GUI_800x480
	// 	define PokerTraining default GameTableStyle for Maemo
	treeWidget_gameTableStyles->clear();
        QString filename;
#ifdef MAEMO
        filename = "defaulttablestyle_800x480.xml";
#elif ANDROID
        filename = "android_tablestyle_800x480.xml";
#endif
	GameTableStyleReader defaultTableStyle(myConfig, this);
        defaultTableStyle.readStyleFile(QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"gfx/gui/table/default_800x480/"+filename);
	if(defaultTableStyle.getLoadedSuccessfull()) {
		QStringList tempStringList1;
		tempStringList1 << defaultTableStyle.getStyleDescription() << defaultTableStyle.getStyleMaintainerName();
		MyStyleListItem *defaultTableItem = new MyStyleListItem(tempStringList1, treeWidget_gameTableStyles);
                defaultTableItem->setData(0, 15, QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"gfx/gui/table/default_800x480/"+filename);
		defaultTableItem->setData(0, 16, POKERTRAINING_DISTRIBUTED_STYLE);
                defaultTableItem->setData(0, Qt::ToolTipRole, QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"gfx/gui/table/default_800x480/"+filename);
		defaultTableItem->setData(2, Qt::ToolTipRole, defaultTableStyle.getStateToolTipInfo());
		if(defaultTableStyle.getState()) defaultTableItem->setIcon(2, QIcon(":/gfx/emblem-important.png"));
		else defaultTableItem->setIcon(2, QIcon(":/gfx/dialog_ok_apply.png"));
	}
#else
	// 	define PokerTraining default GameTableStyle
	treeWidget_gameTableStyles->clear();

	GameTableStyleReader defaultTableStyle(myConfig, this);
	defaultTableStyle.readStyleFile(QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"gfx/gui/table/default/defaulttablestyle.xml");
	if(defaultTableStyle.getLoadedSuccessfull()) {
		QStringList tempStringList1;
		tempStringList1 << defaultTableStyle.getStyleDescription() << defaultTableStyle.getStyleMaintainerName();
		MyStyleListItem *defaultTableItem = new MyStyleListItem(tempStringList1, treeWidget_gameTableStyles);
		defaultTableItem->setData(0, 15, QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"gfx/gui/table/default/defaulttablestyle.xml");
		defaultTableItem->setData(0, 16, POKERTRAINING_DISTRIBUTED_STYLE);
		defaultTableItem->setData(0, Qt::ToolTipRole, QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"gfx/gui/table/default/defaulttablestyle.xml");
		defaultTableItem->setData(2, Qt::ToolTipRole, defaultTableStyle.getStateToolTipInfo());
		if(defaultTableStyle.getState()) defaultTableItem->setIcon(2, QIcon(":/gfx/emblem-important.png"));
		else defaultTableItem->setIcon(2, QIcon(":/gfx/dialog_ok_apply.png"));
	}
	//add danuxi table
	GameTableStyleReader danuxi1TableStyle(myConfig, this);
	danuxi1TableStyle.readStyleFile(QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"gfx/gui/table/danuxi1/danuxi1tablestyle.xml");
	if(danuxi1TableStyle.getLoadedSuccessfull()) {
		QStringList tempStringList2;
		tempStringList2 << danuxi1TableStyle.getStyleDescription() << danuxi1TableStyle.getStyleMaintainerName();
		MyStyleListItem *danuxi1TableItem = new MyStyleListItem(tempStringList2, treeWidget_gameTableStyles);
		danuxi1TableItem->setData(0, 15, QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"gfx/gui/table/danuxi1/danuxi1tablestyle.xml");
		danuxi1TableItem->setData(0, 16, POKERTRAINING_DISTRIBUTED_STYLE);
		danuxi1TableItem->setData(0, Qt::ToolTipRole, QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"gfx/gui/table/danuxi1/danuxi1tablestyle.xml");
		danuxi1TableItem->setData(2, Qt::ToolTipRole, danuxi1TableStyle.getStateToolTipInfo());
		if(danuxi1TableStyle.getState()) danuxi1TableItem->setIcon(2, QIcon(":/gfx/emblem-important.png"));
		else danuxi1TableItem->setIcon(2, QIcon(":/gfx/dialog_ok_apply.png"));
	}
#endif

	//load secondary styles into list (if fallback no entry)
	myGameTableStylesList = myConfig->readConfigStringList("GameTableStylesList");
	list<std::string>::iterator it1;
	for(it1= myGameTableStylesList.begin(); it1 != myGameTableStylesList.end(); ++it1) {
		GameTableStyleReader nextStyle(myConfig, this);
		nextStyle.readStyleFile(QString::fromUtf8(it1->c_str()));
		if(!nextStyle.getFallBack() && nextStyle.getLoadedSuccessfull()) {
			QStringList tempStringList1;
			tempStringList1 << nextStyle.getStyleDescription() << nextStyle.getStyleMaintainerName();
			MyStyleListItem *nextItem = new MyStyleListItem(tempStringList1, treeWidget_gameTableStyles);
			nextItem->setData(0, 15,QString::fromUtf8(it1->c_str()));
			nextItem->setData(0, 16, ADDITIONAL_STYLE);
			nextItem->setData(0, Qt::ToolTipRole,QString::fromUtf8(it1->c_str()));
			nextItem->setData(2, Qt::ToolTipRole, nextStyle.getStateToolTipInfo());
			if(nextStyle.getState()) nextItem->setIcon(2, QIcon(":/gfx/emblem-important.png"));
			else nextItem->setIcon(2, QIcon(":/gfx/dialog_ok_apply.png"));
			treeWidget_gameTableStyles->addTopLevelItem(nextItem);
		}
	}
	treeWidget_gameTableStyles->sortItems(0, Qt::AscendingOrder);

	//set current Game Table Style from config file or fallback to first entry
	GameTableStyleReader currentGameTableStyle(myConfig, this);
	currentGameTableStyle.readStyleFile(QString::fromUtf8(myConfig->readConfigString("CurrentGameTableStyle").c_str()));
	if(currentGameTableStyle.getLoadedSuccessfull()) {
		int i;
		bool currentGameTableFound(false);
		for(i=0; i < treeWidget_gameTableStyles->topLevelItemCount(); i++) {
			QTreeWidgetItem *item = treeWidget_gameTableStyles->topLevelItem(i);
			if(item->data(0, 15) == currentGameTableStyle.getCurrentFileName()) {
				item->setIcon(0, QIcon(QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"/gfx/gui/misc/rating.png"));
				treeWidget_gameTableStyles->setCurrentItem(item);
				currentGameTableFound=true;
			} else item->setIcon(0, QIcon());
		}
		if(!currentGameTableFound) {
			qDebug() << "Config ERROR: current game table style file not found in List. Try to mark default as selected.";
			QTreeWidgetItem *item = treeWidget_gameTableStyles->topLevelItem(0);
			if(item) {
				item->setIcon(0, QIcon(QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"/gfx/gui/misc/rating.png"));
				treeWidget_gameTableStyles->setCurrentItem(item);
			}
		}
	} else {
		qDebug() << "Config ERROR: current game table style file could not be loaded. Try to mark default as selected.";
		QTreeWidgetItem *item = treeWidget_gameTableStyles->topLevelItem(0);
		if(item) {
			item->setIcon(0, QIcon(QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"/gfx/gui/misc/rating.png"));
			treeWidget_gameTableStyles->setCurrentItem(item);
		}
	}

	//refresh Game Table Style Preview
	showCurrentGameTableStylePreview();

	//define PokerTraining default CardDeck
	treeWidget_cardDeckStyles->clear();
	CardDeckStyleReader defaultCardStyle(myConfig, this);
	defaultCardStyle.readStyleFile(QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"gfx/cards/default/defaultdeckstyle.xml");
	if(defaultCardStyle.getLoadedSuccessfull()) {
		QStringList tempStringList1;
		tempStringList1 << defaultCardStyle.getStyleDescription() << defaultCardStyle.getStyleMaintainerName();
		MyStyleListItem *defaultCardItem = new MyStyleListItem(tempStringList1, treeWidget_cardDeckStyles);
		defaultCardItem->setData(0, 15, QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"gfx/cards/default/defaultdeckstyle.xml");
		defaultCardItem->setData(0, 16, POKERTRAINING_DISTRIBUTED_STYLE);
		defaultCardItem->setData(0, Qt::ToolTipRole, QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"gfx/cards/default/defaultdeckstyle.xml");
	}
	//define PokerTraining default CardDeck4c
	CardDeckStyleReader default4cCardStyle(myConfig, this);
	default4cCardStyle.readStyleFile(QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"gfx/cards/default4c/default4cdeckstyle.xml");
	if(default4cCardStyle.getLoadedSuccessfull()) {
		QStringList tempStringList1;
		tempStringList1 << default4cCardStyle.getStyleDescription() << default4cCardStyle.getStyleMaintainerName();
		MyStyleListItem *default4cCardItem = new MyStyleListItem(tempStringList1, treeWidget_cardDeckStyles);
		default4cCardItem->setData(0, 15, QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"gfx/cards/default4c/default4cdeckstyle.xml");
        default4cCardItem->setData(0, 16, POKERTRAINING_DISTRIBUTED_STYLE);
		default4cCardItem->setData(0, Qt::ToolTipRole, QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"gfx/cards/default4c/default4cdeckstyle.xml");
	}

	//load secondary card styles into list (if fallback no entry)
	myCardDeckStylesList = myConfig->readConfigStringList("CardDeckStylesList");
	list<std::string>::iterator it2;
	for(it2= myCardDeckStylesList.begin(); it2 != myCardDeckStylesList.end(); ++it2) {
		CardDeckStyleReader nextStyle(myConfig, this);
		nextStyle.readStyleFile(QString::fromUtf8(it2->c_str()));
		if(!nextStyle.getFallBack() && nextStyle.getLoadedSuccessfull()) {
			QStringList tempStringList1;
			tempStringList1 << nextStyle.getStyleDescription() << nextStyle.getStyleMaintainerName();
			MyStyleListItem *nextItem = new MyStyleListItem(tempStringList1, treeWidget_cardDeckStyles);
			nextItem->setData(0, 15,QString::fromUtf8(it2->c_str()));
			nextItem->setData(0, 16, ADDITIONAL_STYLE);
			nextItem->setData(0, Qt::ToolTipRole,QString::fromUtf8(it2->c_str()));
			treeWidget_cardDeckStyles->addTopLevelItem(nextItem);
		}
	}
	treeWidget_cardDeckStyles->sortItems(0, Qt::AscendingOrder);

	//set current card deck style from config file
	CardDeckStyleReader currentCardDeckStyle(myConfig, this);
	currentCardDeckStyle.readStyleFile(QString::fromUtf8(myConfig->readConfigString("CurrentCardDeckStyle").c_str()));
	if(currentCardDeckStyle.getLoadedSuccessfull()) {
		int j;
		bool currentCardDeckFound(false);
		for(j=0; j < treeWidget_cardDeckStyles->topLevelItemCount(); j++) {
			QTreeWidgetItem *item = treeWidget_cardDeckStyles->topLevelItem(j);
			if(item->data(0, 15) == currentCardDeckStyle.getCurrentFileName()) {
				item->setIcon(0, QIcon(QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"/gfx/gui/misc/rating.png"));
				treeWidget_cardDeckStyles->setCurrentItem(item);
				currentCardDeckFound=true;
			} else item->setIcon(0, QIcon());
		}
		if(!currentCardDeckFound) {
			qDebug() << "Config ERROR: current card deck style file not found in List. Try to mark default as selected.";
			QTreeWidgetItem *item = treeWidget_cardDeckStyles->topLevelItem(0);
			if(item) {
				item->setIcon(0, QIcon(QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"/gfx/gui/misc/rating.png"));
				treeWidget_cardDeckStyles->setCurrentItem(item);
			}
		}
	} else {
		qDebug() << "Config ERROR: current card deck style file could not be loaded. Try to mark default as selected.";
		QTreeWidgetItem *item = treeWidget_cardDeckStyles->topLevelItem(0);
		if(item) {
			item->setIcon(0, QIcon(QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"/gfx/gui/misc/rating.png"));
			treeWidget_cardDeckStyles->setCurrentItem(item);
		}
	}

	// 	refresh Card Deck Style Preview
	showCurrentCardDeckStylePreview();

	//adjust column sizes for styles treeWidgets
	treeWidget_gameTableStyles->resizeColumnToContents(0);
	treeWidget_gameTableStyles->resizeColumnToContents(1);
	treeWidget_gameTableStyles->resizeColumnToContents(2);
	treeWidget_cardDeckStyles->resizeColumnToContents(0);
	treeWidget_cardDeckStyles->resizeColumnToContents(1);
	treeWidget_cardDeckStyles->resizeColumnToContents(2);


	radioButton_flipsideTux->setChecked(myConfig->readConfigInt("FlipsideTux"));
	radioButton_flipsideOwn->setChecked(myConfig->readConfigInt("FlipsideOwn"));
	if(radioButton_flipsideOwn->isChecked()) {
		lineEdit_OwnFlipsideFilename->setEnabled(true);
		pushButton_openFlipsidePicture->setEnabled(true);
	}
	lineEdit_OwnFlipsideFilename->setText(QString::fromUtf8(myConfig->readConfigString("FlipsideOwnFile").c_str()));

	//Sound
	groupBox_playSoundEffects->setChecked(myConfig->readConfigInt("PlaySoundEffects"));
	horizontalSlider_soundVolume->setValue(myConfig->readConfigInt("SoundVolume"));
	checkBox_playGameActions->setChecked(myConfig->readConfigInt("PlayGameActions"));
	checkBox_playBlindRaiseSound->setChecked(myConfig->readConfigInt("PlayBlindRaiseNotification"));

	//Manual Blinds Order Dialog ()
	myManualBlindsList = myConfig->readConfigIntList("ManualBlindsList");
	myAfterMBAlwaysDoubleBlinds = myConfig->readConfigInt("AfterMBAlwaysDoubleBlinds");
	myAfterMBAlwaysRaiseAbout = myConfig->readConfigInt("AfterMBAlwaysRaiseAbout");
	myAfterMBAlwaysRaiseValue = myConfig->readConfigInt("AfterMBAlwaysRaiseValue");
	myAfterMBStayAtLastBlind = myConfig->readConfigInt("AfterMBStayAtLastBlind");

	//Manual Blinds Order Dialog (network)
	myNetManualBlindsList = myConfig->readConfigIntList("NetManualBlindsList");
	myNetAfterMBAlwaysDoubleBlinds = myConfig->readConfigInt("NetAfterMBAlwaysDoubleBlinds");
	myNetAfterMBAlwaysRaiseAbout = myConfig->readConfigInt("NetAfterMBAlwaysRaiseAbout");
	myNetAfterMBAlwaysRaiseValue = myConfig->readConfigInt("NetAfterMBAlwaysRaiseValue");
	myNetAfterMBStayAtLastBlind = myConfig->readConfigInt("NetAfterMBStayAtLastBlind");

	setFirstSmallBlindMargin();

	//set this AFTER switch combobox like config-settings. This IS a currentIndexChanged() ;-)
	languageIsChanged = false;

}

int settingsDialogImpl::exec(bool in_game)
{
	calledIngame = in_game;
	prepareDialog();
	return QDialog::exec();
}

void settingsDialogImpl::isAccepted()
{

	settingsCorrect = true;

	// 	 Game Settings
	myConfig->writeConfigInt("NumberOfPlayers", spinBox_quantityPlayers->value());
	myConfig->writeConfigInt("StartCash", spinBox_startCash->value());
	myConfig->writeConfigInt("FirstSmallBlind", spinBox_firstSmallBlind->value());
	myConfig->writeConfigInt("RaiseBlindsAtHands", radioButton_raiseBlindsAtHands->isChecked());
	myConfig->writeConfigInt("RaiseBlindsAtMinutes", radioButton_raiseBlindsAtMinutes->isChecked());
	myConfig->writeConfigInt("RaiseSmallBlindEveryHands", spinBox_raiseSmallBlindEveryHands->value());
	myConfig->writeConfigInt("RaiseSmallBlindEveryMinutes", spinBox_raiseSmallBlindEveryMinutes->value());
	myConfig->writeConfigInt("AlwaysDoubleBlinds", radioButton_alwaysDoubleBlinds->isChecked());
	myConfig->writeConfigInt("ManualBlindsOrder", radioButton_manualBlindsOrder->isChecked());
	myConfig->writeConfigInt("GameSpeed", spinBox_gameSpeed->value());
	myConfig->writeConfigInt("PauseBetweenHands", checkBox_pauseBetweenHands->isChecked());
	myConfig->writeConfigInt("ShowGameSettingsDialogOnNewGame", checkBox_showGameSettingsDialogOnNewGame->isChecked());


	// 	Interface
	myConfig->writeConfigString("Language", comboBox_switchLanguage->itemData(comboBox_switchLanguage->currentIndex()).toString().toUtf8().constData());
	myConfig->writeConfigInt("ShowFadeOutCardsAnimation", checkBox_showFadeOutCardsAnimation->isChecked());
	myConfig->writeConfigInt("ShowFlipCardsAnimation", checkBox_showFlipCardsAnimation->isChecked());
	myConfig->writeConfigInt("ShowBlindButtons", checkBox_showBlindButtons->isChecked());
	myConfig->writeConfigInt("AntiPeekMode", checkBox_antiPeekMode->isChecked());
	myConfig->writeConfigInt("DontTranslateInternationalPokerStringsFromStyle", checkBox_dontTranslatePokerStrings->isChecked());
	myConfig->writeConfigInt("FlipsideTux", radioButton_flipsideTux->isChecked());
	myConfig->writeConfigInt("FlipsideOwn", radioButton_flipsideOwn->isChecked());

	if(radioButton_flipsideOwn->isChecked()) {
		QFile ownFlipsideFilename(lineEdit_OwnFlipsideFilename->text());
		if(ownFlipsideFilename.exists() && lineEdit_OwnFlipsideFilename->text() != "") {
			myConfig->writeConfigString("FlipsideOwnFile", lineEdit_OwnFlipsideFilename->text().toUtf8().constData());
		} else {
			MyMessageBox::warning(this, tr("Settings Error"),
								 tr("The entered flipside picture doesn't exist.\n"
									"Please enter an valid picture!"),
								 QMessageBox::Ok);
			settingsCorrect = false;
		}
	}

	//S T Y L E
	// 	TABLE
	//save game table styles list
	myGameTableStylesList.clear();
	int i;
	for(i=0; i < treeWidget_gameTableStyles->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = treeWidget_gameTableStyles->topLevelItem(i);
		if(item->data(0, 16).toInt() == ADDITIONAL_STYLE) {
			myGameTableStylesList.push_back(item->data(0, 15).toString().toUtf8().constData());
		}
	}
	myConfig->writeConfigStringList("GameTableStylesList", myGameTableStylesList);
	//save current game table style
	for(i=0; i < treeWidget_gameTableStyles->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = treeWidget_gameTableStyles->topLevelItem(i);
		if(!item->icon(0).isNull())
			myConfig->writeConfigString("CurrentGameTableStyle", item->data(0, 15).toString().toUtf8().constData());
	}
	// 	CARDS
	//save card deck styles list
	myCardDeckStylesList.clear();
	int j;
	for(j=0; j < treeWidget_cardDeckStyles->topLevelItemCount(); j++) {
		QTreeWidgetItem *item = treeWidget_cardDeckStyles->topLevelItem(j);
		if(item->data(0, 16).toInt() == ADDITIONAL_STYLE) {
			myCardDeckStylesList.push_back(item->data(0, 15).toString().toUtf8().constData());
		}
	}
	myConfig->writeConfigStringList("CardDeckStylesList", myCardDeckStylesList);
	//save current card deck style
	for(j=0; j < treeWidget_cardDeckStyles->topLevelItemCount(); j++) {
		QTreeWidgetItem *item = treeWidget_cardDeckStyles->topLevelItem(j);
		if(!item->icon(0).isNull())
			myConfig->writeConfigString("CurrentCardDeckStyle", item->data(0, 15).toString().toUtf8().constData());
	}

	//Sound
	myConfig->writeConfigInt("PlaySoundEffects", groupBox_playSoundEffects->isChecked());
	myConfig->writeConfigInt("SoundVolume", horizontalSlider_soundVolume->value());
	myConfig->writeConfigInt("PlayGameActions", checkBox_playGameActions->isChecked());
	myConfig->writeConfigInt("PlayBlindRaiseNotification", checkBox_playBlindRaiseSound->isChecked());

	//Manual Blinds Order Dialog ()
	myConfig->writeConfigIntList("ManualBlindsList",myManualBlindsList);
	myConfig->writeConfigInt("AfterMBAlwaysDoubleBlinds",myAfterMBAlwaysDoubleBlinds);
	myConfig->writeConfigInt("AfterMBAlwaysRaiseAbout",myAfterMBAlwaysRaiseAbout);
	myConfig->writeConfigInt("AfterMBAlwaysRaiseValue",myAfterMBAlwaysRaiseValue);
	myConfig->writeConfigInt("AfterMBStayAtLastBlind",myAfterMBStayAtLastBlind);

	//write buffer to disc
	myConfig->writeBuffer();

	if(languageIsChanged) {
		MyMessageBox::information(this, tr("Language Changed"),
								 tr("You have changed application language to %1.\nPlease restart PokerTraining to load new language!").arg(comboBox_switchLanguage->itemText(changedLanguageIndex)),
								 QMessageBox::Ok);
	}

	//Wenn alles richtig eingegeben wurde --> Dialog schließen
	if(settingsCorrect) {
		this->accept();
	}
}

void settingsDialogImpl::setFlipsidePicFileName()
{
	QDir flipSideDir(QDir::homePath());
	QFile flipSideFile(QString::fromUtf8(myConfig->readConfigString("FlipsideOwnFile").c_str()));
	if(flipSideFile.exists()) {
		flipSideDir = QDir(QString::fromUtf8(myConfig->readConfigString("FlipsideOwnFile").c_str()));
	}

	QString fileName = QFileDialog::getOpenFileName(this, tr("Select your flipside picture"),
					   flipSideDir.absolutePath(),
					   tr("Images (*.png *.jpg *.gif)"));

	if (!fileName.isEmpty())
		lineEdit_OwnFlipsideFilename->setText(fileName);
}

void settingsDialogImpl::setAvatarFile0()
{

	callSelectAvatarDialog();
	if(mySelectAvatarDialogImpl->getSettingsCorrect()) {
		pushButton_HumanPlayerAvatar->setLink(mySelectAvatarDialogImpl->getAvatarLink());
		pushButton_HumanPlayerAvatar->setIcon(QIcon(pushButton_HumanPlayerAvatar->getLink()));
	}
}

void settingsDialogImpl::setAvatarFile1()
{

	callSelectAvatarDialog();
	if(mySelectAvatarDialogImpl->getSettingsCorrect()) {
		pushButton_Opponent1Avatar->setLink(mySelectAvatarDialogImpl->getAvatarLink());
		pushButton_Opponent1Avatar->setIcon(QIcon(pushButton_Opponent1Avatar->getLink()));
	}
}

void settingsDialogImpl::setAvatarFile2()
{

	callSelectAvatarDialog();

	if(mySelectAvatarDialogImpl->getSettingsCorrect()) {
		pushButton_Opponent2Avatar->setLink(mySelectAvatarDialogImpl->getAvatarLink());
		pushButton_Opponent2Avatar->setIcon(QIcon(pushButton_Opponent2Avatar->getLink()));
	}
}

void settingsDialogImpl::setAvatarFile3()
{

	callSelectAvatarDialog();
	if(mySelectAvatarDialogImpl->getSettingsCorrect()) {
		pushButton_Opponent3Avatar->setLink(mySelectAvatarDialogImpl->getAvatarLink());
		pushButton_Opponent3Avatar->setIcon(QIcon(pushButton_Opponent3Avatar->getLink()));
	}
}

void settingsDialogImpl::setAvatarFile4()
{

	callSelectAvatarDialog();
	if(mySelectAvatarDialogImpl->getSettingsCorrect()) {
		pushButton_Opponent4Avatar->setLink(mySelectAvatarDialogImpl->getAvatarLink());
		pushButton_Opponent4Avatar->setIcon(QIcon(pushButton_Opponent4Avatar->getLink()));
	}
}

void settingsDialogImpl::setAvatarFile5()
{

	callSelectAvatarDialog();
	if(mySelectAvatarDialogImpl->getSettingsCorrect()) {
		pushButton_Opponent5Avatar->setLink(mySelectAvatarDialogImpl->getAvatarLink());
		pushButton_Opponent5Avatar->setIcon(QIcon(pushButton_Opponent5Avatar->getLink()));
	}
}

void settingsDialogImpl::setAvatarFile6()
{

	callSelectAvatarDialog();
	if(mySelectAvatarDialogImpl->getSettingsCorrect()) {
		pushButton_Opponent6Avatar->setLink(mySelectAvatarDialogImpl->getAvatarLink());
		pushButton_Opponent6Avatar->setIcon(QIcon(pushButton_Opponent6Avatar->getLink()));
	}
}

void settingsDialogImpl::setAvatarFile7()
{

	callSelectAvatarDialog();
	if(mySelectAvatarDialogImpl->getSettingsCorrect()) {
		pushButton_Opponent7Avatar->setLink(mySelectAvatarDialogImpl->getAvatarLink());
		pushButton_Opponent7Avatar->setIcon(QIcon(pushButton_Opponent7Avatar->getLink()));
	}
}

void settingsDialogImpl::setAvatarFile8()
{

	callSelectAvatarDialog();
	if(mySelectAvatarDialogImpl->getSettingsCorrect()) {
		pushButton_Opponent8Avatar->setLink(mySelectAvatarDialogImpl->getAvatarLink());
		pushButton_Opponent8Avatar->setIcon(QIcon(pushButton_Opponent8Avatar->getLink()));
	}
}

void settingsDialogImpl::setAvatarFile9()
{

	callSelectAvatarDialog();
	if(mySelectAvatarDialogImpl->getSettingsCorrect()) {
		pushButton_Opponent9Avatar->setLink(mySelectAvatarDialogImpl->getAvatarLink());
		pushButton_Opponent9Avatar->setIcon(QIcon(pushButton_Opponent9Avatar->getLink()));
	}
}
void settingsDialogImpl::resetHumanPlayerStatistics(){
	
	this->setCursor(Qt::WaitCursor);
	QApplication::processEvents();
 
	for (int j = 2; j <= MAX_NUMBER_OF_PLAYERS; j++){ 
		myLog->InitializePlayersStatistics(" ", j);
	}

	this->setCursor(Qt::ArrowCursor);

    MyMessageBox::warning(this, tr("Operation done"),
                                  tr("Human player statistics have been reinitialized to 0"),
                                  QMessageBox::Ok); 
}

void settingsDialogImpl::resetAllPlayersStatistics(){

	this->setCursor(Qt::WaitCursor);
	QApplication::processEvents();

	myLog->createDatabase();

	this->setCursor(Qt::ArrowCursor);

    MyMessageBox::warning(this, tr("Operation done"),
                                  tr("All players statistics have been reinitialized to 0"),
                                  QMessageBox::Ok); 	

}
void settingsDialogImpl::callManualBlindsOrderDialog()
{

	myManualBlindsOrderDialog->listWidget_blinds->clear();
	myManualBlindsOrderDialog->spinBox_input->setMinimum(spinBox_firstSmallBlind->value());

	list<int>::iterator it1;
	for(it1= myManualBlindsList.begin(); it1 != myManualBlindsList.end(); ++it1) {
		myManualBlindsOrderDialog->listWidget_blinds->addItem(QString::number(*it1,10));
	}
	myManualBlindsOrderDialog->sortBlindsList();

	myManualBlindsOrderDialog->radioButton_alwaysDoubleBlinds->setChecked(myAfterMBAlwaysDoubleBlinds);
	myManualBlindsOrderDialog->radioButton_alwaysRaiseAbout->setChecked(myAfterMBAlwaysRaiseAbout);
	myManualBlindsOrderDialog->spinBox_alwaysRaiseValue->setValue(myAfterMBAlwaysRaiseValue);
	myManualBlindsOrderDialog->radioButton_stayAtLastBlind->setChecked(myAfterMBStayAtLastBlind);

	myManualBlindsOrderDialog->exec();
	if(myManualBlindsOrderDialog->result() == QDialog::Accepted) {

		bool ok = true;
		int i;
		myManualBlindsList.clear();
		for(i=0; i<myManualBlindsOrderDialog->listWidget_blinds->count(); i++) {
			myManualBlindsList.push_back(myManualBlindsOrderDialog->listWidget_blinds->item(i)->text().toInt(&ok,10));
		}

		myAfterMBAlwaysDoubleBlinds = myManualBlindsOrderDialog->radioButton_alwaysDoubleBlinds->isChecked();
		myAfterMBAlwaysRaiseAbout = myManualBlindsOrderDialog->radioButton_alwaysRaiseAbout->isChecked();
		myAfterMBAlwaysRaiseValue = myManualBlindsOrderDialog->spinBox_alwaysRaiseValue->value();
		myAfterMBStayAtLastBlind = myManualBlindsOrderDialog->radioButton_stayAtLastBlind->isChecked();

		setFirstSmallBlindMargin();
	}
}

void settingsDialogImpl::setFirstSmallBlindMargin()
{

	if(radioButton_manualBlindsOrder->isChecked() && !myManualBlindsList.empty()) {
		if(spinBox_firstSmallBlind->value() > myManualBlindsList.front()) {
			MyMessageBox::warning(this, tr("Blinds Error"),
								 tr("The first element in your manual-blinds-list \nis smaller than current first-small-blind!\nThis first-small-blind-value will be set to maximum allowed value."),
								 QMessageBox::Close);
		}
		spinBox_firstSmallBlind->setMaximum(myManualBlindsList.front());
	} else {
		spinBox_firstSmallBlind->setMaximum(9999);
	}
}

void settingsDialogImpl::setLanguageChanged(int index)
{

	languageIsChanged = true;
	changedLanguageIndex = index;

}

void settingsDialogImpl::showCurrentGameTableStylePreview()
{
	QTreeWidgetItem* item = treeWidget_gameTableStyles->currentItem();
	if(item) {
		GameTableStyleReader style(myConfig, this);
		style.readStyleFile(item->data(0, 15).toString());
		QPixmap preview(style.getPreview());
		if(preview.height() > 160 || preview.width() > 120) label_gameTableStylePreview->setPixmap(preview.scaled(160,120,Qt::KeepAspectRatio,Qt::SmoothTransformation));
		else label_gameTableStylePreview->setPixmap(preview);

		QString MaintainerName = tr("Maintainer Name");
		QString MaintainerEMail = tr("Maintainer EMail");
		QString CreateDate = tr("Create Date");
		QString WindowBehaviour = tr("Windows Behaviour");
		QString scaleable = tr("scaleable");
		QString fixed = tr("fixed");
		QString MinimumSize = tr("Minimum Size");
		QString MaximumSize = tr("Maximum Size");
		QString FixedSize = tr("Fixed Size");
		QString State = tr("State");

		QString windowsSubString;
		if(style.getIfFixedWindowSize().toInt()) {
			windowsSubString = "<b>"+WindowBehaviour+":</b> "+fixed+"<br><b>"+FixedSize+":</b> "+style.getFixedWindowWidth()+"x"+style.getFixedWindowHeight();
		} else {
			windowsSubString = "<b>"+WindowBehaviour+":</b> "+scaleable+"<br><b>"+MinimumSize+":</b> "+style.getMinimumWindowWidth()+"x"+style.getMinimumWindowHeight()+"<br><b>"+MaximumSize+":</b> "+style.getMaximumWindowWidth()+"x"+style.getMaximumWindowHeight();
		}

		QString maintainerEMailString;
		if(style.getStyleMaintainerEMail() != "NULL" && style.getStyleMaintainerEMail() != "") {
			maintainerEMailString = "<b>"+MaintainerEMail+":</b> "+style.getStyleMaintainerEMail()+"<br>";
		}

		label_gameTableStyleInfo->setWordWrap(true);
		label_gameTableStyleInfo->setText("<b>"+MaintainerName+":</b> "+style.getStyleMaintainerName()+"<br>"+maintainerEMailString+"<b>"+CreateDate+":</b> "+style.getStyleCreateDate()+"<br>"+windowsSubString+"<br><b>"+State+": </b>"+style.getStateToolTipInfo());

		//active the current selected item directly
		setSelectedGameTableStyleActivated();

		//disable remove button for distributed styles
		if(item->data(0, 16).toInt() == POKERTRAINING_DISTRIBUTED_STYLE) {
			pushButton_removeGameTableStyle->setDisabled(true);
		} else {
			pushButton_removeGameTableStyle->setDisabled(false);
		}

	}
}

void settingsDialogImpl::setSelectedGameTableStyleActivated()
{
	QTreeWidgetItem* selectedItem = treeWidget_gameTableStyles->currentItem();
	if(selectedItem) {
		int i;
		for(i=0; i < treeWidget_gameTableStyles->topLevelItemCount(); i++) {
			QTreeWidgetItem *item = treeWidget_gameTableStyles->topLevelItem(i);
			if(item == selectedItem) {
				item->setIcon(0, QIcon(QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"/gfx/gui/misc/rating.png"));
			} else item->setIcon(0, QIcon());
		}
	}
}

void settingsDialogImpl::addGameTableStyle()
{
	QDir dir(QString::fromUtf8(myConfig->readConfigString("LastGameTableStyleDir").c_str()));
	QString dirString;

	if(dir.exists()) dirString = dir.absolutePath();
	else dirString = QDir::home().absolutePath();

	QString fileName = QFileDialog::getOpenFileName(this, tr("Please select your game table style"),
					   dirString,
					   tr("PokerTraining game table styles (*.xml)"));

	if (!fileName.isEmpty()) {

		int i;
		bool fileNameAlreadyFound(false);
		for(i=0; i < treeWidget_gameTableStyles->topLevelItemCount(); i++) {
			QTreeWidgetItem *item = treeWidget_gameTableStyles->topLevelItem(i);
			if(item->data(0, 15).toString() == fileName)
				fileNameAlreadyFound = true;
		}

		if(fileNameAlreadyFound) {
			MyMessageBox::warning(this, tr("Game Table Style Error"),
								 tr("Selected game table style file is already in the list. \nPlease select another one to add!"),
								 QMessageBox::Ok);
		} else {
			GameTableStyleReader newStyle(myConfig, this);
			newStyle.readStyleFile(fileName);
			if(!newStyle.getFallBack() && newStyle.getLoadedSuccessfull()) {
				QStringList tempStringList1;
				tempStringList1 << newStyle.getStyleDescription() << newStyle.getStyleMaintainerName();
				MyStyleListItem *newItem = new MyStyleListItem(tempStringList1, treeWidget_gameTableStyles);
				newItem->setData(0, 15,fileName);
				newItem->setData(0, 16, ADDITIONAL_STYLE);
				newItem->setData(0, Qt::ToolTipRole,fileName);
				newItem->setData(2, Qt::ToolTipRole, newStyle.getStateToolTipInfo());
				if(newStyle.getState() != GT_STYLE_OK) newItem->setIcon(2, QIcon(":/gfx/emblem-important.png"));
				else newItem->setIcon(2, QIcon(":/gfx/dialog-ok-apply.png"));
				treeWidget_gameTableStyles->addTopLevelItem(newItem);
				treeWidget_gameTableStyles->setCurrentItem(newItem);
				treeWidget_gameTableStyles->sortItems(0, Qt::AscendingOrder);

				qDebug() << "settings: " << newStyle.getStyleDescription() << newStyle.getState();
				if(newStyle.getState() != GT_STYLE_OK) newStyle.showErrorMessage();
			} else {
				MyMessageBox::warning(this, tr("Game Table Style File Error"),
									 tr("Could not load game table style file correctly. \nStyle will not be placed into list!"),
									 QMessageBox::Ok);
			}
		}
		//save current filepath
		myConfig->writeConfigString("LastGameTableStyleDir",QFileInfo(fileName).absolutePath().toUtf8().constData());

		//adjust column sizes for styles treeWidgets
		treeWidget_gameTableStyles->resizeColumnToContents(0);
		treeWidget_gameTableStyles->resizeColumnToContents(1);
		treeWidget_gameTableStyles->resizeColumnToContents(2);
	}
}

void settingsDialogImpl::removeGameTableStyle()
{
	QTreeWidgetItem* selectedItem = treeWidget_gameTableStyles->currentItem();
	//never delete PokerTraining defaullt Styles
	if(selectedItem && selectedItem->data(0, 16).toInt() == ADDITIONAL_STYLE) {
		// if selected is activated --> swith activation to first default
		if(!selectedItem->icon(0).isNull()) {
			QTreeWidgetItem* firstItem = treeWidget_gameTableStyles->topLevelItem(0);
			firstItem->setIcon(0, QIcon(QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"/gfx/gui/misc/rating.png"));
		}
		//remove from List
		treeWidget_gameTableStyles->takeTopLevelItem(treeWidget_gameTableStyles->currentIndex().row());

	}
}

void settingsDialogImpl::showCurrentCardDeckStylePreview()
{
	QTreeWidgetItem* item = treeWidget_cardDeckStyles->currentItem();
	if(item) {
		CardDeckStyleReader style(myConfig, this);
		style.readStyleFile(item->data(0, 15).toString());
		QPixmap preview(style.getPreview());
		if(preview.height() > 160 || preview.width() > 120) label_cardDeckStylePreview->setPixmap(preview.scaled(160,120,Qt::KeepAspectRatio,Qt::SmoothTransformation));
		else label_cardDeckStylePreview->setPixmap(preview);

		QString MaintainerName = tr("Maintainer Name");
		QString MaintainerEMail = tr("Maintainer EMail");
		QString CreateDate = tr("Create Date");

		QString maintainerEMailString;
		if(style.getStyleMaintainerEMail() != "NULL" && style.getStyleMaintainerEMail() != "") {
			maintainerEMailString = "<b>"+MaintainerEMail+":</b> "+style.getStyleMaintainerEMail()+"<br>";
		}

#ifdef GUI_800x480
		label_cardDeckStyleInfo->setWordWrap(true);
#endif
		label_cardDeckStyleInfo->setText("<b>"+MaintainerName+":</b> "+style.getStyleMaintainerName()+"<br>"+maintainerEMailString+"<b>"+CreateDate+":</b> "+style.getStyleCreateDate()+"");
		//active the current selected item directly
		setSelectedCardDeckStyleActivated();

		//disable remove button for distributed styles
		if(item->data(0, 16).toInt() == POKERTRAINING_DISTRIBUTED_STYLE) {
			pushButton_removeCardDeckStyle->setDisabled(true);
		} else {
			pushButton_removeCardDeckStyle->setDisabled(false);
		}
	}
}

void settingsDialogImpl::setSelectedCardDeckStyleActivated()
{
	QTreeWidgetItem* selectedItem = treeWidget_cardDeckStyles->currentItem();
	if(selectedItem) {
		int i;
		for(i=0; i < treeWidget_cardDeckStyles->topLevelItemCount(); i++) {
			QTreeWidgetItem *item = treeWidget_cardDeckStyles->topLevelItem(i);
			if(item == selectedItem) {
				item->setIcon(0, QIcon(QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"/gfx/gui/misc/rating.png"));
			} else item->setIcon(0, QIcon());
		}
	}
}

void settingsDialogImpl::addCardDeckStyle()
{
	QDir dir(QString::fromUtf8(myConfig->readConfigString("LastCardDeckStyleDir").c_str()));
	QString dirString;

	if(dir.exists()) dirString = dir.absolutePath();
	else dirString = QDir::home().absolutePath();

	QString fileName = QFileDialog::getOpenFileName(this, tr("Please select your card deck style"),
					   dirString,
					   tr("PokerTraining card deck styles (*.xml)"));

	if (!fileName.isEmpty()) {

		int i;
		bool fileNameAlreadyFound(false);
		for(i=0; i < treeWidget_cardDeckStyles->topLevelItemCount(); i++) {
			QTreeWidgetItem *item = treeWidget_cardDeckStyles->topLevelItem(i);
			if(item->data(0, 15).toString() == fileName)
				fileNameAlreadyFound = true;
		}

		if(fileNameAlreadyFound) {
			MyMessageBox::warning(this, tr("Card Deck Style Error"),
								 tr("Selected card deck style file is already in the list. \nPlease select another one to add!"),
								 QMessageBox::Ok);
		} else {
			CardDeckStyleReader newStyle(myConfig, this);
			newStyle.readStyleFile(fileName);
			if(!newStyle.getFallBack() && newStyle.getLoadedSuccessfull()) {
				QStringList tempStringList1;
				tempStringList1 << newStyle.getStyleDescription() << newStyle.getStyleMaintainerName();
				MyStyleListItem *newItem = new MyStyleListItem(tempStringList1, treeWidget_cardDeckStyles);
				newItem->setData(0, 15,fileName);
				newItem->setData(0, 16, ADDITIONAL_STYLE);
				newItem->setData(0, Qt::ToolTipRole,fileName);
				treeWidget_cardDeckStyles->addTopLevelItem(newItem);
				treeWidget_cardDeckStyles->setCurrentItem(newItem);
				treeWidget_cardDeckStyles->sortItems(0, Qt::AscendingOrder);
			} else {
				MyMessageBox::warning(this, tr("Card Deck Style File Error"),
									 tr("Could not load card deck style file correctly. \nStyle will not be placed into list!"),
									 QMessageBox::Ok);
			}
		}
		//save current filepath
		myConfig->writeConfigString("LastCardDeckStyleDir",QFileInfo(fileName).absolutePath().toUtf8().constData());

		treeWidget_cardDeckStyles->resizeColumnToContents(0);
		treeWidget_cardDeckStyles->resizeColumnToContents(1);
		treeWidget_cardDeckStyles->resizeColumnToContents(2);
	}
}

void settingsDialogImpl::removeCardDeckStyle()
{
	QTreeWidgetItem* selectedItem = treeWidget_cardDeckStyles->currentItem();
	//never delete PokerTraining defaullt Styles
	if(selectedItem && selectedItem->data(0, 16).toInt() == ADDITIONAL_STYLE) {
		// if selected is activated --> swith activation to first default
		if(!selectedItem->icon(0).isNull()) {
			QTreeWidgetItem* firstItem = treeWidget_cardDeckStyles->topLevelItem(0);
			firstItem->setIcon(0, QIcon(QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str())+"/gfx/gui/misc/rating.png"));
		}
		//remove from List
		treeWidget_cardDeckStyles->takeTopLevelItem(treeWidget_cardDeckStyles->currentIndex().row());
	}
}

void settingsDialogImpl::keyPressEvent ( QKeyEvent * event )
{
	if (event->key() == Qt::Key_Delete ) { /*Delete*/
	}
}
