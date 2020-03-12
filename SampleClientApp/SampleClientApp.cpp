// SampleClientApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <TSLogger.h>
int main()
{
	std::string s;
	{
		ExternalLog::CLoggerUtility log("OX");
		ExternalLog::CLoggerUtility log1("OX");
		log.LogMessage(1, "Hello OX 1");
		log1.LogMessage(1, "Hello OX 2");
		std::cout << "Press any key to continue\n";
		
		std::getline(std::cin, s);
	}
	std::getline(std::cin, s);
	{
		ExternalLog::CLoggerUtility log3 ("OX");
		log3.LogMessage(1, "Hello OX 3");
	}

	int count = 0;
	
	{
		ExternalLog::CLoggerUtility Oxlogger("OX");
		ExternalLog::CLoggerUtility logger("Chart");
		while (true)
		{
			std::stringstream ss;
			ss << " How are you ? " << count++ << " Process ID : " << GetCurrentProcessId();
			logger.LogMessage(1, ss.str());
			Oxlogger.LogMessage(1, " Hello from OX 4..");
			{
				ExternalLog::CLoggerUtility Oxlogger1("OX");
				Oxlogger1.LogMessage(1, "New Log for OX-5++");
			}
			std::cout << "more log ?";
			std::string response;
			std::getline(std::cin, response);
			if (response == "no")
				break;
		}
	}
	std::cout << "Hello World!\n";
	std::getchar();
}

