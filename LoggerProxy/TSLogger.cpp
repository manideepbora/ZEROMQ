#include "pch.h"
#include "TSLogger.h"
using namespace ExternalLog;

std::string Command::StartCommnad(const std::string& log_name, int processID)
{
	std::ostringstream strm;
	strm << "{\"Action\":0,\"Parameters\":{\"AppName\":\"" << log_name << "\", \"ProcessId\":\"" << processID << "\"}}";

	return strm.str();
}

std::string Command::StopCommnad(const std::string& log_name, int processID)
{
	std::ostringstream strm;
	strm << "{\"Action\":1, \"Parameters\" : {\"AppName\":\"" << log_name << "\", \"ProcessId\":\"" << processID << "\"}}";
	return strm.str();
}

std::string Command::LogCommnad(const std::string& appID, const std::string& message)
{
	std::ostringstream strm;
	strm << "{\"Action\":2, \"Parameters\" : {\"AppName\":\"" << appID << "\", \"Message\" : \"" << message << "\"}}";
	return strm.str();
}

//==================================================
//
//==================================================


void CLogger::SendLogMessage(const std::string& msg)
{
	zmq::message_t request;
	if (socket_->connected())
	{
		auto l = socket_->send(zmq::const_buffer(msg.c_str(), msg.size()), zmq::send_flags::dontwait);

		zmq::pollitem_t items[] = {
		  { *socket_, 0, ZMQ_POLLIN, 0 } };
		if (zmq::poll(items, 1, 10000))
		{
			socket_->recv(request, zmq::recv_flags::none);
		}
		else
		{
			pending_messages_.push_back(msg);
			TerminateConnection();
			Connect();
		}
	}
}
void CLogger::Init()
{
	StartServer();
	context_.setctxopt(1, 1);
	socket_ = std::make_unique< zmq::socket_t>(context_, ZMQ_REQ);
	socket_->connect(Address.c_str());
}
void CLogger::StartServer()
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
bool CLogger::IsLoggerServerRunning()
{
	return IsProcessRunning(LoggerServerName);
}
bool CLogger::IsProcessRunning(std::string processName)
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
		auto process = cvt.to_bytes(std::wstring(pe32.szExeFile));
		if (process == processName)
		{
			Found = true;
			break;
		}
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return(Found);
}
void CLogger::TerminateConnection()
{
	if (socket_ != nullptr && IsLoggerServerRunning())
		socket_->disconnect(Address.c_str());
	socket_ = nullptr;
}
void CLogger::FlushQueuedMessage()
{
	for (auto m : pending_messages_)
	{
		SendLogMessage(m);
	}
	pending_messages_.clear();
}
bool CLogger::IsSocketEmpty() { return socket_ == nullptr; }

void CLogger::Connect()
{
	if (socket_ == nullptr)
	{
		Init();
		SendLogMessage(Command::StartCommnad(name_, GetCurrentProcessId()));
	}
	FlushQueuedMessage();
}

void CLogger::Disconnect()
{
	SendLogMessage(Command::StopCommnad(name_, GetCurrentProcessId()));
	TerminateConnection();
}

void CLogger::LogMessage(long level, const std::string& message, bool bReEnter)
{
	if (!bReEnter)
	{
		queue_message_.push(std::move(std::async([this, level, message] {LogMessage(level, message, true); })));
		return;
	}
	if (IsSocketEmpty())
		Connect();
	SendLogMessage(Command::LogCommnad(name_, message));
	queue_message_.pop();
}