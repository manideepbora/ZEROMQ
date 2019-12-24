using NetMQ;
using NetMQ.Sockets;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MMQServer
{
    class Program
    {
        static void Main(string[] args)
        {
            ReqResponse();
            return;
            //Subscriber();
        }

        private static void ReqResponse()
        {
            using (var server = new ResponseSocket())
            {

                server.Bind("tcp://*:5556");
                while (true)
                {
                    string msg = server.ReceiveFrameString();
                    Console.WriteLine("From Client: {0}", msg);
                    server.SendFrame("World");
                }

            }
        }

        private static void Subscriber()
        {
            using (var subscriber = new SubscriberSocket())
            {
                subscriber.Connect("tcp://127.0.0.1:5556");
                subscriber.Subscribe("");

                while (true)
                {
                    var topic = subscriber.ReceiveFrameString();
                    var msg = subscriber.ReceiveFrameString();
                    Console.WriteLine("From Publisher: {0} {1}", topic, msg);
                }
            }
        }
    }
}
