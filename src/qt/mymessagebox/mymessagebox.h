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

#ifndef MYMESSAGEBOX_H
#define MYMESSAGEBOX_H

#include <QtWidgets/QMessageBox>

class MyMessageBox : public QMessageBox
{
    Q_OBJECT
public:
    MyMessageBox(QWidget *parent = 0);

    MyMessageBox(Icon icon, const QString &title, const QString &text,
                    QFlags<QMessageBox::StandardButton> buttons = QMessageBox::NoButton, QWidget *parent = 0,
                    Qt::WindowFlags flags = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

    ~MyMessageBox();

    static QMessageBox::StandardButton information(QWidget *parent, const QString &title,
           const QString &text, QFlags<QMessageBox::StandardButton> buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    static QMessageBox::StandardButton question(QWidget *parent, const QString &title,
           const QString &text, QFlags<QMessageBox::StandardButton> buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    static QMessageBox::StandardButton warning(QWidget *parent, const QString &title,
           const QString &text, QFlags<QMessageBox::StandardButton> buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

    static QMessageBox::StandardButton critical(QWidget *parent, const QString &title,
           const QString &text, QFlags<QMessageBox::StandardButton> buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

signals:

public slots:

};

#endif // MYMESSAGEBOX_H
