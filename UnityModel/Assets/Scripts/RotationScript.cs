using UnityEngine;
using System;
using System.IO.Ports;
using System.Text;
using NetMQ.Sockets;
using NetMQ;
using UnityEngine.UI;

/*
 * Script to rotate and translate an object in unity according to data received from a ZeroMQ Publisher
 * @author Thanasis Theocharis
 * Should be applied on a Unity model and then be sure to:
 * 1) drag the unity object in this script in Unity UI
 * 2) Unity edit > project settings > player is NET2.0 and not NET2.0Subset
 */

public class RotationScript : MonoBehaviour
{
    //Subscriber to receive data from ZeroMQ Publisher
    SubscriberSocket subscriberSocket;

    //CubeSat Model
    public GameObject target;

    //float acc_normalizer_factor = 0.00025f;
    //float gyro_normalizer_factor = 1.0f / 32768.0f;   // 32768 is max value captured during test on imu
    float gyro_normalizer_factor = 1.0f;   // 32768 is max value captured during test on imu


    float curr_angle_x = 0;
    float curr_angle_y = 0;
    float curr_angle_z = 0;

    float curr_offset_x = 0;
    float curr_offset_y = 0;
    float curr_offset_z = 0;

    // Increase the speed/influence rotation
    public float factor = 3;

    //Booleans to enable the rotation and the translation of our  model
    public bool enableRotation;
    //public bool enableTranslation;

    public bool received, calibrated;

    //calibrated<> adds the first 40 calibrated values, avgG<> is the average of these first calibrated values that will be reducted so that the system will be calibrated
    float calibratedX, calibratedY, calibratedZ, avgGx, avgGy, avgGz;

    //counter of values added to take the average for calibration purposes
    public int calibrationCounter;

    private int counter;

    public Text rotX;
    public Text rotY;
    public Text rotZ;
    public Text gyroX;
    public Text gyroY;
    public Text gyroZ;
    public Text count;
    public Text status;


    Vector3 offset;

    void Start()
    {
        Debug.LogError("Start");

        //The values are not calibrated yet
        calibrated = false;

        calibrationCounter = 0;
        calibratedX = 0;
        calibratedY = 0;
        calibratedZ = 0;

        //might be unnecessary
        target = this.gameObject;

        //Initializes a new Subscriber
        subscriberSocket = new SubscriberSocket();

        //Connects to the specific IP
        subscriberSocket.Connect("tcp://localhost:5555");

        //Connects to a specific topic
        subscriberSocket.Subscribe("cubesat");
    }

    void Update()
    {
        received = false;
        calibrated = true;
        string dataString = "null received";


        //Tries to receive a string published from ZeroMQ Publisher
        if (subscriberSocket.TryReceiveFrameString(TimeSpan.FromMilliseconds(5), out dataString))
        {
            //Debug.Log("RCV_ : " + dataString);
            received = true;
        }
        else
        {
            //Debug.LogError("RCV_Error: " + dataString);
        }

        //If received a string only then edit the string and change the rotation of the CubeSat
        if (received)
        {
            // received string is  like  "topic accx accy accz gyrox gyroy gyroz"
            char splitChar = ' ';
            string[] dataRaw = dataString.Split(splitChar);
            string identifier = dataRaw[1].Substring(0, (dataRaw[1]).Length < 9 ? dataRaw[1].Length : 9);

            if (identifier.Equals("calibrate")) {
                 calibrated = false;
                 calibrationCounter = 0;
                 //reseting angles
                 curr_angle_x = 0;
                 curr_angle_y = 0;
                 curr_angle_z = 0;

                 //reseting calibrating values
                 calibratedX = 0;
                 calibratedY = 0;
                 calibratedZ = 0;

                 return;
             }

            // normalized accelerometer values
            //float ax = float.Parse(dataRaw[1]) * acc_normalizer_factor;
            //float ay = float.Parse(dataRaw[2]) * acc_normalizer_factor;
            //float az = float.Parse(dataRaw[3]) * acc_normalizer_factor;

            // normalized gyrocope values
            float gx = float.Parse(dataRaw[1]) * gyro_normalizer_factor;
            float gy = float.Parse(dataRaw[2]) * gyro_normalizer_factor;
            float gz = float.Parse(dataRaw[3]) * gyro_normalizer_factor;

            //Debug.Log("The data raw of 1 iz: " + gx);

            // prevent
            //if (Mathf.Abs(ax) - 1 < 0) ax = 0;
            //if (Mathf.Abs(ay) - 1 < 0) ay = 0;
            //if (Mathf.Abs(az) - 1 < 0) az = 0;


            //curr_offset_x += ax;
            //curr_offset_y += ay;
            //curr_offset_z += 0; // The IMU module have value of z axis of 16600 caused by gravity

            gyroX.text = string.Format("Gyroscope X: {0:N4}", gx);
            gyroY.text = string.Format("Gyroscope Y: {0:N4}", gy);
            gyroZ.text = string.Format("Gyroscope Z: {0:N4}", gz);

            status.text = (calibrated) ? "Status: Running" : "Status: Calibrating";
            count.text = string.Format("Counter: {0:N0}", counter);

            // prevent little noise effect
            //if (Mathf.Abs(gx) < 0.025f) gx = 0f;
            //if (Mathf.Abs(gy) < 0.025f) gy = 0f;
            //if (Mathf.Abs(gz) < 0.025f) gz = 0f;

            if (calibrated)
            {
                counter++;
                curr_angle_x = gx;
                curr_angle_y = gy;
                curr_angle_z = gz;
                /*curr_angle_x += gx-avgGx;
                curr_angle_y += gy-avgGy;
                curr_angle_z += gz-avgGz;*/

                //if (enableTranslation) target.transform.position = new Vector3(curr_offset_x, curr_offset_y, curr_offset_z);
                if (enableRotation) target.transform.rotation = Quaternion.Euler(curr_angle_x * factor, curr_angle_y * factor, curr_angle_z * factor);

                rotX.text = string.Format("Rotation X: {0:N4}", curr_angle_x * factor);
                rotY.text = string.Format("Rotation Y: {0:N4}", curr_angle_y * factor);
                rotZ.text = string.Format("Rotation Z: {0:N4}", curr_angle_z * factor);
            } else
            {
                calibrationCounter++;
                calibratedX += gx;
                calibratedY += gy;
                calibratedZ += gz;
                if (calibrationCounter == 40)
                {
                    calibrated = true;
                    avgGx = calibratedX / (float)40;
                    avgGy = calibratedY / (float)40;
                    avgGz = calibratedZ / (float)40;
                }
            }


        }

    }

}
