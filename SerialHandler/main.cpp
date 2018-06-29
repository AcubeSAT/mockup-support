#include <iostream>

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <chrono>
#include <thread>
#include <boost/asio.hpp>
#include <fstream>

#include "lib/imgui/imgui.h"
#include "imgui_impl_glfw.h"
#include <stdio.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include "MadgwickAHRS/MadgwickAHRS.h"
#include <math.h>
#include <zmq.hpp>

// The number of points to include in the graph
const int GRAPH_SIZE = 300;

// The time between different serial data fetches and stores
const int update_ms = 1000;
const int update_zmq_ms = 50;

const int MAX_CALIBRATION_VALUES = 80;
const float gyro_normalizer_factor = 57.3f / 7.0f;

static void error_callback(int error, const char *description) {
    fprintf(stderr, "Error %d: %s\n", error, description);
}

// Interthread variables
float magneticData[GRAPH_SIZE] = {0};
bool stop = false;
bool dataReceived = false;
bool dataSent = false;
bool dataSendingDB = false;
bool dataSentDB = false;
bool dataSendingZMQ = false;
bool dataSentZMQ = false;
bool zmqEnabled = true;
bool noiseGateEnabled = true;
std::array<bool, 3> noiseGateActivated;
char pendingCommand = 0;

bool flipX = true;
bool flipY = false;
bool flipZ = true;

std::array<float, 6> calibration;
bool calibrated = false;
int calibrationValues;

bool sqlDataPending = false;
std::array<float, 7> sqlData;

std::string host, username, password, database, port;

zmq::context_t context(1);
zmq::socket_t publisher(context, ZMQ_PUB);

float varAngx = 0, varAngy = 0, varAngz = 0;

float dataToShow[6];

void resetCalibration() {
    varAngx = varAngy = varAngz = 0;

    calibrated = false;
    calibrationValues = -10; // don't include first 10 values
    calibration = {0,0,0,0,0,0};

     q0 = 1.0f;
     q1 = 0.0f;
     q2 = 0.0f;
     q3 = 0.0f;
}

void sqlStorage() {
    sql::Driver *driver;
    sql::Connection *con;
    sql::PreparedStatement *stMagnetic;
    sql::PreparedStatement *stAcc;
    sql::PreparedStatement *stGyro;
    sql::PreparedStatement *stSignal;

    // MySQL initialisation
    driver = get_driver_instance();
    con = driver->connect(host.c_str(), username.c_str(), password.c_str());
    con->setSchema(database.c_str());

    // Prepare insertion statements
    stMagnetic = con->prepareStatement(
            "INSERT INTO mag(value,time) VALUES(?,NOW(2))     ON DUPLICATE KEY UPDATE value = ?");
    stAcc = con->prepareStatement(
            "INSERT INTO acc(x,y,z,time) VALUES(?,?,?,NOW(2)) ON DUPLICATE KEY UPDATE x = ?, y = ?, z = ?");
    stGyro = con->prepareStatement(
            "INSERT INTO gyro(x,y,z,time) VALUES(?,?,?,NOW(2)) ON DUPLICATE KEY UPDATE x = ?, y = ?, z = ?");
    stSignal = con->prepareStatement(
            "INSERT INTO `signal`(value,time) VALUES(?,NOW(2))  ON DUPLICATE KEY UPDATE value = ?");

    while (!stop) {
        if (!sqlDataPending) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        sqlDataPending = false;

        dataSendingDB = true;

        std::array<float, 7> data = sqlData;

        // Perform the update
        stMagnetic->setDouble(1, data[6]);
        stMagnetic->setDouble(2, data[6]);
        stMagnetic->execute();

//        stSignal->setDouble(1, data[1]);
//        stSignal->setDouble(2, data[1]);
//        stSignal->execute();

        stAcc->setDouble(1, data[0]);
        stAcc->setDouble(2, data[1]);
        stAcc->setDouble(3, data[2]);
        stAcc->setDouble(4, data[0]);
        stAcc->setDouble(5, data[1]);
        stAcc->setDouble(6, data[2]);
        stAcc->execute();

        stGyro->setDouble(1, data[3]);
        stGyro->setDouble(2, data[4]);
        stGyro->setDouble(3, data[5]);
        stGyro->setDouble(4, data[3]);
        stGyro->setDouble(5, data[4]);
        stGyro->setDouble(6, data[5]);
        stGyro->execute();

        dataSentDB = true;
    }

    delete stMagnetic;
    delete stSignal;
    delete con;

}

