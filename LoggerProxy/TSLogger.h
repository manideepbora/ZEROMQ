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
		static std::string StartCommnad(int processID);

		static std::string StopCommnad(int processID);

		static std::string LogCommnad(std::string appID, std::string message);
	};

	class LOGGER_CLASS CLogger
	{
		void SendLogMessage(std::string msg);
		void Init();
		void StartServer();
		bool IsLoggerServerRunning();
		bool IsProcessRunning(std::string processName);
		void TerminateConnection();
		void FlushQueuedMessage();
		bool IsSocketEmpty();
	public:
		void Connect();
		void Disconnect();
		void LogMessage(std::string appID, long level, std::string message, bool bReEnter = false);

	private:

		const std::string address = "tcp://127.0.0.1:5556";
		const std::string LoggerServerName = "MMQServer.exe";
		const std::wstring LoggerServerPath = L"..\\MMQServer\\bin\\Debug\\MMQServer.exe";
		zmq::context_t context;
		std::unique_ptr<zmq::socket_t> socket;// (context, ZMQ_REQ);
		std::vector<std::string> PendingMessages;
		std::queue <std::future<void> > q;
	};
}