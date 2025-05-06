/*****************************************************************************
 * PokerTraining - Texas Holdem No Limit training software          *
 * Copyright (C) 2025 Marc Ennaji                                            *
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
#include "manualblindsorderdialogimpl.h"
#include <configfile.h>
#include <qt/mymessagebox/mymessagebox.h>
#include <iostream>


manualBlindsOrderDialogImpl::manualBlindsOrderDialogImpl(QWidget *parent, ConfigFile *c)
	: QDialog(parent), myConfig(c), settingsCorrect(true)
{
#ifdef __APPLE__
	setWindowModality(Qt::ApplicationModal);
	setWindowFlags(Qt::WindowSystemMenuHint | Qt::CustomizeWindowHint | Qt::Dialog);
#endif
	setupUi(this);

	connect( pushButton_add, SIGNAL( clicked() ), this, SLOT( addBlindValueToList() ) );
	connect( pushButton_delete, SIGNAL( clicked() ), this, SLOT( removeBlindFromList() ) );

}

int manualBlindsOrderDialogImpl::exec()
{
	return QDialog::exec();
}


void manualBlindsOrderDialogImpl::addBlindValueToList()
{

	if(listWidget_blinds->count() == 30) {
		MyMessageBox::warning(this, tr("Manual Blinds Order"),
							 tr("You cannot set more than 30 manual blinds."),
							 QMessageBox::Close);
	} else {
		listWidget_blinds->addItem(QString::number(spinBox_input->value(),10));
		sortBlindsList();
	}
}

void manualBlindsOrderDialogImpl::removeBlindFromList()
{

	listWidget_blinds->takeItem(listWidget_blinds->currentRow());
	sortBlindsList();
}

void manualBlindsOrderDialogImpl::sortBlindsList()
{

	int i;
	QList<int> tempIntList;
	QStringList tempStringList;
	bool ok = true;

	for(i=0; i<listWidget_blinds->count(); i++) {
// 		std::cout << listWidget_blinds->item(i)->text().toInt(&ok,10) << "\n";
		tempIntList << listWidget_blinds->item(i)->text().toInt(&ok,10);
	}

	std::stable_sort(tempIntList.begin(), tempIntList.end());
//
	for(i=0; i<tempIntList.count(); i++) {
//
// 		std::cout << tempIntList[i] << "\n";
		tempStringList << QString::number(tempIntList[i],10);
	}
//
	listWidget_blinds->clear();
	listWidget_blinds->addItems(tempStringList);
}
