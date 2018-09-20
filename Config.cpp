#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>

#include <stdio.h>


#include "Config.h"

bool Config::ConfigExists()
{
	//Does this file exist?
	FILE *fp;
	errno_t err;
	err = fopen_s(&fp, m_configfile.c_str(), "r");
	if (err != NULL) {
		return false;
	}
	err=fclose(fp);
	return true;
}

//need to create the folder since the stream writers do not....
bool Config::CreateConfig()
{
	FILE *fp;
	errno_t err;
	err = fopen_s(&fp, m_configfile.c_str(), "a");
	if (err != NULL) 
	{
		return false;
	}
	fprintf(fp, "%s", "");
	err = fclose(fp);
	return true;
}

//Load file data into std::string
bool Config::LoadStringData()
{
	FILE *fs;
	errno_t err;
	err = fopen_s(&fs, m_configfile.c_str(), "rb");
	if (err != NULL)
	{
		return false;
	}
	fseek(fs, 0, SEEK_END);
	UINT64 fsSize = ftell(fs);
	rewind(fs);
	char *fsBuffer = new char[fsSize + 1];
	ZeroMemory(fsBuffer, fsSize + 1);
	if (fsBuffer == NULL)
	{
		fclose(fs);
		return false; //zeromemory error
	}
	size_t result = fread(fsBuffer, 1, fsSize, fs);
	fclose(fs); //done with the file.
	if (result != fsSize)
	{
		return false;
	}
	m_filedata = std::string(fsBuffer);
	delete[] fsBuffer;
	return true;
}

bool Config::CreateFolder(const char folder_path[])
{
	bool created = CreateDirectory(folder_path, NULL); //windows.h
	if (!created)
	{
		if (ERROR_ALREADY_EXISTS == GetLastError())
		{
			return true; //as the error says, the folder already exists good to go.
		}
		return false; //else we could not create this folder for (reasons)
	}
	return true;
}

void Config::add_line_to_file(const char key_value[], const char default_value[])
{
	FILE *fs;
	fopen_s(&fs, m_configfile.c_str(), "at");
	fprintf(fs, "%s%s\r\n", key_value, default_value);
	fclose(fs);
	//add this new data to the internal file buffer so were not reloading the file again.
	m_filedata += std::string(key_value);
	m_filedata += std::string(default_value);
	m_filedata += "\r\n";
}

int Config::get_value_from_file(const char key_search[], std::string &string_out, const char default_value[])
{
	if (!ConfigExists()) {
		if (!CreateConfig())
		{
			return -1; //if it gets we could not create the config, and it didnt exist in the firstplace, or is open by another application.
		}
	}
	
	bool value_was_found = false;
	char *token1, *nexttoken;
	char found_value[8196] = "";
	int index_value = 0;
	std::string temp_buffer = m_filedata;
	token1 = strtok_s((char *)temp_buffer.c_str(), "\r\n", &nexttoken);
	while (token1)
	{
		if (!_strnicmp(token1, key_search, strlen(key_search))) {
			if (strlen(token1 + strlen(key_search)) < 1) {
				sprintf_s(found_value, "%s", default_value);
			}
			else {
				sprintf_s(found_value, "%s", token1 + strlen(key_search));
			}
			value_was_found = true;
			break;
		}
		token1 = strtok_s(NULL, "\r\n", &nexttoken);
		index_value++;
	}
	//if the value is missing add it to the file, and our current buffer.
	if (!value_was_found) {
		sprintf_s(found_value, "%s", default_value); //because we didnt find it we need to copy it
		add_line_to_file(key_search, default_value); //
		index_value++; //the data was written to the end of the file so index++
	}

	string_out = std::string(found_value);
	return index_value;
}

void Config::GetString(const char key_search[], std::string &string_out, const char default_value[])
{
	std::string tDefVal = "";
	if (default_value == NULL || strlen(default_value) == 0) { tDefVal = "Nothing"; }
	get_value_from_file(key_search, string_out, tDefVal.c_str());
}

void Config::GetInteger32(const char key_search[], UINT32 &Int_out, const char default_value[])
{
	std::string temp_string = "";
	std::string tDefVal = "";
	if (default_value == NULL || strlen(default_value) == 0) { tDefVal = "0"; }
	get_value_from_file(key_search, temp_string, tDefVal.c_str());
	Int_out = (UINT32)(GetINTfromString(temp_string.c_str(), temp_string.length()) & 0xFFFFFFFF);
}

void Config::GetHexInt32(const char key_search[], UINT32 &Int_out, const char default_value[])
{
	std::string temp_string = "";
	std::string tDefVal = "";
	if (default_value == NULL || strlen(default_value) == 0) { tDefVal = "00000000"; }
	get_value_from_file(key_search, temp_string, tDefVal.c_str());
	Int_out = GetHEXINTfromString(temp_string.c_str(), temp_string.length());
}

//Math values
UINT32 Config::GetHEXINTfromString(const char inBuf[], size_t inBufLen)
{
	if (strlen(inBuf) < inBufLen) { return 0; } //test the string length against suplied length.
	if (inBufLen == 0) { return 0; }

	size_t i;
	UINT32 outValue = 0;
	UINT32 tempValue = 0;
	UINT32 tempMaxIndex = (UINT32)(inBufLen - 1); //due to 0 based indexing
	UINT32 shiftValue = 0;

	for (i = 0; i < inBufLen; i++) {
		tempValue = ((UINT32)inBuf[i]) - 48;
		shiftValue = ((tempMaxIndex - (UINT32)i) * 4);
		//numerics
		if ((tempValue >= 0) && (tempValue <= 9)) {
			outValue += tempValue << shiftValue;
		}
		else {
			// A-F
			tempValue = ((UINT32)inBuf[i]) - 55; //UCASE A-F
			if ((tempValue >= 10) && (tempValue <= 15)) {
				outValue += tempValue << shiftValue;
			}
			else {
				//a-f
				tempValue = ((UINT32)inBuf[i]) - 87; //LCASE a-f
				if ((tempValue >= 10) && (tempValue <= 15)) {
					outValue += tempValue << shiftValue;
				}
				else {
					return 0; //malformed hex string
				}
			}
		}
	}

	return outValue;
}

UINT64 Config::GetINTfromString(const char inBuf[], size_t inBufLen)
{
	if (strlen(inBuf) < inBufLen) { return 0; } //test the string length against suplied length.
	if (inBufLen == 0) { return 0; }

	size_t i;
	UINT64 outValue = 0;
	UINT64 tempValue = 0;
	UINT64 addValue = 0;

	UINT64 tempMaxIndex = (UINT64)inBufLen - 1; //due to 0 based indexing

	if (inBufLen > 19) { return 0; } //10 to the power of 18 max (uint64)

	for (i = 0; i < inBufLen; i++) {
		tempValue = ((UINT64)inBuf[i]) - 48;
		if ((tempValue >= 0) && (tempValue <= 9)) {
			if (i == (tempMaxIndex)) {
				addValue += tempValue;
			}
			else {
				addValue = (UINT64)(std::pow((double)10, (tempMaxIndex - i))); //10^18 max
				addValue *= tempValue;
			}
			outValue += addValue;
			addValue = 0;
		}
		else {
			return 0; //else we got an non numerical
		}
	}

	return outValue;
}
