using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using NetMQ;
using NetMQ.Sockets;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace MMQClientNet
{
    class Command
    {
        public enum ActionKind
        {
            Start,
            Stop,
            Log
        }
        public enum ParameterName
        {
            AppName,
            ProcessId,
            Message
        }
        public ActionKind Action { get; set; }
        public Dictionary<string, string> Parameters { get; set; }
    
        public string GetJson()
        {
            return JsonSerializer.Serialize(this);
        }

        static Command CreateCommand(string json)
        {
            return JsonSerializer.Deserialize<Command>(json);
        }

    }
    class Program
    {
        private const string Address = "tcp://127.0.0.1:5556";
        private static RequestSocket loggerServer = null;
        private static List<Command> pendingCommand = null;
        private static Process ServerProces = null;

        private static bool Conneceted { get; set; }

        static void Main(string[] args)
        {
            while (true)
            {
                LogMe("Test", 1, "how are you ?");
                Console.WriteLine("more log ?");
                var input = Console.ReadLine();
                if (input == "no")
                    break;
            }
            StopLogger();
        }

        private static void Connect()
        {
            if(loggerServer == null)
            {
                var process = Process.GetProcessesByName("MMQServer");
                ServerProces = (process.Length > 0) ? process[0] : null;
                if(ServerProces == null)
                {
                    Console.WriteLine("MMQServer.exe is stopped");
                    ServerProces = Process.Start(@"C:\myPOC\ZEROMQ\MMQServer\bin\Debug\MMQServer.exe");
                }

                loggerServer = new RequestSocket();
                loggerServer.Connect(Address);

                var cmd = new Command() { Action = Command.ActionKind.Start };
                cmd.Parameters = new Dictionary<string, string>();
                
                cmd.Parameters[Command.ParameterName.ProcessId.ToString()] = Process.GetCurrentProcess().Id.ToString();

                SendLogAsync(cmd);

                Conneceted = true;
                SendPendingCommand();
            }
        }

        private static void SendPendingCommand()
        {
            if(pendingCommand != null)
            {
                foreach (var cmd in pendingCommand)
                    SendLogAsync(cmd);
            }
        }

        private static void SendLogAsync(Command cmd)
        {
            if (loggerServer == null)
            {
                if (pendingCommand == null)
                    pendingCommand = new List<Command>();
                pendingCommand.Add(cmd);
            }
            else
            {
                loggerServer.SendFrame(cmd.GetJson());
                if (!loggerServer.TryReceiveFrameString(TimeSpan.FromSeconds(40), out string response))
                {
                    Disconnect();
                    Connect();
                    SendLogAsync(cmd);
                }
            }
        }

        private static void Disconnect()
        {
            loggerServer.Disconnect(Address);
            loggerServer = null;
            Conneceted = false;
        }


        public static void LogMe(string appID, long level, string message)
        {
            if (!Conneceted)
            {
                Connect();
            }
            var cmd = new Command() { Action = Command.ActionKind.Log };
            cmd.Parameters = new Dictionary<string, string>();
            cmd.Parameters[Command.ParameterName.AppName.ToString()] =  appID;
            cmd.Parameters[Command.ParameterName.Message.ToString()] =  message + Process.GetCurrentProcess().Id;

            SendLogAsync(cmd);
        }
        public static void StopLogger()
        {
            var cmd = new Command() { Action = Command.ActionKind.Stop };
            cmd.Parameters = new Dictionary<string, string>();

            cmd.Parameters[Command.ParameterName.ProcessId.ToString()] = Process.GetCurrentProcess().Id.ToString();

            SendLogAsync(cmd);
            Disconnect();
        }
    }
}
