// SampleClientApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <TSLogger.h>
int main()
{
	ExternalLog::CLogger logger("Chart");
	logger.Connect();

	ExternalLog::CLogger Oxlogger("OX");
	Oxlogger.Connect();

	int count = 0;
	
	while (true)
	{
		std::stringstream ss;
		ss << " How are you ? " << count++ << " Process ID : " << GetCurrentProcessId();
		logger.LogMessage(1, ss.str());
		Oxlogger.LogMessage(1, " Hello from OX");
		std::cout << "more log ?";
		std::string response;
		std::getline(std::cin, response);
		if (response == "no")
			break;
	}

	std::cout << "Hello World!\n";
}

