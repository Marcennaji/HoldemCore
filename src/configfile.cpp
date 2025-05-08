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
#include "configfile.h"
#include <qt/qttoolsinterface.h>
#include <third_party/tiny/tinyxml.h>

#define MODUS 0711

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#endif

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <set>
#include <algorithm>

#include <sys/types.h>

using namespace std;

ConfigFile::ConfigFile(char *argv0, bool readonly) : noWriteAccess(readonly)
{
	myArgv0 = argv0;

	myQtToolsInterface = CreateQtToolsWrapper();

	myConfigState = OK;

	configRev = 104;

	// standard defaults
	logOnOffDefault = "1";

	const char *appDataPath = getenv("AppData");
	if (appDataPath && appDataPath[0] != 0)
	{
		configFileName = appDataPath;
	}
	else
	{
		const int MaxPathSize = 1024;
		char curDir[MaxPathSize + 1];
		curDir[0] = 0;
		_getcwd(curDir, MaxPathSize);
		curDir[MaxPathSize] = 0;
		configFileName = curDir;
		ofstream tmpFile;
		const char *tmpFileName = "pokertraining_test.tmp";
		tmpFile.open((configFileName + "\\" + tmpFileName).c_str());
		if (tmpFile)
		{
			tmpFile.close();
			remove((configFileName + "\\" + tmpFileName).c_str());
		}
		else
		{
			curDir[0] = 0;
			GetTempPathA(MaxPathSize, curDir);
			curDir[MaxPathSize] = 0;
			configFileName = curDir;
		}
	}
	// define app-dir
	configFileName += "\\pokertraining\\";
	////define log-dir
	logDir = configFileName;
	logDir += "log-files\\";
	////define data-dir
	dataDir = configFileName;
	dataDir += "data\\";

	// create directories on first start of app
	_mkdir(configFileName.c_str());
	_mkdir(logDir.c_str());
	_mkdir(dataDir.c_str());

	configList.push_back(ConfigInfo("AppDataDir", CONFIG_TYPE_STRING, myQtToolsInterface->getDataPathStdString(myArgv0)));
	configList.push_back(ConfigInfo("LogDir", CONFIG_TYPE_STRING, logDir));
	configList.push_back(ConfigInfo("UserDataDir", CONFIG_TYPE_STRING, dataDir));

	// fill tempList firstTime
	configBufferList = configList;

	// 	cout << configTempList[3].name << " " << configTempList[10].defaultValue << endl;

	if (!noWriteAccess)
	{
		configFileName += "config.xml";

		TiXmlDocument doc(configFileName);
		if (!doc.LoadFile())
		{
			myConfigState = NONEXISTING;
			updateConfig(myConfigState);
		}
		else
		{
			string tempAppDataPath("");
			TiXmlHandle docHandle(&doc);
			TiXmlElement *confAppDataPath = docHandle.FirstChild("PokerTraining").FirstChild("Configuration").FirstChild("AppDataDir").ToElement();
			if (confAppDataPath)
			{
				const char *tmpStr = confAppDataPath->Attribute("value");
				if (tmpStr)
					tempAppDataPath = tmpStr;
				if (tempAppDataPath != myQtToolsInterface->getDataPathStdString(myArgv0))
				{
					confAppDataPath->SetAttribute("value", myQtToolsInterface->stringToUtf8(myQtToolsInterface->getDataPathStdString(myArgv0)));
					doc.SaveFile(configFileName);
				}
			}
		}

		fillBuffer();
		checkAndCorrectBuffer();
	}
}

ConfigFile::~ConfigFile()
{
	delete myQtToolsInterface;
	myQtToolsInterface = 0;
}

