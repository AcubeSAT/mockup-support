#include <iostream>



#include <chrono>
#include <thread>
#include <boost/asio.hpp>
#include <fstream>
#include <map>

#include "lib/imgui/imgui.h"
#include "imgui_impl_glfw.h"
#include <stdio.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <zmq.hpp>
#include <ecss-services/inc/Services/FunctionManagementService.hpp>
#include <ecss-services/inc/ServicePool.hpp>
#include <mockup/ECSSObjects.h>
#include <ecss-services/inc/Logger.hpp>
#include <cobs/cobs.h>
#include <queue>


using namespace std::chrono_literals;

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

// FreeRTOS defines
enum eTaskState {
    eRunning = 0,	/* A task is querying the state of itself, so must be running. */
	eReady,			/* The task being queried is in a read or pending ready list. */
	eBlocked,		/* The task being queried is in the Blocked state. */
	eSuspended,		/* The task being queried is in the Suspended state, or is in the Blocked state with an infinite time out. */
	eDeleted,		/* The task being queried has been deleted, but its TCB has not yet been freed. */
	eInvalid
 };

struct TaskInfo {
    unsigned int id;
    int state;
    uint32_t runTime;
    std::string name;
};

enum MessageType {
    Log = 1, // A log string
    SpacePacket = 2, // A CCSDS space packet
    Ping = 3, // Ping message
};

std::map<std::string, TaskInfo> taskList;
unsigned int currentTaskId = 0;

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
bool manualCalibrationEnabled = true;
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

/**
 * A queue of ECSS messages to be sent
 */
std::queue<Message> txMessages;

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

        LOG_INFO << "Connection successful";

        // Time when the last MySQL data was sent; used to prevent too frequent updates
//        std::chrono::steady_clock::time_point last_update = std::chrono::steady_clock::now();
//        std::chrono::steady_clock::time_point last_zmq_update = std::chrono::steady_clock::now();
        while (!stop) {
            try {
//                if (pendingCommand != 0) {
//                    // Send pending command to arduino
//                    std::ostringstream oss;
//                    oss << pendingCommand << '\n';
//                    boost::asio::write(serial, boost::asio::buffer(oss.str()));
//                    dataSent = true;
//                    pendingCommand = 0;
//                }

                //TODO: Parallelism fixes
                if (!txMessages.empty()) {
                    Message message = txMessages.back();
                    txMessages.pop();

                    auto data = MessageParser::composeECSS(message);

                    // Now encode the data via COBS
                    data.insert(data.begin(), static_cast<uint8_t>(MessageType::SpacePacket)); // Append packet type

                    LOG_TRACE << "Will send " << data.size() << " bytes of data. " << data[0];

                    uint8_t encoded[258];
                    auto result = cobs_encode(encoded, 257, data.c_str(), data.size());
                    encoded[result.out_len] = 0; // The null byte
                    boost::asio::write(serial, boost::asio::buffer(encoded, result.out_len + 1));
                }

                boost::asio::read_until(serial, buf, 0, ec);
                Logger::format.decimal();
                LOG_TRACE << "Read " << buf.size() << " bytes of data";

                std::string receivedRaw(reinterpret_cast<const char*>(buf.data().data()), buf.size());

                buf.consume(buf.size());

                // Decode the received data with cobs
                uint8_t received[300];
                auto result = cobs_decode(received, 300, receivedRaw.c_str(), receivedRaw.size() - 1); // strip the last byte

                if (result.out_len < 1) {
                    // Error
                    LOG_WARNING << "Too small packet received";
                    continue;
                }

                if (received[0] == Log) {
                    // Incoming log
                    LOG_TRACE << "[inc. log] " << std::string(reinterpret_cast<char*>(received + 1), result.out_len);
                } else if (received[0] == Ping) {
                    // Do nothing
                } else {
                        Logger::format.hex();
                        LOG_WARNING << "Unknown data received: " << received[0];
                }



//                    float norm = sqrtf(powf(valMagx, 2) + powf(valMagy, 2) + powf(valMagz, 2));

                //valMagz *= 100;
                //valPressure *= 100;
                //valBat *= 100;

//        serial.close();
            } catch (boost::system::system_error &e) {
                LOG_ERROR << "UART error: " << e.what();
            }
        }
    } catch (boost::system::system_error &e) {
        LOG_EMERGENCY << "Unable to open interface " << port << ": " << e.what();
        exit(5);
    }
}


