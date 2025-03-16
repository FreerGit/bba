#include <GLFW/glfw3.h>
#include <stdio.h>

#include <iostream>
#include <vector>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

static void glfwError(int id, const char* description) {
  std::cout << description << std::endl;
}

void setupGLFW() {
  glfwInit();
  glfwSetErrorCallback(&glfwError);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

int main() {
  setupGLFW();
  GLFWwindow* window =
      glfwCreateWindow(1280, 720, "Bybit Order Book Example", nullptr, nullptr);
  if (!window) {
    fprintf(stderr, "Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");

  std::vector<float> x = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  std::vector<float> bid = {99, 99, 98, 98, 97, 97, 97, 96, 95, 95, 95};
  std::vector<float> ask = {101, 101, 101, 100, 100, 100, 99, 99, 99, 98, 98};

  std::vector<float> trades_x = {2.5f, 5.5f, 7.2f, 9.8f};
  std::vector<float> trades_y = {99.5f, 97.5f, 96.8f, 98.5f};

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Open orders and fills");

    if (ImPlot::BeginPlot("Order Book Plot")) {
      ImPlot::SetNextLineStyle(ImVec4(0.2f, 0.6f, 1.0f, 1.0f));  // blue
      ImPlot::PlotStairs("Best Bid", x.data(), bid.data(), bid.size());

      ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.4f, 0.0f, 1.0f));  // orange
      ImPlot::PlotStairs("Best Ask", x.data(), ask.data(), ask.size());

      ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 6.0f,
                                 ImVec4(0.4f, 1.0f, 0.4f, 1.0f), 1.5f);
      ImPlot::PlotScatter("Trades", trades_x.data(), trades_y.data(),
                          trades_x.size());

      ImPlot::EndPlot();
    }

    ImGui::End();

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  ImPlot::DestroyContext();
  ImGui::DestroyContext();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
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