void ConfigFile::fillBuffer()
{

	string tempString1("");
	string tempString2("");

	TiXmlDocument doc(configFileName);

	if (doc.LoadFile())
	{
		TiXmlHandle docHandle(&doc);

		for (size_t i = 0; i < configBufferList.size(); i++)
		{

			TiXmlElement *conf = docHandle.FirstChild("PokerTraining").FirstChild("Configuration").FirstChild(configList[i].name).ToElement();

			if (conf)
			{

				const char *tmpStr1 = conf->Attribute("value");
				if (tmpStr1)
					tempString1 = tmpStr1;
				configBufferList[i].defaultValue = tempString1;

				const char *tmpStr2 = conf->Attribute("type");
				if (tmpStr2)
				{
					tempString2 = tmpStr2;
					if (tempString2 == "list")
					{

						list<string> tempStringList2;

						TiXmlElement *confList = docHandle.FirstChild("PokerTraining").FirstChild("Configuration").FirstChild(configList[i].name).FirstChild().ToElement();

						for (; confList; confList = confList->NextSiblingElement())
						{
							tempStringList2.push_back(confList->Attribute("value"));
						}

						configBufferList[i].defaultListValue = tempStringList2;
					}
				}
			}

			// 			cout << configBufferList[i].name << " " << configBufferList[i].defaultValue << endl;
		}
	}
}

void ConfigFile::checkAndCorrectBuffer()
{
	// For now, only the player names are checked.
	checkAndCorrectPlayerNames();
}

void ConfigFile::checkAndCorrectPlayerNames()
{
	// Verify that the player names are uniquely set.
	set<string> playerNames;
	playerNames.insert(readConfigString("MyName"));
	for (int i = 1; i <= 9; i++)
	{
		ostringstream opponentVar;
		opponentVar << "Opponent" << i << "Name";
		playerNames.insert(readConfigString(opponentVar.str()));
	}
	if (playerNames.size() < 10 || playerNames.find("") != playerNames.end())
	{
		// The set contains less than 10 players or an empty player name.
		// Reset to default player names.
		writeConfigString("MyName", "Human Player");
		for (int i = 1; i <= 9; i++)
		{
			ostringstream opponentVar;
			ostringstream opponentName;
			opponentVar << "Opponent" << i << "Name";
			opponentName << "Player " << i;
			writeConfigString(opponentVar.str(), opponentName.str());
		}
	}
}

void ConfigFile::writeBuffer() const
{

	// write buffer to disc if enabled
	if (!noWriteAccess)
	{
		TiXmlDocument doc;
		TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "UTF-8", "");
		doc.LinkEndChild(decl);

		TiXmlElement *root = new TiXmlElement("PokerTraining");
		doc.LinkEndChild(root);

		TiXmlElement *config;
		config = new TiXmlElement("Configuration");
		root->LinkEndChild(config);

		size_t i;

		for (i = 0; i < configBufferList.size(); i++)
		{
			TiXmlElement *tmpElement = new TiXmlElement(configBufferList[i].name);
			config->LinkEndChild(tmpElement);
			tmpElement->SetAttribute("value", configBufferList[i].defaultValue);

			if (configBufferList[i].type == CONFIG_TYPE_INT_LIST || configBufferList[i].type == CONFIG_TYPE_STRING_LIST)
			{

				tmpElement->SetAttribute("type", "list");
				list<string> tempList = configBufferList[i].defaultListValue;
				list<string>::iterator it;
				for (it = tempList.begin(); it != tempList.end(); ++it)
				{

					TiXmlElement *tmpSubElement = new TiXmlElement(configBufferList[i].defaultValue);
					tmpElement->LinkEndChild(tmpSubElement);
					tmpSubElement->SetAttribute("value", *it);
				}
			}
		}

		doc.SaveFile(configFileName);
	}
}

