# Purpose
These are sample projects to use ZeroMQ RPC model to communicate the between two process. The server process is built using .net and has two client applications one in C++ and other in C#.
The application is developed as an alternative to COM projects, including auto creation as well as object’s life management policy.

# Repository's Structure
* MMQServer- is the centralized singleton server which listen to multiple clients. Clients sends various log messages and server saved them using netlog utility. The project is built in managed code (C#)
* MMQClientNet- it is managed client to communicate with the Server. 
* MMQClient- Is a standalone unmanaged console ( C++) client application.
* LoggerProxy- un managed (C++) dll. It is very light weight dll and will be used by various client applications.
* SampleClientApp- un managed (C++) client application. It uses the LoggerProxy dll to communicate with the server.

# Steps to build
* Setup up vcpkg ( clone it from github and run bootstrap-vcpkg.bat)
* Get ZeroMQ using ‘vcpkg install cppzmq’
* Modify BuildVariables.props file according to the directory stucture
* Get the repository and modify the LoggerServerPath variable accoring to the MMQServer projet's output folder
* Compile all projects and run any of these client application (there are three, one C#, one standalone C++, one C++ using dll)

#How to use the logger:
* In Native C++ project, take a look at SampleClientApp Project
* Create the ExternalLog::CLoggerUtility object as the following code sinppet and call LogMessage on that object:
    log1("OX");
		log.LogMessage(1, "Hello OX 1");