void dataAcquisition() {
    try {
        // Serial interface initialisation
        boost::asio::io_service io;
        boost::asio::serial_port serial(io, port);
        serial.set_option(boost::asio::serial_port_base::baud_rate(115200));

        boost::asio::streambuf buf;
        std::istream is(&buf);
        std::istringstream iss;

        std::string line;
        boost::system::error_code ec;

        // Time when the last MySQL data was sent; used to prevent too frequent updates
        std::chrono::steady_clock::time_point last_update = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point last_zmq_update = std::chrono::steady_clock::now();
        while (!stop) {
            try {
                if (pendingCommand != 0) {
                    // Send pending command to arduino
                    std::ostringstream oss;
                    oss << pendingCommand << '\n';
                    boost::asio::write(serial, boost::asio::buffer(oss.str()));
                    dataSent = true;
                    pendingCommand = 0;
                }

                // TODO: Move this into another function
                // TODO: Make this asynchronous
                boost::asio::read_until(serial, buf, '\n', ec);

                if (!ec) {
                    float valAccx, valAccy, valAccz, valGyrox, valGyroy, valGyroz, valBright;
                    //valSignal = 0;
                    std::getline(is, line);
                    iss = std::istringstream(line);
                    iss >> valAccx >> valAccy >> valAccz >> valGyrox >> valGyroy >> valGyroz >> valBright;// >> valSignal;
                    dataReceived = true;
                    dataToShow[0] = valAccx;
                    dataToShow[1] = valAccy;
                    dataToShow[2] = valAccz;
                    dataToShow[3] = valGyrox;
                    dataToShow[4] = valGyroy;
                    dataToShow[5] = valGyroz;


//                    float norm = sqrtf(powf(valMagx, 2) + powf(valMagy, 2) + powf(valMagz, 2));

                    //valMagz *= 100;
                    //valPressure *= 100;
                    //valBat *= 100;

                    if (std::chrono::steady_clock::now() - last_update > std::chrono::milliseconds(update_ms)) {
                        std::cout
                                << valAccx << '\t'
                                << valAccy << '\t'
                                << valAccz << '\t'
                                << valGyrox << '\t'
                                << valGyroy << '\t'
                                << valGyroz << '\t'
                                << valBright << '\t'
                                << std::endl;

                        sqlDataPending = true;
                        sqlData[0] = valAccx;
                        sqlData[1] = valAccy;
                        sqlData[2] = valAccz;
                        sqlData[3] = valGyrox;
                        sqlData[4] = valGyroy;
                        sqlData[5] = valGyroz;
                        sqlData[6] = valBright;

                        last_update = std::chrono::steady_clock::now();
                    }

                    // Calibrate received values
                    if (!calibrated) {
                        if (calibrationValues >= 0) {
                            calibration[0] += valGyrox / (float) MAX_CALIBRATION_VALUES;
                            calibration[1] += valGyroy / (float) MAX_CALIBRATION_VALUES;
                            calibration[2] += valGyroz / (float) MAX_CALIBRATION_VALUES;
                            calibration[3] += valAccx / (float) MAX_CALIBRATION_VALUES;
                            calibration[4] += valAccy / (float) MAX_CALIBRATION_VALUES;
                            calibration[5] += valAccz / (float) MAX_CALIBRATION_VALUES;
                        }

                        calibrationValues++;
                        if (calibrationValues >= MAX_CALIBRATION_VALUES) {
                            calibrated = true;
                        }
                    }
                    valGyrox = valGyrox - calibration[0];
                    valGyroy = valGyroy - calibration[1];
                    valGyroz = valGyroz - calibration[2];

                    if (calibrated) {
                        if (noiseGateEnabled) {
                          if (fabs(valGyrox) < 0.100) { valGyrox /= 10.0; noiseGateActivated[0] = true; }
                          if (fabs(valGyroy) < 0.100) { valGyroy /= 10.0; noiseGateActivated[1] = true; }
                          if (fabs(valGyroz) < 0.100) { valGyroz /= 10.0; noiseGateActivated[2] = true; }
                        }

                        MadgwickAHRSupdateIMU(valGyrox, valGyroy, valGyroz, valAccx, valAccy, valAccz);
//                         if (fabs(valGyrox * gyro_normalizer_factor) > 0.025){
//                             varAngx += valGyrox * gyro_normalizer_factor;
//                         }
//                         if (fabs(valGyroy * gyro_normalizer_factor) > 0.025){
//                             varAngy += valGyroy * gyro_normalizer_factor;
//                         }
//                         if (fabs(valGyroz * gyro_normalizer_factor) > 0.005){
//                             varAngz += valGyroz * gyro_normalizer_factor;
//                         }

                        varAngx = atan2f(2*(q0*q1+q2*q3),1-2*(q1*q1+q2*q2));
                        varAngy = asinf(2*(q0*q2-q3*q1));
                        varAngz = atan2f(2*(q0*q3+q1*q2),1-2*(q2*q2+q3*q3));

                        varAngx *= gyro_normalizer_factor;
                        varAngy *= gyro_normalizer_factor;
                        varAngz *= gyro_normalizer_factor;
                    }
		    int factorX, factorY, factorZ;
                    factorX = factorY = factorZ = 1;
                    if (flipX) factorX = -1;
                    if (flipY) factorY = -1;
                    if (flipZ) factorZ = -1;

                    dataSendingZMQ = true;
                    // Send the data to ZeroMQ
                    if (zmqEnabled && std::chrono::steady_clock::now() - last_zmq_update > std::chrono::milliseconds(update_zmq_ms)) {
                        zmq::message_t message(128);
                        snprintf((char *) message.data(), 128,
                                 "cubesat %f %f %f %f", factorY * varAngy, factorZ * varAngz,
                                 factorX * varAngx, valBright);
                        publisher.send(message);
                        dataSentZMQ = true;

                        last_zmq_update = std::chrono::steady_clock::now();
                    }

                    // Move data back
                    for (int i = 1; i < GRAPH_SIZE; i++) {
                        magneticData[i - 1] = magneticData[i];
                    }
                    magneticData[GRAPH_SIZE - 1] = valBright;

                    std::this_thread::sleep_for(std::chrono::milliseconds(2));

                    if (line == "stop") {
                        break;
                    }

                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(2));
                }
            }
            catch (boost::system::system_error &e) {
                std::cerr << "Unable to execute " << e.what();
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }

//        serial.close();
    } catch (boost::system::system_error &e) {
        std::cerr << "Unable to open interface " << port << ": " << e.what();
    }
}


