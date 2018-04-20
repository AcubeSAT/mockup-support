using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using NetMQ;
using NetMQ.Sockets;

/**
  * This program receives data using NetMQ library from a Publisher instance. It subscribes on "xaris" topic.
  * @author Thanasis Theocharis
  *

namespace NetMQtest
{
    class NetMQ_Test_CSharp
    {
        static SubscriberSocket subscriberSocket;

        static void Main(string[] args)
        {
            //Initializes the Subscriber instance
            subscriberSocket = new SubscriberSocket();
            
            //IP to connect is 192.168.43.117:5555
            subscriberSocket.Connect("tcp://192.168.43.117:5555");
            
            //Subscribes to "xaris" topic
            subscriberSocket.Subscribe("xaris");
            
            while (true)
            {
                Update();
            }
        }

        static void Update()
        {
            string dataString = "null received";
            
            if (1 == 1)
            {
                try
                {
                    //Tries to receive a string from the Publisher
                    dataString = subscriberSocket.ReceiveFrameString();
                }
                catch (System.IO.IOException ioe)
                {
                    Console.WriteLine("IOException: " + ioe.Message);
                }

            }
            else
                dataString = "NOT OPEN";
            
            //Prints either the received String or "NOT OPEN"
            Console.WriteLine("RCV_ : " + dataString);
        }
    }
}
