// ciph-studio: node graph editor and debugger for ciph.
//
// For now this only opens a window with a hard-coded sample graph, to prove
// the Dear ImGui + imnodes + GLFW/OpenGL toolchain builds and runs.

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imnodes.h>

#include <GLFW/glfw3.h>

namespace {

void
glfw_error_callback(int error, const char* description) {
    std::fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

// Placeholder graph mirroring:
//
//   graph heuristic(a, b, d) {
//       let c = check(b)
//       let q = div(c, d)
//       return ne(a, q)
//   }
//
// Ids are arbitrary but must be unique per kind (node / pin / link).
void
draw_sample_graph(bool first_frame) {
    enum Node { inputs = 1, check, div, ne, result };
    enum Pin {
        in_a = 1, in_b, in_d,
        check_b, check_out,
        div_lhs, div_rhs, div_out,
        ne_lhs, ne_rhs, ne_out,
        result_in
    };

    if (first_frame) {
        ImNodes::SetNodeGridSpacePos(inputs, ImVec2(40.0f, 80.0f));
        ImNodes::SetNodeGridSpacePos(check, ImVec2(220.0f, 120.0f));
        ImNodes::SetNodeGridSpacePos(div, ImVec2(400.0f, 140.0f));
        ImNodes::SetNodeGridSpacePos(ne, ImVec2(580.0f, 80.0f));
        ImNodes::SetNodeGridSpacePos(result, ImVec2(760.0f, 80.0f));
    }

    auto title = [](const char* text) {
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted(text);
        ImNodes::EndNodeTitleBar();
    };
    auto input = [](int id, const char* text) {
        ImNodes::BeginInputAttribute(id);
        ImGui::TextUnformatted(text);
        ImNodes::EndInputAttribute();
    };
    auto output = [](int id, const char* text) {
        ImNodes::BeginOutputAttribute(id);
        ImGui::TextUnformatted(text);
        ImNodes::EndOutputAttribute();
    };

    ImNodes::BeginNode(inputs);
    title("inputs");
    output(in_a, "a");
    output(in_b, "b");
    output(in_d, "d");
    ImNodes::EndNode();

    ImNodes::BeginNode(check);
    title("check");
    input(check_b, "b");
    output(check_out, "c");
    ImNodes::EndNode();

    ImNodes::BeginNode(div);
    title("div");
    input(div_lhs, "lhs");
    input(div_rhs, "rhs");
    output(div_out, "q");
    ImNodes::EndNode();

    ImNodes::BeginNode(ne);
    title("ne");
    input(ne_lhs, "lhs");
    input(ne_rhs, "rhs");
    output(ne_out, "out");
    ImNodes::EndNode();

    ImNodes::BeginNode(result);
    title("return");
    input(result_in, "value");
    ImNodes::EndNode();

    int link = 0;
    ImNodes::Link(++link, in_b, check_b);
    ImNodes::Link(++link, check_out, div_lhs);
    ImNodes::Link(++link, in_d, div_rhs);
    ImNodes::Link(++link, in_a, ne_lhs);
    ImNodes::Link(++link, div_out, ne_rhs);
    ImNodes::Link(++link, ne_out, result_in);
}

} // namespace

int
main(int argc, char* argv[]) {
    // `--frames N` renders N frames and exits; used to smoke-test the build.
    long frame_limit = -1;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            frame_limit = std::strtol(argv[++i], nullptr, 10);
        }
    }

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return 1;
    }

    // OpenGL 3.2 core: the lowest version macOS offers a core profile for.
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "ciph-studio", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImNodes::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();
    ImNodes::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool first_frame = true;
    long frame = 0;
    while (!glfwWindowShouldClose(window) && (frame_limit < 0 || frame < frame_limit)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport();

        ImGui::Begin("Graph");
        ImNodes::BeginNodeEditor();
        draw_sample_graph(first_frame);
        ImNodes::MiniMap(0.15f, ImNodesMiniMapLocation_BottomRight);
        ImNodes::EndNodeEditor();
        ImGui::End();

        ImGui::Render();

        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.10f, 0.10f, 0.11f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        first_frame = false;
        ++frame;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImNodes::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
