#pragma once

#include <windows.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <time.h>

#include "sCritSect.h"

#define CONFIG_F "DATA\\DRAGONBOT.CFG"

//			Create Default Folder if non existant	[done]
//			Create Config file if non existant		[done] -> the above was needed before this.
//			Dump file contents into std string		[done] --> both the above needed before this.
//			Create function to get value by key		[done]
//			Create function to add line to file		[done] -> this works with the above, via default_value 
//															  (if said key is missing this will save the key 
//															   with a value set in the default)
//			Get string from key						[done] -v
//			Get Integer from key					[done] -> by default if you dont provide a default value one will be set for you.
//			Get Hex integer from key				[done] -^	v
//																The 3 main get function default values can be set as "" or NULL 
//																to enforce the defaults.

enum MessageTypes
{
	Error = 1, Warning = 2, Good = 3
};

struct TIME_PASSED {
	int years;
	int months;
	int weeks;
	int days;
	int hours;
	int minutes;
	int seconds;
};

#define tpYEARS (tpMONTHS * 12)
#define tpMONTHS (tpWEEKS * 4)
#define tpWEEKS (tpDAYS * 7)
#define tpDAYS (tpHOURS * 24)
#define tpHOURS (tpMINS * 60)
#define tpMINS 60
#define LASTSEEN "_LASTSEEN="

class Config
{
private:
	//vars
	SCritSect SCrit; //Critical section

	std::string m_configfile;
	std::string m_filedata;

private: //methods
	bool ConfigExists();
	bool CreateConfig();
	bool LoadStringData();
	void add_line_to_file(const char key_value[], const char default_value[]);
	int get_value_from_file(const char key_search[], std::string &string_out, const char default_value[]);
	UINT32 GetHEXINTfromString(const char inBuf[], size_t inBufLen);
	UINT64 GetINTfromString(const char inBuf[], size_t inBufLen);
	void delete_line_in_file(int line_number, std::string replace_value);
	void Save();

public: //methods
	bool CreateFolder(const char folder_path[]);

	void GetString(const char key_search[], std::string &string_out, const char default_value[]);
	void GetInteger32(const char key_search[], UINT32 &Int_out, const char default_value[]);
	void GetHexInt32(const char key_search[], UINT32 &Int_out, const char default_value[]);
	void GetLastSeen(const char user[], std::string &out_value);

	void SetInteger32(const char key_search[], UINT32 dwValue);		//convert integer to string
	//void SetHexInt32(const char key_search[], UINT32 dwValue);	//convert int to hex string
	void SetString(const char key_search[], const char strValue[]);	//obvious
	void SetLastSeen(const char user[]);							//if this gets called we already know what time it is.

	time_t long_math_time_passed(time_t value);
	TIME_PASSED do_time_passed(time_t in_value);
	std::string more_than_one_day_etc(int value);
	void asc_time_passed(TIME_PASSED values, std::string &outbuf);

public: //typedefs
	//Message out optional.
	typedef void(*_MessageOut)(MessageTypes MessageType, const char sMessageOut[]);
	struct _FunctionPointers {
		_MessageOut MessageOut;
	} _events;

public:
	//methods
	Config(_MessageOut objFunc) : m_configfile(CONFIG_F)
	{ 
		std::string outMessage = "";

		_events.MessageOut = (_MessageOut &)objFunc;
		_events.MessageOut(MessageTypes::Warning, "Loading..");

		if (!CreateFolder(".\\DATA\\")) //if the folder exists or gets created this will be true.
		{
			//failed to create or some other error do not continue due to we cant create the file if the folder dosent exist.
			return;
		}
		outMessage = "Folder [" + std::string("DATA") + "], exists.";
		_events.MessageOut(MessageTypes::Warning, outMessage.c_str());

		if (!ConfigExists())
		{
			//Config dosen't exist, lets create it.
			outMessage = "Config not found [" + m_configfile + "], attempting to create it.";
			_events.MessageOut(MessageTypes::Warning, outMessage.c_str());
			if (!CreateConfig())
			{
				//failed to create, or is open by another application?
				outMessage = "Config [" + m_configfile + "], could not be created or is open by another device.";
				_events.MessageOut(MessageTypes::Error, outMessage.c_str());
				return;
			}
			outMessage = "Config [" + m_configfile + "], has been created and is ready to go.";
			_events.MessageOut(MessageTypes::Good, outMessage.c_str());
		}
		//Load config?
		//dump all contents of the config into a std::string?
		if (!LoadStringData())
		{
			return; //failed to dump the file into the string.
		}

		//testing get value.
		/*std::string sBuf = "";
		UINT32 TestInt = 0, TestHex = 0;

		GetString("asdffdsa=", sBuf, "GetStringTest");
		GetInteger32("TestInteger=", TestInt, "2863311530");	//0xAAAAAAAA
		GetHexInt32("TestHex=", TestHex, "BBBBBBBB");			//3149642683
		GetString("N2asdffdsa=", sBuf, "");				//Nothing
		GetInteger32("NTestInteger=", TestInt, "");		//0
		GetHexInt32("NTestHex=", TestHex, "");			//00000000
		GetString("N3asdffdsa=", sBuf, NULL);				//Nothing
		GetInteger32("N2TestInteger=", TestInt, NULL);		//0
		GetHexInt32("N2TestHex=", TestHex, NULL);			//00000000*/
	}

	Config() : m_configfile(CONFIG_F)
	{
		//STD file streams being used, means the folders need to exist must be created 1 by 1
		if (!CreateFolder(".\\DATA\\"))	{ return; } 
		//if we got here the folder was either created or it already existed.
		if (!ConfigExists())
		{
			if (!CreateConfig()) { return; }
		}
		//if we got here our config file was created or already existed.
		if (!LoadStringData()) { return; } //failed to dump the file into the string.
		//if we got here then we have all the contents of the file in m_filedata
	}
	Config(const std::string& inConfigFile) : m_configfile(inConfigFile) 
	{
		//STD file streams being used, means the folders need to exist must be created 1 by 1
		if (!CreateFolder(".\\DATA\\")) { return; }
		//if we got here the folder was either created or it already existed.
		if (!ConfigExists())
		{
			if (!CreateConfig()) { return; }
		}
		//if we got here our config file was created or already existed.
		if (!LoadStringData()) { return; } //failed to dump the file into the string.
		//if we got here then we have all the contents of the file in m_filedata
	}
	~Config() { }


};
