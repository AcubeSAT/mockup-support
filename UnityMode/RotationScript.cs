using UnityEngine;
using System;
using System.IO.Ports;
using NetMQ.Sockets;
using NetMQ;

/*
 * Script to rotate and translate an object in unity according to data received from a ZeroMQ Publisher 
 * @author Thanasis Theocharis
 * Should be applied on a Unity model and then be sure to: 
 * 1) drag the unity object in this script in Unity UI
 * 2) Unity edit > project settings > player is NET2.0 and not NET2.0Subset
 */

public class Rotation_Script : MonoBehaviour
{
    //Subscriber to receive data from ZeroMQ Publisher
    SubscriberSocket subscriberSocket;
    
    //CubeSat Model 
    public GameObject target;

    float acc_normalizer_factor = 0.00025f;
    float gyro_normalizer_factor = 1.0f / 32768.0f;   // 32768 is max value captured during test on imu

    float curr_angle_x = 0;
    float curr_angle_y = 0;
    float curr_angle_z = 0;

    float curr_offset_x = 0;
    float curr_offset_y = 0;
    float curr_offset_z = 0;

    // Increase the speed/influence rotation
    public float factor = 7;
    
    //Booleans to enable the rotation and the translation of our  model
    public bool enableRotation;
    public bool enableTranslation;

    // SELECT YOUR COM PORT AND BAUDRATE
    int readTimeout = 25;
    Vector3 offset;

    void Start()
    {
        Debug.Log("Start");
        
        //might be unnecessary
        target = this.gameObject;
        //Initializes a new Subscriber
        subscriberSocket = new SubscriberSocket();
        
        //Connects to the specific IP
        subscriberSocket.Connect("tcp://192.168.43.117:5555");
        
        //Connects to a specific topic
        subscriberSocket.Subscribe("xaris");
    }

    void Update()
    {

        string dataString = "null received";

        if (1 == 1)
        {
            try
            {
                //Tries to receive a string published from ZeroMQ Publisher 
                dataString = subscriberSocket.ReceiveFrameString();
                Debug.Log("RCV_ : " + dataString);
                
                Debug.LogError("RCV_Error: " + dataString);
            }
            catch (System.IO.IOException ioe)
            {
                Debug.Log("IOException: " + ioe.Message);
            }

        }
        else
            dataString = "NOT OPEN";
        Debug.Log("RCV_ : " + dataString);
        
        //Uncomment the following part when the receiver works properly on Unity
        /*
        if (!dataString.Equals("NOT OPEN"))
        {
            // recived string is  like  "accx;accy;accz;gyrox;gyroy;gyroz"
            char splitChar = ';';
            string[] dataRaw = dataString.Split(splitChar);

            // normalized accelerometer values
            float ax = Int32.Parse(dataRaw[0]) * acc_normalizer_factor;
            float ay = Int32.Parse(dataRaw[1]) * acc_normalizer_factor;
            float az = Int32.Parse(dataRaw[2]) * acc_normalizer_factor;

            // normalized gyrocope values
            float gx = Int32.Parse(dataRaw[3]) * gyro_normalizer_factor;
            float gy = Int32.Parse(dataRaw[4]) * gyro_normalizer_factor;
            float gz = Int32.Parse(dataRaw[5]) * gyro_normalizer_factor;

            // prevent 
            //if (Mathf.Abs(ax) - 1 < 0) ax = 0;
            //if (Mathf.Abs(ay) - 1 < 0) ay = 0;
            //if (Mathf.Abs(az) - 1 < 0) az = 0;


            curr_offset_x += ax;
            curr_offset_y += ay;
            curr_offset_z += 0; // The IMU module have value of z axis of 16600 caused by gravity


            // prevent little noise effect
            if (Mathf.Abs(gx) < 0.025f) gx = 0f;
            if (Mathf.Abs(gy) < 0.025f) gy = 0f;
            if (Mathf.Abs(gz) < 0.025f) gz = 0f;

            curr_angle_x += gx;
            curr_angle_y += gy;
            curr_angle_z += gz;

            if (enableTranslation) target.transform.position = new Vector3(curr_offset_x, curr_offset_y, curr_offset_z);
            if (enableRotation) target.transform.rotation = Quaternion.Euler(curr_angle_x * factor, curr_angle_y * factor, curr_angle_z * factor);
           
       }
        */
    }

}
