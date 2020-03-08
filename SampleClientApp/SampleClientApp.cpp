// SampleClientApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <TSLogger.h>
int main()
{
	{
		ExternalLog::CLoggerUtility log("OX");
		log.LogMessage(1, "Hello OX 1");
		log.LogMessage(1, "Hello OX 2");
	}

	//auto logger = ExternalLog::CLogManager::GetLogManager()->GetLogger("Chart");
	//logger->Connect();
	//auto Oxlogger = ExternalLog::CLogManager::GetLogManager()->GetLogger("OX");
	//Oxlogger->Connect();
	int count = 0;
	
	ExternalLog::CLoggerUtility Oxlogger("OX");
	ExternalLog::CLoggerUtility logger("Chart");
	while (true)
	{
		std::stringstream ss;
		ss << " How are you ? " << count++ << " Process ID : " << GetCurrentProcessId();
		logger.LogMessage(1, ss.str());
		Oxlogger.LogMessage(1, " Hello from OX 3");

		{
			ExternalLog::CLoggerUtility Oxlogger1("OX");
			Oxlogger1.LogMessage(1, "New Log for OX-4");
		}
		std::cout << "more log ?";
		std::string response;
		std::getline(std::cin, response);


		if (response == "no")
			break;
	}
	//ExternalLog::CLogManager::GetLogManager()->ClearManager();
	std::cout << "Hello World!\n";
}

