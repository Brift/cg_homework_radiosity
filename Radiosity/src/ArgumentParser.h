#ifndef ARGUMENTSPASER_H
#define ARGUMENTSPASER_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <exception>
#include <sstream>
#include <iterator>
#include <iomanip>
#include <algorithm>

class ArgumentsParseFailException : public std::exception
{
public:
	ArgumentsParseFailException(const char *errMsg) : exception(errMsg){}
};

class ArgumentsParser
{
	struct ArguInfo{
		bool m_bNecessary;
		std::string m_strValue;
		std::string m_strDescription;
	};
	std::string m_strProDescription;
	std::string m_strVersion;
	std::map<std::string, ArguInfo> m_mapArguments;
	std::string m_strDelimiter;
	int m_nNecessaryCount;

public:

	ArgumentsParser(void)
		:m_nNecessaryCount(0)
	{
		setDelimiter("--");
		setVersion("1.0");
		setProDescription("");
	}

	virtual ~ArgumentsParser(void)
	{
	}

	void addArguments(std::string description, std::string argName, bool bNessary = true,
		std::string defaultValue = "")
	{
		m_mapArguments[argName].m_strDescription = description;
		m_mapArguments[argName].m_strValue = "";
		if(bNessary)
			++m_nNecessaryCount;
		else
			m_mapArguments[argName].m_strValue = defaultValue;
		m_mapArguments[argName].m_bNecessary = bNessary;
	}

	void setProDescription(std::string description){
		m_strProDescription = description;
	}

	void setVersion(std::string version){
		m_strVersion = version;
	}

	void setDelimiter(std::string delimiter){
		m_strDelimiter = delimiter;
	}

	void showHelp();

	void parseArguments(int argc, char **argv) throw (ArgumentsParseFailException);

	template<typename T> T getValue(const std::string arguName);
};

template<typename T> T ArgumentsParser::getValue(const std::string arguName){
	auto iteFind = m_mapArguments.find(arguName);
	if(iteFind == m_mapArguments.end())
		throw ArgumentsParseFailException("Undefined Argument");

	T temp;
	std::istringstream istrstream(iteFind->second.m_strValue);
	if((istrstream >> temp).fail())
		throw ArgumentsParseFailException("Invalid Argument Value");

	return temp;
}


class RadiosityParserArguments : public ArgumentsParser
{
public:
	RadiosityParserArguments(){
		initialize();
	}

	std::string getFileName() throw(ArgumentsParseFailException){
		return getValue<std::string>("file");
	}

	bool getScreenShot() throw(ArgumentsParseFailException){
		std::string temp = getValue<std::string>("shot");
		if(temp == "1" || temp == "true")
			return 1;
		else if( temp == "0" || temp == "false")
			return 0;
		throw ArgumentsParseFailException("Para [screenShot] : Invalid value");
	}

	double getTriArea() throw(ArgumentsParseFailException){
		return getValue<double>("area");
	}

private:
	void initialize(){
		addArguments("Obj file name", "file");
		addArguments("Save ScreenShot", "shot", false, "false");
		addArguments("The Area of Triangle", "area", false, "0.001");
	}

	int a;
	float b;
};
#endif