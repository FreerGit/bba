
#include <glaze/glaze.hpp>
#include <iostream>
#include <string>

auto main() -> int { return 0; }

// constexpr std::string BASE_URL{"api.bybit.com"};

// struct Symbol {
//   std::string symbol;
//   std::string fundingRate;
// };

// struct Instrument {
//   std::string symbol;
//   int fundingInterval;  // in minutes
// };

// template <typename T>
// struct Market {
//   std::string category;
//   std::vector<T> list;
// };

// template <typename T>
// struct Response {
//   int retCode;
//   std::string retMsg;
//   Market<T> result;

//   void from_json(const std::string &buffer) {
//     const glz::error_ctx e =
//         glz::read<glz::opts{.error_on_unknown_keys = false}>(*this, buffer);
//     if (e.ec != glz::error_code::none) {
//       std::cerr << "JSON: " << glz::format_error(e) << std::endl;
//       assert(false);
//     }
//   }

//   std::string to_json() {
//     std::string json =
//         glz::write_json(*this).value_or("Error serializing JSON");
//     return glz::prettify_json(json);
//   }
// };