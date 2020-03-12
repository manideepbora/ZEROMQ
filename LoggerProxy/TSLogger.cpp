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

std::mutex CLogger::Mutex;
std::atomic_int CLogger::LoggerCount = 0;
void CLogger::QueueLogMessage(const std::string& msg)
{
	if (!started_)
	{
		const std::lock_guard<std::mutex> lock2(pending_messages_mutux_);
		pending_messages_.push_back(msg);
		return;
	}
	FlushQueuedMessage();
	SendLogMessage(msg);
} 


void CLogger::SendLogMessage(const std::string& msg)
{
	const std::lock_guard<std::mutex> lock(Mutex);
	zmq::message_t request;
	if ((socket_ != nullptr) && (socket_->connected()))
	{
		auto l = socket_->send(zmq::const_buffer(msg.c_str(), msg.size()), zmq::send_flags::dontwait);

		zmq::pollitem_t items[] = {
		  { *socket_, 0, ZMQ_POLLIN, 0 } };
		if (zmq::poll(items, 1, 10000))
			socket_->recv(request, zmq::recv_flags::none);
		else
		{
			{
				const std::lock_guard<std::mutex> lock2(pending_messages_mutux_);
				pending_messages_.push_back(msg);
			}
			TerminateConnection();
			Connect();
		}
	}

	const std::lock_guard<std::mutex> lock2(message_q_mutux_);

	auto proceed (false);
	do
	{
		if (queue_message_.size() == 0)
			break;
		auto result = queue_message_.front().wait_for(std::chrono::seconds(0));
		proceed = (result == std::future_status::ready);
		if(proceed)
			queue_message_.pop();
	} while (proceed);
}
void CLogger::Init()
{
	const std::lock_guard<std::mutex> lock(Mutex);
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
	started_ = true;
	stopped_ = false;
}
bool CLogger::IsLoggerServerRunning()
{
	if (IsProcessRunning(LoggerServerName))
	{
		stopped_ = false;
		started_ = true;

		return true;
	}
	else
	{
		stopped_ = true;
		started_ = false;
		return false;
	}
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
	const std::lock_guard<std::mutex> lock(Mutex);
	started_ = false;
	stopped_ = true;
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
	const std::lock_guard<std::mutex> lock2(pending_messages_mutux_);
	pending_messages_.clear();
}
bool CLogger::IsSocketEmpty() { return socket_ == nullptr; }

//Started the server and flush all queued message
void CLogger::Connect()
{
	//std::cout << "Connect\n";
	
	if (socket_ == nullptr || !started_)
	{
		Init();
		
		if (stopped_)
		{
			TerminateConnection();
		}
		pending_futures_++;
		SendLogMessage(Command::StartCommnad(name_, GetCurrentProcessId()));
	}
	
	if(started_)
		FlushQueuedMessage();
}

void CLogger::Disconnect( )
{
	//if (!started_ || pending_futures_ != 0 )
	//{
	//	auto self = shared_from_this();

	//	queue_message_.push(std::move(std::async([self, this]
	//	{
	//		std::this_thread::sleep_for(std::chrono::seconds{ 1 });
	//		Disconnect();
	//	})));

	//	return;
	//}
	//if (started_ /*&& LoggerCount-- == 1*/)
	{
		pending_futures_++;
		SendLogMessage(Command::StopCommnad(name_, GetCurrentProcessId()));
		TerminateConnection();
	}
}

void CLogger::LogMessage(long level, const std::string& message, bool bReEnter)
{
	if (!bReEnter)
	{
		//std::cout << "LogMessage before queue " << message << '\n';
		auto self = shared_from_this();
		const std::lock_guard<std::mutex> lock(message_q_mutux_);
		pending_futures_++;
		queue_message_.push(std::move(std::async([self, this, level, message] 
			{
				LogMessage(level, message, true); 
			})));
		return;
	}
	
	if (!started_)
	{
		//std::cout << "Execute on different thread == CONNECT\n";
		Connect();
	}
	QueueLogMessage(Command::LogCommnad(name_, message));

}


std::shared_ptr<CLogManager> CLogManager::LogManager;