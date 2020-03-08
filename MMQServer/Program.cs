using NetMQ;
using NetMQ.Sockets;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Threading;
using System.Threading.Tasks;

namespace MMQServer
{
    class Program
    {

        static void Main(string[] args)
        {
            using (LoggerService logService_ = new LoggerService())
                logService_.Start();
            //ReqResponse();
            //return;
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

    class Subscriptions
    {
        private Dictionary<int, Process> Processes = new Dictionary<int, Process>();
        private readonly object _obj = new object();

        public bool ActiveMonitor { get; set; }
        public bool IsListenerPresent() { lock (_obj) { return Processes.Count > 0; } }

        public void AddListener(int id)
        {
            lock (_obj)
            {
                var process = Process.GetProcessById(id);
                process.EnableRaisingEvents = true;
                process.Exited += Process_Exited;
                if(!Processes.ContainsKey(id))
                    Processes.Add(id, process);
            }
        }

        private void Process_Exited(object sender, EventArgs e)
        {
            var p = sender as Process;
            Processes.Remove(p.Id);
        }

        public void RemoveListener(int id)
        {
            lock (_obj)
            {
                Processes.Remove(id);
            }
        }

   
    }

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

        public static Command CreateCommand(string json)
        {
            return JsonSerializer.Deserialize<Command>(json);
        }

    }
    public class LoggerService : System.IDisposable
    {
        private const string Address = "tcp://127.0.0.1:5556";
        private Subscriptions sub = new Subscriptions();
        private ResponseSocket socket = null;
        private bool disposed = false;
        private static readonly NLog.Logger Logger = NLog.LogManager.GetCurrentClassLogger();

        private static Dictionary<string, NLog.Logger> loggers = new Dictionary<string, NLog.Logger>();

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (disposed)
                return;

            if (disposing)
            {
                socket.Dispose();
            }
            disposed = true;
        }

        public void Start()
        {
            sub.ActiveMonitor = true;
            //Thread montitorThread = new Thread(sub.MontitorLister);
            while (socket == null || sub.IsListenerPresent())
            {
                if (socket == null)
                {
                    socket = new ResponseSocket();
                    socket.Bind(Address);
                    //montitorThread.Start();
                }
                if (socket.TryReceiveFrameString(TimeSpan.FromSeconds(20), out string msg))
                {
                    if (!ProcessMessage(msg))
                        break;
                }
            }
            sub.ActiveMonitor = false;
            socket.Unbind(Address);
            socket.Dispose();
            
        }

        private bool ProcessMessage(string msg)
        {
            Command cmd = Command.CreateCommand(msg);
            bool returnval = true;
            switch (cmd.Action)
            {
                case Command.ActionKind.Start:
                    //Console.WriteLine("Start ProcessId: {0}", cmd.Parameters[Command.ParameterName.ProcessId.ToString()]);
                    sub.AddListener(Int32.Parse(cmd.Parameters[Command.ParameterName.ProcessId.ToString()]));
                    break;
                case Command.ActionKind.Log:
                    //Console.WriteLine("Log AppName: {0} Message{1}", cmd.Parameters[Command.ParameterName.AppName.ToString()], cmd.Parameters[Command.ParameterName.Message.ToString()]);
                    LogData(cmd);
                    break;
                case Command.ActionKind.Stop:
                    //Console.WriteLine("Stop ProcessId: {0}", cmd.Parameters[Command.ParameterName.ProcessId.ToString()]);
                    sub.RemoveListener(Int32.Parse(cmd.Parameters[Command.ParameterName.ProcessId.ToString()]));
                    if (!sub.IsListenerPresent())
                        returnval = false;
                    break;
            }
            socket.SendFrame("Received");
            return returnval;
        }
       // private static readonly NLog.Logger Logger1 = NLog.LogManager.GetCurrentClassLogger();
        private void LogData(Command cmd)
        {
            var appName = cmd.Parameters[Command.ParameterName.AppName.ToString()];

            NLog.Logger taskLogger;
            if (loggers.ContainsKey(appName))
                taskLogger = loggers[appName];
            else
            {
                taskLogger = NLog.LogManager.GetLogger(appName);
                loggers[appName] = taskLogger;
            }

            taskLogger.Error(cmd.Parameters[Command.ParameterName.Message.ToString()] + "*** " + cmd.Parameters[Command.ParameterName.AppName.ToString()]); 
        }
    }

}
