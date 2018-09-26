#include "Config.h"

bool Config::ConfigExists()
{
	SCrit.enter();
	//Does this file exist?
	FILE *fp;
	errno_t err;
	err = fopen_s(&fp, m_configfile.c_str(), "r");
	if (err != NULL) {
		return false;
	}
	err=fclose(fp);
	SCrit.leave();
	return true;
}

//need to create the folder since the stream writers do not.... (pretty sure they used to on XP and lower)
bool Config::CreateConfig()
{
	SCrit.enter();
	FILE *fp;
	errno_t err;
	err = fopen_s(&fp, m_configfile.c_str(), "a");
	if (err != NULL) 
	{
		return false;
	}
	fprintf(fp, "%s", "");
	err = fclose(fp);
	SCrit.leave();
	return true;
}

//Load file data into std::string
bool Config::LoadStringData()
{
	SCrit.enter();

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

	SCrit.leave();
	return true;
}

bool Config::CreateFolder(const char folder_path[])
{
	SCrit.enter();

	bool created = CreateDirectory(folder_path, NULL); //windows.h
	if (!created)
	{
		if (ERROR_ALREADY_EXISTS == GetLastError())
		{
			return true; //as the error says, the folder already exists good to go.
		}
		return false; //else we could not create this folder for (reasons)
	}

	SCrit.leave();
	return true;
}

void Config::add_line_to_file(const char key_value[], const char default_value[])
{
	SCrit.enter();

	FILE *fs;
	fopen_s(&fs, m_configfile.c_str(), "at");
	fprintf(fs, "%s%s\r\n", key_value, default_value);
	fclose(fs);
	//add this new data to the internal file buffer so were not reloading the file again.
	m_filedata += std::string(key_value);
	m_filedata += std::string(default_value);
	m_filedata += "\r\n";

	SCrit.leave();
}

