using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ConsoleUI
{
    class SerialPortInterface : IDisposable
    {

        public event Action OnRTS;

        private SerialPort _serialPort;

        public SerialPortInterface(string portName, int baudRate = 9600, Parity parity = Parity.None, int dataBits = 8, StopBits stopBits = StopBits.One, Handshake handshake = Handshake.None)
        {
            _serialPort = new SerialPort()
            {
                PortName = portName,
                BaudRate = baudRate,
                Parity = parity,
                DataBits = dataBits,
                StopBits = stopBits,
                Handshake = handshake,

                ReadTimeout = 500,
                WriteTimeout = 500,

                NewLine = "\r"
            };

            _serialPort.DataReceived += _serialPort_DataReceived;

            _serialPort.Open();
        }

        private void _serialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            SerialPort data = (SerialPort)sender;

            if(data.ReadExisting() == "R")
            {
                OnRTS();
            }
        }

        public void Write(string data)
        {
            try
            {
                _serialPort.WriteLine(data);
            }
            catch(Exception)
            {
                throw;
            }
        }

        public void Dispose()
        {
            _serialPort.Close();
            _serialPort.Dispose();
        }

        public string[] GetPortNames()
        {
            return SerialPort.GetPortNames();
        }
    }
}
