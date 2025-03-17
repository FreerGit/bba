
#include <cstdint>
#include <glaze/glaze.hpp>
#include <iostream>
#include <string>
#include <vector>

#include "ixwebsocket/IXWebSocket.h"

struct Subscribe {
  std::string op = "subscribe";
  std::vector<std::string> args;
};

using Level = std::array<std::string, 2>;

struct OBUpdate {
  std::string s;
  std::vector<Level> b;
  std::vector<Level> a;
};

template <typename T>
struct Message {
  std::string topic;
  std::string type;
  uint64_t cts;
  T data;

  void from_json(const std::string& buffer) {
    const glz::error_ctx e =
        glz::read<glz::opts{.error_on_unknown_keys = false}>(*this, buffer);
    if (e.ec != glz::error_code::none) {
      std::cerr << "JSON: " << glz::format_error(e) << std::endl;
      assert(false);
    }
  }

  std::string to_json() {
    std::string json =
        glz::write_json(*this).value_or("Error serializing JSON");
    return glz::prettify_json(json);
  }
};

class Stream {
 public:
  Stream() {
    const Subscribe sub = {.args = {"orderbook.1.BTCUSDT"}};
    webSocket.setUrl("wss://stream.bybit.com/v5/public/linear");

    webSocket.setOnMessageCallback([sub,
                                    this](const ix::WebSocketMessagePtr& msg) {
      if (msg->type == ix::WebSocketMessageType::Message) {
        // std::cout << msg->str << std::endl;

        Message<OBUpdate> update{};
        update.from_json(msg->str);
        std::cout << std::left << std::setw(22) << update.topic << std::setw(8)
                  << update.type << std::endl;

        if (!update.data.b.empty()) {
          std::cout << "Bids:" << std::endl;
          for (const auto& level : update.data.b) {
            std::cout << "- Price: " << level[0] << " | Size: " << level[1]
                      << std::endl;
          }
        }

        if (!update.data.a.empty()) {
          std::cout << "Asks:" << std::endl;
          for (const auto& level : update.data.a) {
            std::cout << "- Price: " << level[0] << " | Size: " << level[1]
                      << std::endl;
          }
        }
        std::cout << "--------------------" << std::endl;

      } else if (msg->type == ix::WebSocketMessageType::Open) {
        std::cout << "Connection established" << std::endl;
        std::string json =
            glz::write_json(sub).value_or("Error serializing JSON");
        webSocket.send(json);
      } else if (msg->type == ix::WebSocketMessageType::Error) {
        std::cout << "Connection error: " << msg->errorInfo.reason << std::endl;
      }
    });
  };

  ~Stream() {
    running = false;
    if (heartbeat_thread.joinable()) {
      heartbeat_thread.join();
    }
    webSocket.stop();
    std::cout << "WebSocketClient shut down cleanly." << std::endl;
  }

  auto start() -> void {
    webSocket.start();

    running = true;
    heartbeat_thread = std::thread([this]() {
      while (running) {
        webSocket.send(R"({"op": "ping"})");
        std::this_thread::sleep_for(std::chrono::seconds(10));
      }
    });
  }

 private:
  ix::WebSocket webSocket;
  std::atomic<bool> running{false};
  std::thread heartbeat_thread;
  //   Response<Symbol> symbols_{};

  //   auto buildSubscribe() -> Subscribe {
  //     cpr::Response r =
  //         cpr::Get(cpr::Url(BASE_URL + "/v5/market/tickers"),
  //                  cpr::Parameters{{"category", "linear"}, {"limit",
  //                  "1000"}});

  //     symbols_.from_json(r.text);

  //     Subscribe sub{};

  //     for (const auto& s : symbols_.result.list) {
  //       sub.args.push_back("allLiquidation." + s.symbol);
  //     }
  //     return sub;
  //   }
};