int Config::get_value_from_file(const char key_search[], std::string &string_out, const char default_value[])
{
	SCrit.enter();

	if (!ConfigExists()) {
		if (!CreateConfig())
		{
			return -1; //if it gets here we could not create the config, and it didnt exist in the firstplace, or is open by another application, or [folder dosent exist (should not happen see: constructors)].
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
	SCrit.leave();

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
	else { tDefVal = std::string(default_value); }
	get_value_from_file(key_search, string_out, tDefVal.c_str());
}

void Config::GetInteger32(const char key_search[], UINT32 &Int_out, const char default_value[])
{
	std::string temp_string = "";
	std::string tDefVal = "";
	if (default_value == NULL || strlen(default_value) == 0) { tDefVal = "0"; }
	else { tDefVal = std::string(default_value); }
	get_value_from_file(key_search, temp_string, tDefVal.c_str());
	Int_out = (UINT32)(GetINTfromString(temp_string.c_str(), temp_string.length()) & 0xFFFFFFFF);
}

void Config::GetHexInt32(const char key_search[], UINT32 &Int_out, const char default_value[])
{
	std::string temp_string = "";
	std::string tDefVal = "";
	if (default_value == NULL || strlen(default_value) == 0) { tDefVal = "00000000"; }
	else { tDefVal = std::string(default_value); }
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

/*
lastseen value stored as time_t

	time_t time_value_now;
	time(&time_value_now);
	time_t config_lastseen_time;

	Format to more likeable data (time_value_now - config_lastseen_time)
*/
void Config::GetLastSeen(const char user[], std::string &out_value)
{
	time_t tNow;
	time(&tNow);
	std::string default_value = std::to_string(tNow);
	std::string temp_key = std::string(user);
	temp_key += std::string(LASTSEEN);
	std::string temp_value;

	GetString(temp_key.c_str(), temp_value, default_value.c_str());
	//now format actual_value and put in out_value
	SCrit.enter();
	TIME_PASSED tpData = do_time_passed(long_math_time_passed(GetINTfromString(temp_value.c_str(), temp_value.length())));
	asc_time_passed(tpData, out_value);
	SCrit.leave();
}

time_t Config::long_math_time_passed(time_t value)
{
	time_t time_value_now;
	time(&time_value_now);
	return time_value_now - value;
}

TIME_PASSED Config::do_time_passed(time_t in_value)
{
	time_t value = in_value;
	TIME_PASSED outValues;
	outValues.years = (int)(value / tpYEARS);
	if (outValues.years >= 1) { value -= outValues.years * tpYEARS; }
	outValues.months = (int)(value / tpMONTHS);
	if (outValues.months >= 1) { value -= outValues.months * tpMONTHS; }
	outValues.weeks = (int)(value / tpWEEKS);
	if (outValues.weeks >= 1) { value -= outValues.weeks * tpWEEKS; }
	outValues.days = (int)(value / tpDAYS);
	if (outValues.days >= 1) { value -= outValues.days * tpDAYS; }
	outValues.hours = (int)(value / tpHOURS);
	if (outValues.hours >= 1) { value -= outValues.hours * tpHOURS; }
	outValues.minutes = (int)(value / tpMINS);
	if (outValues.minutes >= 1) { value -= outValues.minutes * tpMINS; }
	outValues.seconds = (int)(value);

	return outValues;
}

std::string Config::more_than_one_day_etc(int value)
{
	if (value > 1) { return "s"; }
	return "";
}

void Config::asc_time_passed(TIME_PASSED values, std::string &outBuf)
{
	char outYears[32] = "";
	if (values.years >= 1) {
		sprintf_s(outYears, "%i year%s", values.years, more_than_one_day_etc(values.years).c_str());
	}
	char outMonths[32] = "";
	if (values.months >= 1) {
		if (strlen(outYears) > 0) {
			sprintf_s(outMonths, ", %i month%s", values.months, more_than_one_day_etc(values.months).c_str());
		}
		else {
			sprintf_s(outMonths, "%i month%s", values.months, more_than_one_day_etc(values.months).c_str());
		}
	}
	char outWeeks[32] = "";
	if (values.weeks >= 1) {
		if (strlen(outYears) > 0 || strlen(outMonths) > 0) {
			sprintf_s(outWeeks, ", %i week%s", values.weeks, more_than_one_day_etc(values.weeks).c_str());
		}
		else {
			sprintf_s(outWeeks, "%i week%s", values.weeks, more_than_one_day_etc(values.weeks).c_str());
		}
	}
	char outDays[32] = "";
	if (values.days >= 1) {
		if (strlen(outYears) > 0 || strlen(outMonths) > 0 || strlen(outWeeks) > 0) {
			sprintf_s(outDays, ", %i day%s", values.days, more_than_one_day_etc(values.days).c_str());
		}
		else {
			sprintf_s(outDays, "%i day%s", values.days, more_than_one_day_etc(values.days).c_str());
		}
	}
	char outHours[32] = "";
	if (values.hours >= 1) {
		if (strlen(outYears) > 0 || strlen(outMonths) > 0 || strlen(outWeeks) > 0 || strlen(outDays) > 0) {
			sprintf_s(outHours, ", %i hour%s", values.hours, more_than_one_day_etc(values.hours).c_str());
		}
		else {
			sprintf_s(outHours, "%i hour%s", values.hours, more_than_one_day_etc(values.hours).c_str());
		}
	}
	char outMins[32] = "";
	if (values.minutes >= 1) {
		if (strlen(outYears) > 0 || strlen(outMonths) > 0 || strlen(outWeeks) > 0 || strlen(outDays) > 0 || strlen(outHours) > 0) {
			sprintf_s(outMins, ", %i minute%s", values.minutes, more_than_one_day_etc(values.minutes).c_str());
		}
		else {
			sprintf_s(outMins, "%i minute%s", values.minutes, more_than_one_day_etc(values.minutes).c_str());
		}
	}
	char outSecs[32] = "";
	if (values.seconds >= 1) {
		if (strlen(outYears) > 0 || strlen(outMonths) > 0 || strlen(outWeeks) > 0 || strlen(outDays) > 0 || strlen(outHours) > 0 || strlen(outMins) > 0) {
			sprintf_s(outSecs, ", %i second%s", values.seconds, more_than_one_day_etc(values.seconds).c_str());
		}
		else {
			sprintf_s(outSecs, "%i second%s", values.seconds, more_than_one_day_etc(values.seconds).c_str());
		}
	}
	char outBuffer[128]; //(70)"x years, xx months, x weeks, x days, xx hours, xx minutes, xx seconds(null)"
	sprintf_s(outBuffer, "%s%s%s%s%s%s%s\x0", outYears, outMonths, outWeeks, outDays, outHours, outMins, outSecs);
	outBuf = std::string(outBuffer);
	return;
}

void Config::SetLastSeen(const char user[])
{
	time_t tNow;
	time(&tNow); //new time value we want to add

	std::string s_NewLineValue = "";
	s_NewLineValue = std::string(user);
	s_NewLineValue += LASTSEEN;
	//we need to find the line that the above key is on
	std::string str_out = "";
	int my_index = get_value_from_file(s_NewLineValue.c_str(), str_out, std::to_string(tNow).c_str());
	if (str_out == std::to_string(tNow)) { return; } //inital value didnt exist so it got added via function above.
	s_NewLineValue += std::to_string(tNow);
	delete_line_in_file(my_index, s_NewLineValue);
}

void Config::SetString(const char key_search[], const char strValue[])
{
	std::string str_out = "";
	std::string s_NewLineValue = "";
	std::string s_valueIn = std::string(strValue);
	s_NewLineValue = std::string(key_search);
	//s_NewLineValue += "=";
	int my_index = get_value_from_file(s_NewLineValue.c_str(), str_out, s_valueIn.c_str());
	if (str_out == s_valueIn) { return; } //inital value didnt exist so it got added via function above.
	s_NewLineValue += s_valueIn;
	delete_line_in_file(my_index, s_NewLineValue);
}

void Config::SetHexInt32(const char key_search[], UINT32 dwValue)
{
	//convert value to hex
	std::stringstream ss_Stream;
	ss_Stream << std::right << std::setfill('0') << std::setw(8) << std::hex << dwValue;
	std::string value(ss_Stream.str());
	//Set the value in the config
	std::string str_out = "";
	std::string s_NewLineValue = "";
	s_NewLineValue = std::string(key_search);
	//s_NewLineValue += "=";
	int my_index = get_value_from_file(s_NewLineValue.c_str(), str_out, value.c_str());
	if (str_out == value) { return; } //inital value didnt exist so it got added via function above. [and or it already existed in the config as this value]
	s_NewLineValue += value;
	delete_line_in_file(my_index, s_NewLineValue);
}

void Config::SetInteger32(const char key_search[], UINT32 dwValue)
{
	std::string str_out = "";
	std::string s_dwValue = std::to_string(dwValue);
	std::string s_NewKeyLine = std::string(key_search);
	int my_index = get_value_from_file(s_NewKeyLine.c_str(), str_out, s_dwValue.c_str());
	if (str_out == s_dwValue) { return; } //inital value didnt exist so it got added via function above. [and or it already existed in the config as this value]
	s_NewKeyLine += s_dwValue;
	delete_line_in_file(my_index, s_NewKeyLine);
}

void Config::delete_line_in_file(int line_number, std::string replace_value)
{
	SCrit.enter();
	std::istringstream temp_buffer(m_filedata);
	std::string buffer = "";
	std::string line = "";
	UINT32 i_Index = 0;
	int i_CR = 0;
	while (std::getline(temp_buffer, line))
	{
		i_CR = (int)line.find("\r", 1); //Windows GetLine will still return "\r" in the string.
		if (i_CR) {
			if (i_CR == 1)
			{
				line = ""; //Remove the empty line carrage return.
			}
			else {
				line = line.substr(0, i_CR);
			}
		} //else linex = linex.
		
		if (i_Index == line_number)
		{
			if (replace_value == "") {
				//if the value is empty skip this.
			}
			else {
				buffer += replace_value;
				buffer += "\r\n";
			}
		}
		else {
			if (line.length() > 0) {
				buffer += line;
				buffer += "\r\n";
			}
		}
		i_Index++;
	}
	//send buffer to be saved.
	m_filedata = buffer;
	SCrit.leave();
	Save();

	return;
}

void Config::Save()
{
	SCrit.enter();
	FILE *stream;
	fopen_s(&stream, m_configfile.c_str(), "wt");
	if (stream) {
		fprintf(stream, "%s", m_filedata.c_str());
	}
	fclose(stream);
	SCrit.leave();
}
