#include <base64.h>
#include <csignal>
#include <exception>
#include <iostream>
#include <netinet/in.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace server {
using json = nlohmann::json;

class TCPServer {
private:
  static std::string call_compiler(const std::string &source_code) {
    // *** TODO ***
    // Call your compiler here and return the compiled executable.
    // Raise an exception on compilation error.
    // *** TODO ***
    throw std::runtime_error("Unimplemented");
  }

  static void handler(int client_socket) {
    char buffer[4096];
    ssize_t n = recv(client_socket, buffer, sizeof(buffer), 0);

    if (n <= 0) {
      close(client_socket);
      return;
    }

    std::string input_str(buffer, n);
    json result;
    try {
      json input = json::parse(input_str);
      if (input["command"] == "compile") {
        std::string source_code = input["code"];
        auto executable = call_compiler(source_code);
        result["program"] = base64_encode(executable, false);
      } else if (input["command"] == "ping") {
        // {}
      } else {
        result["error"] =
            "Unknown command: " + input["command"].get<std::string>();
      }
    } catch (const std::exception &e) {
      result["error"] = e.what();
    }

    std::string res_str = result.dump();
    send(client_socket, res_str.data(), res_str.size(), 0);
    close(client_socket);
  }

public:
  void run_server(int host, int port) {
    int server = socket(AF_INET, SOCK_STREAM, 0);

    if (server < 0)
      throw std::runtime_error("socket() failed");

    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = host;

    if (bind(server, (struct sockaddr *)&address, sizeof(address)) < 0)
      throw std::runtime_error("bind() failed");

    if (listen(server, 32) > 0)
      throw std::runtime_error("listen() failed");

    std::cout << "Starting TCP server at " << host << ":" << "port"
              << std::endl;

    while (true) {
      int client_socket = accept(server, nullptr, nullptr);
      if (client_socket < 0)
        continue;
      std::thread(handler, client_socket).detach();
    }
  }
};
} // namespace server
