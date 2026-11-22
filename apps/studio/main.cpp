// ciph-studio: node graph editor and debugger for ciph.
//
// Current state is a vertical slice: a fixed three node graph
//
//     begin -> script -> end
//
// where the script node holds ciph source. "Compile" runs ciph-compiler on it
// and shows the result; "Run" opens ciph-vm's stepper on the compiled program.
// Both tools are started as separate programs and are never linked in.
//
// Command line:
//   --compile     compile the script once at startup
//   --frames N    render N frames and exit; used to smoke-test the build

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>

#include <imgui.h>
#include <imgui_internal.h> // DockBuilder
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imnodes.h>

#include <GLFW/glfw3.h>

#include <ciph/platform/executable_path.hpp>

#include "build_session.hpp"
#include "source_panel.hpp"

namespace {

using ciph::studio::BuildSession;
using ciph::studio::BuildState;
using ciph::studio::SourcePanel;

// The script the slice starts with: the most involved program the compiler
// and VM handle today. The loop body runs before the condition is checked.
const char* const default_script = "let i = 0;\n"
                                   "while (i < 5) {\n"
                                   "    i++;\n"
                                   "}\n"
                                   "return i;\n";

// Ids are arbitrary but must be unique per kind (node / pin / link).
enum Node { node_begin = 1, node_script, node_end };
enum Pin { pin_begin_out = 1, pin_script_in, pin_script_out, pin_end_in };

void
glfw_error_callback(int error, const char* description) {
    std::fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

const char*
state_name(BuildState state) {
    switch (state) {
        case BuildState::idle: return "not compiled";
        case BuildState::compiling: return "compiling";
        case BuildState::succeeded: return "compiled";
        case BuildState::failed: return "failed";
    }
    return "?";
}

void
draw_graph(bool first_frame) {
    if (first_frame) {
        ImNodes::SetNodeGridSpacePos(node_begin, ImVec2(60.0f, 120.0f));
        ImNodes::SetNodeGridSpacePos(node_script, ImVec2(240.0f, 100.0f));
        ImNodes::SetNodeGridSpacePos(node_end, ImVec2(440.0f, 120.0f));
    }

    auto title = [](const char* text) {
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted(text);
        ImNodes::EndNodeTitleBar();
    };

    ImNodes::BeginNode(node_begin);
    title("begin");
    ImNodes::BeginOutputAttribute(pin_begin_out);
    ImGui::TextUnformatted("start");
    ImNodes::EndOutputAttribute();
    ImNodes::EndNode();

    ImNodes::BeginNode(node_script);
    title("script");
    ImNodes::BeginInputAttribute(pin_script_in);
    ImGui::TextUnformatted("in");
    ImNodes::EndInputAttribute();
    ImNodes::BeginOutputAttribute(pin_script_out);
    ImGui::TextUnformatted("result");
    ImNodes::EndOutputAttribute();
    ImNodes::EndNode();

    ImNodes::BeginNode(node_end);
    title("end");
    ImNodes::BeginInputAttribute(pin_end_in);
    ImGui::TextUnformatted("result");
    ImNodes::EndInputAttribute();
    ImNodes::EndNode();

    ImNodes::Link(1, pin_begin_out, pin_script_in);
    ImNodes::Link(2, pin_script_out, pin_end_in);
}

// Graph on the left with the output underneath, node source on the right.
void
build_default_layout(ImGuiID dockspace) {
    ImGuiID right = 0;
    ImGuiID left = 0;
    ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Right, 0.38f, &right, &left);

    ImGuiID bottom = 0;
    ImGuiID top = 0;
    ImGui::DockBuilderSplitNode(left, ImGuiDir_Down, 0.35f, &bottom, &top);

    ImGui::DockBuilderDockWindow("Graph", top);
    ImGui::DockBuilderDockWindow("Output", bottom);
    ImGui::DockBuilderDockWindow("Node source", right);
    ImGui::DockBuilderFinish(dockspace);
}

} // namespace

int
main(int argc, char* argv[]) {
    long frame_limit = -1;
    bool compile_on_start = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--frames") == 0 && i + 1 < argc) {
            frame_limit = std::strtol(argv[++i], nullptr, 10);
        }
        else if (std::strcmp(argv[i], "--compile") == 0) {
            compile_on_start = true;
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

    // The tools sit next to this executable; scratch files go in the system
    // temp folder.
    BuildSession build(ciph::platform::executable_path().parent_path(),
                       std::filesystem::temp_directory_path() / "ciph-studio");

    SourcePanel source_panel(build.query_keywords());
    source_panel.set_text(default_script);

    BuildState logged_state = build.state();

    bool first_frame = true;
    long frame = 0;
    while (!glfwWindowShouldClose(window) && (frame_limit < 0 || frame < frame_limit)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const ImGuiID dockspace = ImGui::DockSpaceOverViewport();
        if (first_frame) {
            // Default layout, unless imgui.ini already restored one.
            const ImGuiDockNode* root = ImGui::DockBuilderGetNode(dockspace);
            if (root != nullptr && !root->IsSplitNode() && root->Windows.Size == 0) {
                build_default_layout(dockspace);
            }
        }

        // ---- Build state ----

        const std::string script = source_panel.text();
        if (first_frame && compile_on_start) {
            build.compile(script);
        }
        build.update();

        // Run is only offered for a compile of exactly what is in the editor.
        const bool script_changed = script != build.compiled_source();
        const bool can_run = build.state() == BuildState::succeeded && !script_changed;

        // ---- Graph ----

        ImGui::Begin("Graph");

        ImGui::BeginDisabled(build.state() == BuildState::compiling);
        if (ImGui::Button("Compile")) {
            build.compile(script);
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::BeginDisabled(!can_run);
        if (ImGui::Button("Run")) {
            build.run();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        if (build.state() == BuildState::succeeded && script_changed) {
            ImGui::TextDisabled("edited since last compile");
        }
        else {
            ImGui::TextDisabled("%s", state_name(build.state()));
        }

        ImNodes::BeginNodeEditor();
        draw_graph(first_frame);
        ImNodes::EndNodeEditor();
        if (first_frame) {
            ImNodes::SelectNode(node_script);
        }
        int selected_node = 0;
        if (ImNodes::NumSelectedNodes() == 1) {
            ImNodes::GetSelectedNodes(&selected_node);
        }
        ImGui::End();

        // ---- Node source ----

        if (selected_node == node_script) {
            source_panel.draw_editor("script");
        }
        else if (selected_node == 0) {
            source_panel.draw_message("Select the script node to edit its source.");
        }
        else {
            source_panel.draw_message("This node has no source.");
        }

        // ---- Output ----

        ImGui::Begin("Output");
        if (build.state() == BuildState::failed) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.45f, 0.45f, 1.0f));
            ImGui::TextUnformatted(build.output().c_str());
            ImGui::PopStyleColor();
        }
        else {
            ImGui::TextUnformatted(build.output().c_str());
        }
        ImGui::End();

        // Mirror build results on the console, for running without the UI.
        if (build.state() != logged_state) {
            logged_state = build.state();
            if (logged_state == BuildState::succeeded || logged_state == BuildState::failed) {
                std::printf("build %s\n%s\n", state_name(logged_state), build.output().c_str());
                std::fflush(stdout);
            }
        }

        // ---- Render ----

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
