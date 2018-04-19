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

#include <zmq.hpp>

// The number of points to include in the graph
const int GRAPH_SIZE = 100;

// The time between different serial data fetches and stores
const int update_ms = 1000;

static void error_callback(int error, const char *description) {
  fprintf(stderr, "Error %d: %s\n", error, description);
}

// Interthread variables
float magneticData[GRAPH_SIZE] = {0};
bool stop = false;
bool dataReceived = false;
bool dataSendingDB = false;
bool dataSentDB = false;
bool dataSendingZMQ = false;
bool dataSentZMQ = false;

void dataAcquisition() {
  std::string host, username, password, database, port;
  std::ifstream infile;
  infile.open("config");
  if (!infile.is_open()) {
    std::cerr << "Please create a config file with: host username password database serialport" << std::endl;
    return;
  }
  infile >> host >> username >> password >> database >> port;
  infile.close();

  sql::Driver *driver;
  sql::Connection *con;
  sql::PreparedStatement *stMagnetic;
  sql::PreparedStatement *stBattery;
  sql::PreparedStatement *stSignal;

  // ZeroMQ initialisation
  zmq::context_t context(1);
  zmq::socket_t publisher(context, ZMQ_PUB);
  publisher.bind("tcp://*:5555");

  // MySQL initialisation
  driver = get_driver_instance();
  con = driver->connect(host.c_str(), username.c_str(), password.c_str());
  con->setSchema(database.c_str());

  // Prepare insertion statements
  stMagnetic = con->prepareStatement(
      "INSERT INTO mag(value,time) VALUES(?,NOW(2))     ON DUPLICATE KEY UPDATE value = ?");
  stBattery = con->prepareStatement(
      "INSERT INTO battery(value,time) VALUES(?,NOW(2)) ON DUPLICATE KEY UPDATE value = ?");
  stSignal = con->prepareStatement(
      "INSERT INTO `signal`(value,time) VALUES(?,NOW(2))  ON DUPLICATE KEY UPDATE value = ?");

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
    while (!stop) {
      try {
        // TODO: Move this into another function
        // TODO: Make this asynchronous
        boost::asio::read_until(serial, buf, '\n', ec);

        if (!ec) {
          float valMagx, valMagy, valMagz, valTemp, valBat, valPressure, valSignal;
          std::getline(is, line);
          iss = std::istringstream(line);
          iss >> valTemp >> valMagx >> valMagy >> valMagz >> valPressure >> valBat >> valSignal;
          dataReceived = true;

          float norm = sqrtf(powf(valMagx, 2) + powf(valMagy, 2) + powf(valMagz, 2));

          if (std::chrono::steady_clock::now() - last_update > std::chrono::milliseconds(update_ms)) {
            std::cout
                << valTemp << '\t'
                << valMagx << '\t'
                << valMagy << '\t'
                << valMagz << '\t'
                << valPressure << '\t'
                << valBat << '\t'
                << valSignal << '\t'
                << std::endl;

            dataSendingDB = true;

            // Perform the update
            stMagnetic->setDouble(1, norm);
            stMagnetic->setDouble(2, norm);
            stMagnetic->execute();

            stSignal->setDouble(1, valSignal);
            stSignal->setDouble(2, valSignal);
            stSignal->execute();

            stBattery->setDouble(1, valBat);
            stBattery->setDouble(2, valBat);
            stBattery->execute();

            dataSentDB = true;
            last_update = std::chrono::steady_clock::now();
          }

          dataSendingZMQ = true;
          // Send the data to ZeroMQ
          zmq::message_t message(255);
          snprintf ((char *) message.data(), 255,
                    "%f %f %f %f %f %f %f", valTemp, valMagx, valMagy, valMagz, valPressure, valBat, valSignal);
          publisher.send(message);
          dataSentZMQ = true;

          // Move data back
          for (int i = 1; i < GRAPH_SIZE; i++) {
            magneticData[i - 1] = magneticData[i];
          }
          magneticData[GRAPH_SIZE - 1] = norm;

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
  } catch (boost::system::system_error &e) {
    std::cerr << "Unable to open interface " << port << ": " << e.what();
  }

  delete stMagnetic;
  delete stSignal;
  delete con;
}

int main() {
  std::cout << "Starting" << std::endl;

  std::thread dataThread(dataAcquisition);

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
  imguiIo.Fonts->AddFontFromFileTTF("../lib/imgui/misc/fonts/DroidSans.ttf", 16.0f);
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
    ImGui::Checkbox("Data", &dataReceived);
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
    dataReceived = false;
    if (dataSentDB) dataSendingDB = false;
    if (dataSentZMQ) dataSendingZMQ = false;
    dataSentDB = false;
    dataSentZMQ = false;

    /*
    glBegin(GL_LINE_LOOP);//start drawing a line loop
    glVertex3f(-1.0f, 0.0f, 0.0f);//left of window
    glVertex3f(0.0f, -1.0f, 0.0f);//bottom of window
    glVertex3f(1.0f, 0.0f, 0.0f);//right of window
    glVertex3f(0.0f, 1.0f, 0.0f);//top of window
    glEnd();//end drawing of line loo
*/
    ImGui::Text("Magnetic Field Strength (uT)");
    ImGui::PlotLines("", magneticData, GRAPH_SIZE, 0, nullptr, 0, FLT_MAX,
                     ImVec2(ImGui::GetContentRegionAvailWidth(), 80));

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
  std::cout << "Thread stopped." << std::endl;

  // Cleanup
  ImGui_ImplGlfwGL3_Shutdown();
  glfwTerminate();

  return 0;
}
