#include "ArgumentParser.h"

void ArgumentsParser::parseArguments(int argc, char **argv) throw (ArgumentsParseFailException){

	if(argc == 2){
		if(std::string(argv[1]) == m_strDelimiter + "help"){
			showHelp();
			exit(0);
		}
	}

	if(argc < 1 + m_nNecessaryCount) {
		throw ArgumentsParseFailException("two few arugment");
		return;
	}

	for(int i = 1; i < argc; ++i){
		const std::string argument = argv[i];
		if(argument.size() <= m_strDelimiter.size())
			throw ArgumentsParseFailException("size Invalid argument");

		for(int index = 0; index < m_strDelimiter.size(); ++index){
			if(argument[index] != m_strDelimiter[index])
				throw ArgumentsParseFailException("Invalid delimiter");
		}

		size_t pos = argument.find("=", m_strDelimiter.size());
		if(pos == std::string::npos)
			throw ArgumentsParseFailException("Argument need to have =");

		const std::string arguName = argument.substr(m_strDelimiter.size(), pos - m_strDelimiter.size());
		const std::map<std::string, ArguInfo>::iterator iteFind = m_mapArguments.find(arguName);
		if(iteFind == m_mapArguments.end())
			throw ArgumentsParseFailException("Undefined Argument");

		const std::string arguValue = argument.substr(pos + 1, argument.size() - pos - 1);
		iteFind->second.m_strValue = arguValue;
	}
}

void ArgumentsParser::showHelp()
{
	std::cout << m_strProDescription << std::endl;

	std::cout << m_strDelimiter << std::left << std::setw(10) << "help" << ": " << "show help information" << std::endl;
	std::map<std::string, ArguInfo>::iterator ite = m_mapArguments.begin();
	for(; ite != m_mapArguments.end(); ++ite) {
		std::string defaultValue;
		if(ite->second.m_bNecessary)
			defaultValue = "Necessary";
		else
			defaultValue = std::string("Default Value : ") + ite->second.m_strValue;

		std::cout << m_strDelimiter << std::left << std::setw(10) << ite->first << ": " 
			<< ite->second.m_strDescription << "[" << defaultValue << "]" << std::endl;
	}
}
