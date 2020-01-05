//
//  Hello World server in C++
//  Binds REP socket to tcp://*:5555
//  Expects "Hello" from client, replies with "World"
//
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <codecvt>

#include <map>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>

#define sleep(n)    Sleep(n)
#endif

class Command
{
public:
	static std::string StartCommnad(int processID)
	{
		std::ostringstream strm;
		strm << "{\"Action\":0,\"Parameters\":{\"ProcessId\":\"";
		strm << processID << "\"}}";

		return strm.str();
	}

	static std::string StopCommnad(int processID)
	{
		std::ostringstream strm;
		strm << "{\"Action\":1, \"Parameters\" : {\"ProcessId\":\"" << processID << "\"}}";
		return strm.str();
	}

	static std::string LogCommnad(std::string appID, std::string message)
	{
		std::ostringstream strm;
		strm << "{\"Action\":2, \"Parameters\" : {\"AppName\":\"" << appID << "\", \"Message\" : \"" << message << "\"}}";
		return strm.str();
	}
};


class CLogger
{
    void SendMessage(std::string msg)
    {
        zmq::message_t request;
        if (socket->connected())
        {
            auto l = socket->send(zmq::const_buffer(msg.c_str(), msg.size()), zmq::send_flags::dontwait);
			
			zmq::pollitem_t items[] = {
			  { *socket, 0, ZMQ_POLLIN, 0 } };
			if (zmq::poll(items, 1, 1000))
			{
				socket->recv(request, zmq::recv_flags::none);
			}
			else
			{
				PendingMessages.push_back(msg);
				TerminateConnection();
				Connect();
			}
        }
    }

    void Init()
    {
		StartServer();
        context.setctxopt(1, 1);
        socket = std::make_unique< zmq::socket_t>(context, ZMQ_REQ);
        socket->connect(address.c_str());
    }
	void StartServer()
	{
		if (!IsLoggerServerRunning())
		{
			STARTUPINFO si;
			PROCESS_INFORMATION pi;

			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			ZeroMemory(&pi, sizeof(pi));
			
			// Start the child process. 
			CreateProcess(LoggerServerPath.c_str(),   // No module name (use command line)
				NULL,        // Command line
				NULL,           // Process handle not inheritable
				NULL,           // Thread handle not inheritable
				FALSE,          // Set handle inheritance to FALSE
				CREATE_NEW_CONSOLE,              // New console
				NULL,           // Use parent's environment block
				NULL,           // Use parent's starting directory 
				&si,            // Pointer to STARTUPINFO structure
				&pi);           // Pointer to PROCESS_INFORMATION structure
				
		}
	}
	bool IsLoggerServerRunning()
	{
		return IsProcessRunning(LoggerServerName);
	}
	bool IsProcessRunning(std::string processName)
	{
		HANDLE hProcessSnap;
		PROCESSENTRY32 pe32;
		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		// Set the size of the structure before using it.
		pe32.dwSize = sizeof(PROCESSENTRY32);

		// Now walk the snapshot of processes, and
		// display information about each process in turn
		std::wstring_convert< std::codecvt_utf8<wchar_t>, wchar_t > cvt;
		bool Found(false);
		do
		{
			auto process = cvt.to_bytes( std::wstring(pe32.szExeFile));
			if (process == processName)
			{
				Found = true;
				break;
			}
		} while (Process32Next(hProcessSnap, &pe32));

		CloseHandle(hProcessSnap);
		return(Found);
	}
	void TerminateConnection()
	{
		if(socket != nullptr && IsLoggerServerRunning())
			socket->disconnect(address.c_str());
		socket = nullptr;
	}
	void FlushQueuedMessage()
	{
		for (auto m : PendingMessages)
		{
			SendMessage(m);
		}
		PendingMessages.clear();
	}
public:
	void Connect()
	{
		if (socket == nullptr)
		{
			Init();
			SendMessage(Command::StartCommnad(GetCurrentProcessId()));
		}
		FlushQueuedMessage();
	}
	void Disconnect()
	{
		SendMessage(Command::StopCommnad(GetCurrentProcessId()));
		TerminateConnection();
	}
	void LogMessage(std::string appID, long level, std::string message)
	{
		if (socket == nullptr)
			Connect();
		SendMessage(Command::LogCommnad(appID, message));
	}

private:
	const std::string address = "tcp://127.0.0.1:5556";
	const std::string LoggerServerName = "MMQServer.exe";
	const std::wstring LoggerServerPath = L"C:\\myPOC\\ZEROMQ\\MMQServer\\bin\\Debug\\MMQServer.exe";
    zmq::context_t context;
    std::unique_ptr<zmq::socket_t> socket;// (context, ZMQ_REQ);
	std::vector<std::string> PendingMessages;
};

int main() {
	std::unique_ptr<CLogger> logger(std::make_unique<CLogger>());
	int count(0);
	while (true)
	{
		std::stringstream ss;
		ss << " How are you ? " << count++ << " Process ID : "<< GetCurrentProcessId();
		logger->LogMessage("Chart", 1, ss.str());
		std::cout << "more log ?";
		std::string response;
		std::getline(std::cin, response);
		if (response == "no")
			break;
	}
	logger->Disconnect();
	logger = nullptr;
	return 0;
}

