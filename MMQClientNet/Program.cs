using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using NetMQ;
using NetMQ.Sockets;

namespace MMQClientNet
{
    class Program
    {
        static void Main(string[] args)
        {

            // using (var client = new RequestSocket())
            //using (var client = new PublisherSocket())
            //{
            //    client.Connect("tcp://127.0.0.1:5556");
            //    for (int n = 0; n < 10000; n++)
            //    {
            //        client.SendFrame("Hello from .net");
            //        //var msg = client.ReceiveFrameString();
            //       // Console.WriteLine("From Server: {0}", msg);
            //    }

            //}


            using (var publisher = new PublisherSocket())
            {
                publisher.Bind("tcp://*:5556");

                int i = 0;

                while (true)
                {
                    publisher
                        .SendMoreFrame("") // Topic
                        .SendFrame(i.ToString()); // Message

                    i++;
                    Thread.Sleep(1000);
                }
            }
        }
    }
}
