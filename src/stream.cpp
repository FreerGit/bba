
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

// const std::array<double, 2> from_level(const Level& l) {
//   return { std::stod(l[0]), std::stod(l[1]), }
// }

struct OBEvent {
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
  std::deque<float> best_asks_;
  std::deque<float> best_bids_;

  Stream() : best_asks_({}), best_bids_({}) {
    const Subscribe sub = {.args = {"orderbook.1.BTCUSDT"}};
    websocket.setUrl("wss://stream.bybit.com/v5/public/linear");

    websocket.setOnMessageCallback([sub,
                                    this](const ix::WebSocketMessagePtr& msg) {
      if (msg->type == ix::WebSocketMessageType::Message) {
        // std::cout << msg->str << std::endl;

        Message<OBEvent> update{};
        update.from_json(msg->str);

        if (update.type == "snapshot") {
          this->best_asks_.push_back(std::stof(update.data.a[0][0]));
          this->best_bids_.push_back(std::stof(update.data.b[0][0]));
        } else {
          if (!update.data.a.empty()) {
            const float best_ask = std::stof(update.data.a[0][0]);
            if (this->best_asks_.empty() ||
                best_ask != this->best_asks_.back()) {
              this->best_asks_.push_back(best_ask);
              std::cout << "best ask: " << best_ask << std::endl;
            }
          }

          if (!update.data.b.empty()) {
            const float best_bid = std::stof(update.data.b[0][0]);
            if (this->best_bids_.empty() ||
                best_bid != this->best_bids_.back()) {
              this->best_bids_.push_back(best_bid);
              std::cout << "best bid: " << best_bid << std::endl;
            }
          }
        }

        // std::cout << std::left << std::setw(22) << update.topic <<
        // std::setw(8)
        //           << update.type << std::endl;

        // if (!update.data.b.empty()) {
        //   std::cout << "Bids:" << std::endl;
        //   for (const auto& level : update.data.b) {
        //     std::cout << "- Price: " << level[0] << " | Size: " << level[1]
        //               << std::endl;
        //   }
        // }

        // if (!update.data.a.empty()) {
        //   std::cout << "Asks:" << std::endl;
        //   for (const auto& level : update.data.a) {
        //     std::cout << "- Price: " << level[0] << " | Size: " << level[1]
        //               << std::endl;
        //   }
        // }
        // std::cout << "--------------------" << std::endl;

      } else if (msg->type == ix::WebSocketMessageType::Open) {
        std::cout << "Connection established" << std::endl;
        std::string json =
            glz::write_json(sub).value_or("Error serializing JSON");
        websocket.send(json);
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
    websocket.stop();
    std::cout << "WebSocketClient shut down cleanly." << std::endl;
  }

  auto start() -> void {
    websocket.start();

    running = true;
    heartbeat_thread = std::thread([this]() {
      while (running) {
        websocket.send(R"({"op": "ping"})");
        std::this_thread::sleep_for(std::chrono::seconds(10));
      }
    });
  }

 private:
  ix::WebSocket websocket;
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