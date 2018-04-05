#include <zmq.hpp>
#include <iostream>
#include <sstream>

int main (int argc, char *argv[])
{
  zmq::context_t context (1);

  //  Socket to talk to server
  std::cout << "Collecting updates from Cubesat server…\n" << std::endl;
  zmq::socket_t subscriber (context, ZMQ_SUB);
  subscriber.connect("tcp://localhost:5555");

  // Receive all incoming messages
  subscriber.setsockopt(ZMQ_SUBSCRIBE, nullptr, 0);

  while (true) {
    zmq::message_t update;
    subscriber.recv(&update);

    std::istringstream iss(static_cast<char*>(update.data()));
    std::cout << iss.str() << std::endl;
  }
}
