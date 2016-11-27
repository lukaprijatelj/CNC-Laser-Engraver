using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ConsoleUI
{
    class Program
    {
        static string _portName;
        static string _imagePath;

        static string _logPath = @".\log.txt";

        static SerialPortInterface _spi;
        static PointSequence.PointSequence _ps;
        static int index = 0;

        static void Main(string[] args)
        {
            GetConfigurations();
            LogClear();

            _ps = new PointSequence.PointSequence(_imagePath);

            _spi = new SerialPortInterface(_portName);
            _spi.OnRTS += _spi_OnRTS;

            Console.WriteLine("Press key to start...");
            Console.ReadKey();

            _spi.Write("S");

            Console.WriteLine("STARTED");

            while (_ps.Count > 0) ;
            Console.ReadKey();
        }

        // Recieved request from STM for new point
        private static void _spi_OnRTS()
        {
            if (index < _ps.Count)
            {
                Point point = _ps.ElementAt<Point>(index);
                LogWrite(point.X, point.Y);
                _spi.Write(point.X.ToString());
                _spi.Write(point.Y.ToString());
                Console.WriteLine("Write: " + point.ToString());
                index++;
            }
            else
            {
                _spi.Write("D");
                Console.WriteLine("DONE");
            }
        }

        static void GetConfigurations()
        {

#if DEBUG
            _portName = "COM3";
            _imagePath = @".\cut.png";
#else
            Console.WriteLine("Available port names: " + string.Join(", ", SerialPort.GetPortNames()));
            Console.Write("Port name: ");
            while(string.IsNullOrEmpty(_portName))
            {
                _portName = Console.ReadLine();
                if(!SerialPort.GetPortNames().Contains(_portName))
                {
                    _portName = null;
                }
            }

            Console.Write(@"Image path (.\cut1.png): ");
            while (string.IsNullOrEmpty(_imagePath))
            {
                _imagePath = Console.ReadLine();
                if(!File.Exists(_imagePath))
                {
                    _imagePath = null;
                }
            }
#endif

            Console.Clear();
            Console.WriteLine("------------------------------------------------------------------");
            Console.WriteLine("Selected COM port:\t" + _portName);
            Console.WriteLine("Selected image:\t\t" + _imagePath);
            Console.WriteLine("------------------------------------------------------------------");
        }

        static void LogClear()
        {
            File.WriteAllText(_logPath, string.Empty);
        }

        static void LogWrite(int x, int y)
        {
            File.AppendAllText(_logPath, x + " " + y + Environment.NewLine);
        }
    }
}
