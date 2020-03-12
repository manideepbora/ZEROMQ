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

	class LOGGER_CLASS CLogger : public std::enable_shared_from_this<CLogger>
	{
		friend class CLoggerUtility;
		friend class CLogManager;

		void QueueLogMessage(const std::string& msg);
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
	public:
		void LogMessage(long level, const std::string& message, bool bReEnter = false);

	private:
		const std::string Address = "tcp://127.0.0.1:5556";
		const std::string LoggerServerName = "MMQServer.exe";
		const std::wstring LoggerServerPath = L"..\\MMQServer\\bin\\Debug\\MMQServer.exe";
		zmq::context_t context_;
		std::unique_ptr<zmq::socket_t> socket_;// (context, ZMQ_REQ);
		std::mutex pending_messages_mutux_;
		std::vector<std::string> pending_messages_;
		std::queue <std::future<void> > queue_message_;
		std::string name_;
		static std::mutex Mutex;
		std::mutex message_q_mutux_;
		static std::atomic_int LoggerCount;
		std::atomic_bool started_ = false;
		std::atomic_bool stopped_ = false;
		std::atomic_int pending_futures_ = 0;
	};

	class LOGGER_CLASS CLogManager
	{
		friend class CLogger;
	public:
		std::shared_ptr<CLogger> GetLogger(const std::string& name)
		{
			auto logPair = logs_.find(name);
			if (logPair == logs_.end())
			{
				logPair = logs_.insert(std::make_pair( name, std::make_pair(0, std::make_shared<CLogger>(name)))).first;
			}
			logPair->second.first++;
			return logPair->second.second;
		}

		bool RemoveLogger(const std::string& name)
		{
			auto logPair = logs_.find(name);
			if (logPair != logs_.end())
			{
				logPair->second.first--;
				if (logPair->second.first == 0)
				{
					logPair->second.second->Disconnect();
					logPair = logs_.erase(logPair);
					return true;
				}
			}
			return false;
		}

		static std::shared_ptr< CLogManager> GetLogManager()
		{
			if (!LogManager)
			{
				LogManager.reset(new CLogManager());
			}
			return LogManager;
		}

	private:
		static std::shared_ptr<CLogManager> LogManager;
		std::map<std::string, std::pair<int,  std::shared_ptr<CLogger>>> logs_;
	};

	class LOGGER_CLASS CLoggerUtility
	{
		
	public:
		CLoggerUtility(const std::string& name )
			:name_(name)
		{
			logger_ = CLogManager::GetLogManager()->GetLogger(name_);
		}
		~CLoggerUtility()
		{
			if (logger_)
				CLogManager::GetLogManager()->RemoveLogger(name_);
		}
		void LogMessage(long level, const std::string& message)
		{
			if (!logger_)
			{
				logger_ = CLogManager::GetLogManager()->GetLogger(name_);
			}
			logger_->LogMessage( level, message);
		}
		void Close()
		{
			if (logger_)
			{
				//logger_->Disconnect();
			}
			
		}

	private:
		std::shared_ptr<CLogger> logger_ = nullptr;
		std::string name_;
	};
}