void ConfigFile::updateConfig(ConfigState myConfigState)
{

	size_t i;

	if (myConfigState == NONEXISTING)
	{

		// Create a new ConfigFile!
		TiXmlDocument doc;
		TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "UTF-8", "");
		doc.LinkEndChild(decl);

		TiXmlElement *root = new TiXmlElement("PokerTraining");
		doc.LinkEndChild(root);

		TiXmlElement *config;
		config = new TiXmlElement("Configuration");
		root->LinkEndChild(config);

		for (i = 0; i < configList.size(); i++)
		{
			TiXmlElement *tmpElement = new TiXmlElement(configList[i].name);
			config->LinkEndChild(tmpElement);
			tmpElement->SetAttribute("value", myQtToolsInterface->stringToUtf8(configList[i].defaultValue));

			if (configList[i].type == CONFIG_TYPE_INT_LIST || configBufferList[i].type == CONFIG_TYPE_STRING_LIST)
			{

				tmpElement->SetAttribute("type", "list");
				list<string> tempList = configList[i].defaultListValue;
				list<string>::iterator it;
				for (it = tempList.begin(); it != tempList.end(); ++it)
				{

					TiXmlElement *tmpSubElement = new TiXmlElement(configList[i].defaultValue);
					tmpElement->LinkEndChild(tmpSubElement);
					tmpSubElement->SetAttribute("value", *it);
				}
			}
		}
		doc.SaveFile(configFileName);
	}

	if (myConfigState == OLD)
	{

		TiXmlDocument oldDoc(configFileName);

		// load the old one
		if (oldDoc.LoadFile())
		{

			string tempString1("");
			string tempString2("");

			TiXmlDocument newDoc;

			// Create the new one
			TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "UTF-8", "");
			newDoc.LinkEndChild(decl);

			TiXmlElement *root = new TiXmlElement("PokerTraining");
			newDoc.LinkEndChild(root);

			TiXmlElement *config;
			config = new TiXmlElement("Configuration");
			root->LinkEndChild(config);

			// change configRev and AppDataPath
			std::list<std::string> noUpdateElemtsList;

			TiXmlElement *confElement1 = new TiXmlElement("AppDataDir");
			config->LinkEndChild(confElement1);
			confElement1->SetAttribute("value", myQtToolsInterface->stringToUtf8(myQtToolsInterface->getDataPathStdString(myArgv0)));
			noUpdateElemtsList.push_back("AppDataDir");

			///////// VERSION HACK SECTION ///////////////////////
			// this is the right place for special version depending config hacks:
			// 0.9.1 - log interval needs to be set to 1 instead of 0
			if (configRev >= 95 && configRev <= 98)
			{ // this means 0.9.1 or 0.9.2 or 1.0
				TiXmlElement *confElement2 = new TiXmlElement("LogInterval");
				config->LinkEndChild(confElement2);
				confElement2->SetAttribute("value", 1);
				noUpdateElemtsList.push_back("LogInterval");
			}

			if (configRev == 98)
			{ // this means 1.0
				TiXmlElement *confElement3 = new TiXmlElement("CurrentCardDeckStyle");
				config->LinkEndChild(confElement3);
				confElement3->SetAttribute("value", "");
				noUpdateElemtsList.push_back("CurrentCardDeckStyle");
			}
			///////// VERSION HACK SECTION ///////////////////////

			TiXmlHandle oldDocHandle(&oldDoc);

			for (i = 0; i < configList.size(); i++)
			{

				TiXmlElement *oldConf = oldDocHandle.FirstChild("PokerTraining").FirstChild("Configuration").FirstChild(configList[i].name).ToElement();

				if (oldConf)
				{ // if element is already there --> take over the saved values

					// dont update ConfigRevision and AppDataDir AND possible hacked Config-Elements becaus it was already set ^^
					if (count(noUpdateElemtsList.begin(), noUpdateElemtsList.end(), configList[i].name) == 0)
					{

						TiXmlElement *tmpElement = new TiXmlElement(configList[i].name);
						config->LinkEndChild(tmpElement);

						const char *tmpStr1 = oldConf->Attribute("value");
						if (tmpStr1)
							tempString1 = tmpStr1;
						tmpElement->SetAttribute("value", tempString1);

						// for lists copy elements
						const char *tmpStr2 = oldConf->Attribute("type");
						if (tmpStr2)
						{
							tempString2 = tmpStr2;
							if (tempString2 == "list")
							{

								list<string> tempStringList2;

								TiXmlElement *oldConfList = oldDocHandle.FirstChild("PokerTraining").FirstChild("Configuration").FirstChild(configList[i].name).FirstChild().ToElement();

								for (; oldConfList; oldConfList = oldConfList->NextSiblingElement())
								{
									tempStringList2.push_back(oldConfList->Attribute("value"));
								}

								tmpElement->SetAttribute("type", "list");
								list<string> tempList = tempStringList2;
								list<string>::iterator it;
								for (it = tempList.begin(); it != tempList.end(); ++it)
								{

									TiXmlElement *tmpSubElement = new TiXmlElement(tempString1);
									tmpElement->LinkEndChild(tmpSubElement);
									tmpSubElement->SetAttribute("value", *it);
								}
							}
						}
					}
				}
				else
				{
					// if element is not there --> set it with defaultValue
					TiXmlElement *tmpElement = new TiXmlElement(configList[i].name);
					config->LinkEndChild(tmpElement);
					tmpElement->SetAttribute("value", myQtToolsInterface->stringToUtf8(configList[i].defaultValue));

					if (configList[i].type == CONFIG_TYPE_INT_LIST || configBufferList[i].type == CONFIG_TYPE_STRING_LIST)
					{

						tmpElement->SetAttribute("type", "list");
						list<string> tempList = configList[i].defaultListValue;
						list<string>::iterator it;
						for (it = tempList.begin(); it != tempList.end(); ++it)
						{

							TiXmlElement *tmpSubElement = new TiXmlElement(configList[i].defaultValue);
							tmpElement->LinkEndChild(tmpSubElement);
							tmpSubElement->SetAttribute("value", *it);
						}
					}
				}
			}
			newDoc.SaveFile(configFileName);
		}
	}
}