int main(int argc, char* argv[]) {
    LOG_NOTICE << "Starting";

    if (argc != 2) {
        std::cerr << "You have not specified the serial interface to use. Usage: ./LinuxReceiver [/dev/ttyACM0]" << std::endl;
        return 5;
    }
    port = argv[1];

//    std::ifstream infile;
//    infile.open("config");
//    if (!infile.is_open()) {
//        std::cerr << "Please create a config file with: host username password database serialport" << std::endl;
//        return 5;
//    }
//    infile >> host >> username >> password >> database >> port;
//    infile.close();

    publisher.bind("tcp://*:5555");

    std::thread dataThread(dataAcquisition);
//    std::thread sqlThread(sqlStorage);

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
    imguiIo.Fonts->AddFontFromFileTTF("/home/kongr45gpen/repos/mockup-support/SerialHandler/lib/imgui/misc/fonts/DroidSans.ttf", 16.0f);
    imguiIo.Fonts->AddFontFromFileTTF("/home/kongr45gpen/repos/mockup-support/SerialHandler/ShareTechMono-Regular.ttf", 22.0f);
//    io.Fonts->AddFontFromFileTTF("../lib/imgui/misc/fonts/ProggyClean.ttf", 13.0f);
//    io.Fonts->AddFontFromFileTTF("../lib/imgui/misc/fonts/ProggyTiny.ttf", 10.0f);
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());

    bool show_test_window = true;
    ImVec4 clear_color = ImColor(35, 44, 59);

    addECSSObjects();


    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();
        if (show_test_window) {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
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

        ImGui::Begin("Parameter Management Service");
        static auto parameterList = Services.parameterManagement.getParamsList();

        for (auto it = parameterList.begin(); it != parameterList.end(); it++) {
            ImGui::Text("%s", parIdToString[it->first].data());
            ParameterBase* parameter = (it->second);

            ImGui::PushID(it->first);
            ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() - 340);

            if (dynamic_cast<Parameter<uint8_t>*>(parameter) != nullptr) {
                ImGui::DragScalar("8-bit integer",        ImGuiDataType_U8,     parameter->ptr(), 1);
            } else if (dynamic_cast<Parameter<uint32_t>*>(parameter) != nullptr) {
                ImGui::DragScalar("32-bit integer",        ImGuiDataType_U32,     parameter->ptr(), 1);
            } else if (dynamic_cast<Parameter<float>*>(parameter) != nullptr) {
                ImGui::DragScalar("32-bit float",        ImGuiDataType_Float,     parameter->ptr(), 0.01);
            } else if (dynamic_cast<Parameter<double>*>(parameter) != nullptr) {
                ImGui::DragScalar("64-bit double",        ImGuiDataType_Double,     parameter->ptr(), 0.01);
            }

            ImGui::SameLine();
            if (ImGui::Button("Update", {100, 0})) {
                Logger::format.decimal();
                LOG_DEBUG << "Got call to update parameter _" << parIdToString[it->first] << "_ [" << it->first << "]";

                Message message(20, 3, Message::TC, 1);
                message.appendUint16(1); // Number of parameters to update
                message.appendUint16(it->first); // Parameter ID
                message.appendString(it->second->getValueAsString());

                LOG_TRACE << "New message with size " << message.dataSize;
                Service::storeMessage(message);
            }

            ImGui::PopItemWidth();
            ImGui::PopID();

            ImGui::NewLine();
        }
        ImGui::End();

        ImGui::Begin("Function Management Service");
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(3.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(3.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(3.0f, 0.8f, 0.8f));
        ImGui::PushItemWidth(ImGui::GetWindowWidth());
        ImFont* font = ImGui::GetIO().Fonts->Fonts[1];
        ImGui::PushFont(font);

        static FunctionMap & functionMap = Services.functionManagement.getFunctionMap();

        for (auto it = functionMap.begin(); it != functionMap.end(); it++) {
            if (ImGui::Button((*it).first.c_str(), {ImGui::GetContentRegionAvailWidth(),0})) {
                auto name = it->first;
                LOG_DEBUG << "Creating _" << name.c_str() << "_ function call";

                // Create a new message
                Message message(8, 1, Message::TC, 1);
                message.appendFixedString(String<ECSS_FUNCTION_NAME_LENGTH>(name));
                LOG_TRACE << "New message with size " << message.dataSize;
                Service::storeMessage(message);
            }
        }


        ImGui::PopFont();
        ImGui::PopItemWidth();
        ImGui::PopStyleColor(3);
        ImGui::End();

        ImGui::Begin("Task List");

        ImGui::Text("FreeRTOS Task List:");
        ImGui::Columns(4, "tasks"); // 4-ways, with border
        ImGui::Separator();
        ImGui::Text("ID"); ImGui::NextColumn();
        ImGui::Text("Name"); ImGui::NextColumn();
        ImGui::Text("%% CPU"); ImGui::NextColumn();
        ImGui::Text("State"); ImGui::NextColumn();
        ImGui::Separator();
        static int selected = -1;

        srand(time(NULL));

        uint32_t sum = 0;
        for (auto it = taskList.begin(); it != taskList.end(); it++) {
            sum += it->second.runTime;
        }

        for (auto it = taskList.begin(); it != taskList.end(); it++) {
            TaskInfo & task = it->second;

            char label[32];
            sprintf(label, "%02d", task.id);
            if (ImGui::Selectable(label, selected == task.id, ImGuiSelectableFlags_SpanAllColumns)) {
                selected = task.id;
            }
            bool hovered = ImGui::IsItemHovered();
            ImGui::NextColumn();
            ImGui::Text(task.name.c_str()); ImGui::NextColumn();

            float progress = task.runTime / (float) sum;
            ImGui::ProgressBar(progress, ImVec2(0.0f,0.0f));
            ImGui::NextColumn();

            float hue;
            std::string taskDescription;
            eTaskState taskState = (eTaskState) (task.state);

            switch(taskState) {
                case eRunning:
                    hue = 0.286f;
                    taskDescription = "RUNNING";
                    break;
                case eReady:
                    hue = 0.214f;
                    taskDescription = "READY";
                    break;
                case eBlocked:
                    hue = 0.136f;
                    taskDescription = "BLOCKED";
                    break;
                case eSuspended:
                    hue = 0.025f;
                    taskDescription = "SUSPENDED";
                    break;
                case eDeleted:
                    hue = 0.819f;
                    taskDescription = "DELETED";
                    break;
                default:
                    hue = 0.875f;
                    taskDescription = "INVALID";
            }

            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(hue, 0.9f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(hue, 1.0f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(hue, 1.0f, 0.8f));
            ImGui::Button(taskDescription.c_str());
            ImGui::PopStyleColor(3);
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::Separator();

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
//    dataThread.join();
//    sqlThread.join();
    std::cout << "Thread stopped." << std::endl;

    // Cleanup
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();

    return 0;
}

void Service::storeMessage(Message& message) {
    // appends the remaining bits to complete a byte
    message.finalize();

    // Create a new stream to display the packet
    std::ostringstream ss;

    // Just print it to the screen
    ss << "New " << ((message.packetType == Message::TM) ? "TM" : "TC") << "["
       << std::hex
       << static_cast<int>(message.serviceType) << "," // Ignore-MISRA
       << static_cast<int>(message.messageType) // Ignore-MISRA
       << "] message! ";

    for (unsigned int i = 0; i < message.dataSize; i++) {
        ss << static_cast<int>(message.data[i]) << " "; // Ignore-MISRA
    }

    LOG_TRACE << ss.str();

    txMessages.push(message);
}
