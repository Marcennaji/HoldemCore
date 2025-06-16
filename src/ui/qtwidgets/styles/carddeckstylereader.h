// PokerTraining — Texas Hold'em simulator
// Copyright (c) 2025 Marc Ennaji
// Licensed under the MIT License — see LICENSE file for details.
#ifndef CARDDECKSTYLEREADER_H
#define CARDDECKSTYLEREADER_H

#include <QtCore>
#include <QtGui>
#include <string>
#include <third_party/tiny/tinyxml.h>

#define POKERTRAINING_CD_STYLE_FILE_VERSION 1

enum CdStyleState
{
    CD_STYLE_OK = 0,
    CD_STYLE_OUTDATED,
    CD_STYLE_FIELDS_EMPTY,
    CD_STYLE_PICTURES_MISSING,
};

class CardDeckStyleReader : public QObject
{
    Q_OBJECT
  public:
    CardDeckStyleReader(QWidget* w);
    ~CardDeckStyleReader();

    void readStyleFile(QString, const std::string& appDataDir);

    QString getStyleDescription() const { return StyleDescription; }
    QString getStyleMaintainerName() const { return StyleMaintainerName; }
    QString getStyleMaintainerEMail() const { return StyleMaintainerEMail; }
    QString getStyleCreateDate() const { return StyleCreateDate; }

    QString getCurrentFileName() const { return currentFileName; }
    QString getCurrentDir() const { return currentDir; }

    QString getPreview() const { return Preview; }

    bool getFallBack() const { return fallBack; }
    bool getLoadedSuccessfull() const { return loadedSuccessfull; }

    void showLeftItemsErrorMessage(QString, QStringList, QString);
    void showCardsLeftErrorMessage(QString, QStringList, QString);

  private:
    QString StyleDescription;
    QString StyleMaintainerName;
    QString StyleMaintainerEMail;
    QString StyleCreateDate;
    QString PokerTrainingStyleFileVersion;
    QString Preview;

    QString currentFileName;
    QString currentDir;
    QByteArray fileContent;

    QStringList cardsLeft;
    QStringList leftItems;

    QWidget* myW;

    bool fallBack;
    bool loadedSuccessfull;
};

#endif
