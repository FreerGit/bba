#include <GLFW/glfw3.h>
#include <stdio.h>

#include <iostream>
#include <vector>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include "stream.cpp"

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

  // std::vector<float> x = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  // std::vector<float> bid = {99, 99, 98, 98, 97, 97, 97, 96, 95, 95, 95};
  // std::vector<float> ask = {101, 101, 101, 100, 100, 100, 99, 99, 99, 98,
  // 98};

  // std::vector<float> trades_x = {2.5f, 5.5f, 7.2f, 9.8f};
  // std::vector<float> trades_y = {99.5f, 97.5f, 96.8f, 98.5f};

  Stream s = {};
  s.start();

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Open orders and fills");

    ImVec2 plot_size = ImGui::GetContentRegionAvail();

    std::vector<float> bid_vector(s.best_bids_.begin(), s.best_bids_.end());
    std::vector<float> ask_vector(s.best_asks_.begin(), s.best_asks_.end());

    // ImPlot::SetNextAxesToFit();
    if (ImPlot::BeginPlot("Order Book Plot", plot_size)) {
      static double x_min = 0, x_max = 10;
      double y_min = 0, y_max = 0;
      // ImPlot::FitNextPlotAxes();
      // Generate x-axis values based on size
      std::vector<float> x_axis(s.best_bids_.size());
      for (size_t i = 0; i < x_axis.size(); i++) {
        x_axis[i] = static_cast<float>(i);  // Sequential index
      }

      if (!x_axis.empty()) {
        x_max = x_axis.back();
        x_min = x_max -
                100;  // Always keep the latest `x_range` data points in view
      }

      static double prev_y_min = 0, prev_y_max = 0;
      static double smoothing_factor =
          0.2;  // Adjust between 0 (no smoothing) and 1 (instant update)

      if (!bid_vector.empty() && !ask_vector.empty()) {
        double min_bid =
            *std::min_element(bid_vector.begin(), bid_vector.end());
        double max_bid =
            *std::max_element(bid_vector.begin(), bid_vector.end());
        double min_ask =
            *std::min_element(ask_vector.begin(), ask_vector.end());
        double max_ask =
            *std::max_element(ask_vector.begin(), ask_vector.end());

        double new_y_min = std::min(min_bid, min_ask);
        double new_y_max = std::max(max_bid, max_ask);

        // Cap the margin so it doesn't fluctuate too much
        double margin = std::clamp((new_y_max - new_y_min) * 0.05, 0.1, 2.0);
        new_y_min -= margin;
        new_y_max += margin;

        // Apply smoothing to prevent sudden jumps
        y_min =
            prev_y_min * (1 - smoothing_factor) + new_y_min * smoothing_factor;
        y_max =
            prev_y_max * (1 - smoothing_factor) + new_y_max * smoothing_factor;

        prev_y_min = y_min;
        prev_y_max = y_max;
      }

      ImPlot::SetupAxesLimits(x_min, x_max + 10, y_min, y_max,
                              ImGuiCond_Always);

      // Best Bid
      ImPlot::SetNextLineStyle(ImVec4(0.2f, 0.6f, 1.0f, 1.0f));  // Blue
      ImPlot::PlotStairs("Best Bid", x_axis.data(), bid_vector.data(),
                         bid_vector.size());

      // Best Ask
      ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.4f, 0.0f, 1.0f));  // Orange
      ImPlot::PlotStairs("Best Ask", x_axis.data(), ask_vector.data(),
                         ask_vector.size());

      // // Trades
      // ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 6.0f,
      //                            ImVec4(0.4f, 1.0f, 0.4f, 1.0f), 1.5f);
      // ImPlot::PlotScatter("Trades", trades_x.data(), trades_y.data(),
      //                     trades_x.size());

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