ConfigState ConfigFile::getConfigState() const
{
	return myConfigState;
}

string ConfigFile::readConfigString(string varName) const
{

	size_t i;
	string tempString("");

	for (i = 0; i < configBufferList.size(); i++)
	{

		if (configBufferList[i].name == varName)
		{
			tempString = configBufferList[i].defaultValue;
		}
	}

	return tempString;
}

int ConfigFile::readConfigInt(string varName) const
{

	size_t i;
	string tempString("");
	int tempInt = 0;

	for (i = 0; i < configBufferList.size(); i++)
	{

		if (configBufferList[i].name == varName)
		{
			tempString = configBufferList[i].defaultValue;
		}
	}

	istringstream isst;
	isst.str(tempString);
	isst >> tempInt;

	return tempInt;
}

list<int> ConfigFile::readConfigIntList(string varName) const
{

	size_t i;
	list<string> tempStringList;
	list<int> tempIntList;

	for (i = 0; i < configBufferList.size(); i++)
	{

		if (configBufferList[i].name == varName)
		{
			tempStringList = configBufferList[i].defaultListValue;
		}
	}

	istringstream isst;
	int tempInt;
	list<string>::iterator it;
	for (it = tempStringList.begin(); it != tempStringList.end(); ++it)
	{

		isst.str(*it);
		isst >> tempInt;
		tempIntList.push_back(tempInt);
		isst.str("");
		isst.clear();
	}

	return tempIntList;
}

list<string> ConfigFile::readConfigStringList(string varName) const
{

	size_t i;
	list<string> tempStringList;

	for (i = 0; i < configBufferList.size(); i++)
	{

		if (configBufferList[i].name == varName)
		{
			tempStringList = configBufferList[i].defaultListValue;
		}
	}

	return tempStringList;
}

void ConfigFile::writeConfigInt(string varName, int varCont)
{

	size_t i;
	ostringstream intToString;

	for (i = 0; i < configBufferList.size(); i++)
	{

		if (configBufferList[i].name == varName)
		{
			intToString << varCont;
			configBufferList[i].defaultValue = intToString.str();
		}
	}
}

void ConfigFile::writeConfigIntList(string varName, list<int> varCont)
{

	size_t i;
	ostringstream intToString;
	list<string> stringList;

	for (i = 0; i < configBufferList.size(); i++)
	{

		if (configBufferList[i].name == varName)
		{
			list<int>::iterator it;
			for (it = varCont.begin(); it != varCont.end(); ++it)
			{

				intToString << (*it);
				stringList.push_back(intToString.str());
				intToString.str("");
				intToString.clear();
			}

			configBufferList[i].defaultListValue = stringList;
		}
	}
}

void ConfigFile::writeConfigString(string varName, string varCont)
{

	size_t i;
	for (i = 0; i < configBufferList.size(); i++)
	{
		if (configBufferList[i].name == varName)
		{
			configBufferList[i].defaultValue = varCont;
		}
	}
}

void ConfigFile::writeConfigStringList(string varName, list<string> varCont)
{

	size_t i;
	for (i = 0; i < configBufferList.size(); i++)
	{

		if (configBufferList[i].name == varName)
		{
			configBufferList[i].defaultListValue = varCont;
		}
	}
}

void ConfigFile::deleteConfigFile()
{
	remove(configFileName.c_str());
}