int main() {
    std::cout << "Starting" << std::endl;
    resetCalibration();

    std::ifstream infile;
    infile.open("config");
    if (!infile.is_open()) {
        std::cerr << "Please create a config file with: host username password database serialport" << std::endl;
        return 5;
    }
    infile >> host >> username >> password >> database >> port;
    infile.close();

    publisher.bind("tcp://*:5555");

    std::thread dataThread(dataAcquisition);
    std::thread sqlThread(sqlStorage);

    // Setup window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(1280, 720, "ASAT CubeSAT Demonstration", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    gl3wInit();
    //ImGui_ImplGlfwGL3_GLEWInit();

    // Setup ImGui binding
    ImGui::CreateContext();
    ImGuiIO &imguiIo = ImGui::GetIO();

    ImGui_ImplGlfwGL3_Init(window, true);

    // Load Fonts
    // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
    ImGui::StyleColorsDark();
//  ImGui::StyleColorsClassic();
    //io.Fonts->AddFontDefault();
//    io.Fonts->AddFontFromFileTTF("../lib/imgui/misc/fonts/Cousine-Regular.ttf", 15.0f);
    imguiIo.Fonts->AddFontFromFileTTF("lib/imgui/misc/fonts/DroidSans.ttf", 16.0f);
    imguiIo.Fonts->AddFontFromFileTTF("lib/imgui/misc/fonts/DroidSans.ttf", 32.0f);
//    io.Fonts->AddFontFromFileTTF("../lib/imgui/misc/fonts/ProggyClean.ttf", 13.0f);
//    io.Fonts->AddFontFromFileTTF("../lib/imgui/misc/fonts/ProggyTiny.ttf", 10.0f);
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());

    bool show_test_window = false;
    ImVec4 clear_color = ImColor(35, 44, 59);


    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();
        if (show_test_window) {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
            ImGui::ShowTestWindow();
        }

        ImGui::Begin("ASAT CubeSAT");

        ImGui::Checkbox("Test", &show_test_window);
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4({0.1f, 0.9f, 0.05f, 1.0f}));
        ImGui::Checkbox("", &dataReceived);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4({0.9f, 0.3f, 0.05f, 1.0f}));
        ImGui::Checkbox("Data", &dataSent);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4({0.9f, 0.2f, 0.05f, 1.0f}));
        ImGui::Checkbox("", &dataSendingDB);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4({0.9f, 0.4f, 0.05f, 1.0f}));
        ImGui::Checkbox("Database", &dataSentDB);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4({0.6f, 0.2f, 0.45f, 1.0f}));
        ImGui::Checkbox("", &dataSendingZMQ);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4({0.6f, 0.4f, 0.45f, 1.0f}));
        ImGui::Checkbox("ZeroMQ", &dataSentZMQ);
        ImGui::PopStyleColor();

        // Reset indicators so that they light up just for one frame
        dataReceived = dataSent = false;
        if (dataSentDB) dataSendingDB = false;
        if (dataSentZMQ) dataSendingZMQ = false;
        dataSentDB = false;
        dataSentZMQ = false;
        noiseGateActivated = {false, false, false};

        /*
        glBegin(GL_LINE_LOOP);//start drawing a line loop
        glVertex3f(-1.0f, 0.0f, 0.0f);//left of window
        glVertex3f(0.0f, -1.0f, 0.0f);//bottom of window
        glVertex3f(1.0f, 0.0f, 0.0f);//right of window
        glVertex3f(0.0f, 1.0f, 0.0f);//top of window
        glEnd();//end drawing of line loo
    */
        ImGui::Text("Light intensity");
        ImGui::PlotLines("", magneticData, GRAPH_SIZE, 0, nullptr, 0, FLT_MAX,
                         ImVec2(ImGui::GetContentRegionAvailWidth(), 80));

        ImGui::Checkbox("Enable ZeroMQ Data Transmission", &zmqEnabled);
        ImGui::End();

        ImGui::Begin("Calibration Status");
        ImGui::Text("Status: %s", (calibrated) ? "ready" : "calibrating");
	ImGui::Checkbox("Flip X", &flipX);
	ImGui::SameLine();
	ImGui::Checkbox("Flip Y", &flipY);
	ImGui::SameLine();
	ImGui::Checkbox("Flip Z", &flipZ);
        if (ImGui::Button("Recalibrate")) {
            resetCalibration();
        }
        if (ImGui::Button("Reset Position")) {
            varAngx = varAngy = varAngz = 0;
            q0 = 1.0f;
            q1 = 0.0f;
            q2 = 0.0f;
            q3 = 0.0f;
        }
        ImGui::Text("Processed values: %d", calibrationValues);
        ImGui::Text("aX: %f, aY: %f, aZ: %f", varAngx, varAngy, varAngz);
        ImGui::Text("cX: %f, cY: %f, cZ: %f", calibration[0], calibration[1], calibration[2]);
        ImGui::Text("Raw values");
        ImGui::Text("ACCEL X: %f, Y: %f, Z: %f", dataToShow[0],dataToShow[1],dataToShow[2]);
        ImGui::Text("GYRO  X: %f, Y: %f, Z: %f", dataToShow[3],dataToShow[4],dataToShow[5]);

        ImGui::Checkbox("Enable Noise Gate", &noiseGateEnabled);
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4({0.9f, 0.4f, 0.05f, 1.0f}));
        ImGui::Checkbox("",       &(noiseGateActivated[0])); ImGui::SameLine();
        ImGui::Checkbox("",       &(noiseGateActivated[1])); ImGui::SameLine();
        ImGui::Checkbox("active", &(noiseGateActivated[2])); ImGui::SameLine();
        ImGui::PopStyleColor();

        ImGui::End();

        ImGui::Begin("Commands to Unity 3D-Model");
        if (ImGui::Button("Recalibrate")) {
            dataSendingZMQ = true;
            // Send the data to ZeroMQ
            zmq::message_t message(255);
            snprintf((char *) message.data(), 255,
                     "cubesat calibrate");
            publisher.send(message);
            dataSentZMQ = true;
        }
        ImGui::End();

//        ImGui::Begin("Commands to ground station");
//        if (ImGui::Button("Reset")) {
//            stop = true;
//            dataThread.join();
//            sqlThread.join();
//
//            std::this_thread::sleep_for(std::chrono::seconds(1));
//
//            stop = false;
//            dataThread = std::thread(dataAcquisition);
//            sqlThread = std::thread(sqlStorage);
//        }
//        ImGui::End();

        ImGui::Begin("Commands to satellite");
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(3.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(3.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(3.0f, 0.8f, 0.8f));
        ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
        ImFont* font = ImGui::GetIO().Fonts->Fonts[1];
        ImGui::PushFont(font);
        if (ImGui::Button("Send Command")) {
            pendingCommand = 'l';
        }
        ImGui::PopFont();
        ImGui::PopItemWidth();
        ImGui::PopStyleColor(3);
        if (ImGui::Button("Reset")) {
            pendingCommand = 'r';
        }
        ImGui::End();

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Stop the acquisition thread
    std::cout << "Stopping thread..." << std::endl;
    stop = true;
    dataThread.join();
    sqlThread.join();
    std::cout << "Thread stopped." << std::endl;

    // Cleanup
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();

    return 0;
}
