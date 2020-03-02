#pragma once
#ifdef LOGGERPROXY_EXPORTS
#define LOGGER_CLASS __declspec(dllexport)
#else
#define LOGGER_CLASS __declspec(dllimport)
#endif

#include <zmq.hpp>
#include <string>
#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <codecvt>
#include <future>
#include <queue>

#include <map>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>

#define sleep(n)    Sleep(n)
#endif

namespace ExternalLog
{
	class Command
	{
	public:
		static std::string StartCommnad(const std::string& log_name, int processID);

		static std::string StopCommnad(const std::string& log_name, int processID);

		static std::string LogCommnad(const std::string& appID, const std::string& message);
	};

	class LOGGER_CLASS CLogger
	{
		void SendLogMessage(const std::string& msg);
		void Init();
		void StartServer();
		bool IsLoggerServerRunning();
		bool IsProcessRunning(std::string processName);
		void TerminateConnection();
		void FlushQueuedMessage();
		bool IsSocketEmpty();
	public:
		CLogger(const std::string& name)
			:name_(name)
		{

		}
		void Connect();
		void Disconnect();
		void LogMessage(long level, const std::string& message, bool bReEnter = false);

	private:
		const std::string Address = "tcp://127.0.0.1:5556";
		const std::string LoggerServerName = "MMQServer.exe";
		const std::wstring LoggerServerPath = L"..\\MMQServer\\bin\\Debug\\MMQServer.exe";
		zmq::context_t context_;
		std::unique_ptr<zmq::socket_t> socket_;// (context, ZMQ_REQ);
		std::vector<std::string> pending_messages_;
		std::queue <std::future<void> > queue_message_;
		std::string name_;
	};
}