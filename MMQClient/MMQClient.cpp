//
//  Hello World server in C++
//  Binds REP socket to tcp://*:5555
//  Expects "Hello" from client, replies with "World"
//
#include <zmq.hpp>
#include <string>
#include <iostream>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>

#define sleep(n)    Sleep(n)
#endif


class SendMessageTest
{
public:
    void StartRequestRespose(std::string msg)
    {
        zmq::message_t request;
        if (socket->connected())
        {
            auto l = socket->send(zmq::const_buffer(msg.c_str(), msg.size()), zmq::send_flags::dontwait);
            socket->recv(request);
        }
    }

    void Init()
    {
        context.setctxopt(1, 1);
        socket = std::make_unique< zmq::socket_t>(context, ZMQ_REQ);
        socket->connect("tcp://127.0.0.1:5556");
    }


private:
    zmq::context_t context;
    std::unique_ptr<zmq::socket_t> socket;// (context, ZMQ_REQ);
};
int main() {
    SendMessageTest sm;
    sm.Init();
    while(true)
    sm.StartRequestRespose(std::string("Hello World"));
    return 0;
}

