module;

#include "Core/MacroUtils.hpp"

export module EditorMain;

import Easy;
using namespace Easy;

namespace Easy {
    class EditorLayer : public Layer {
    public:
        EditorLayer() : Layer("EditorLayer"), m_EditorCamera() {}

        void OnAttach() override;

        void OnUpdate(float) override;

        void OnImGuiRender() override;

        void OnEvent(Event &) override;

        void OnDetach() override;

        void RenderViewport();

        void OnSceneUpdate(float ts);

    public:
        bool ShowDemoWindow = false;

    private:
        bool p_open = true;
        bool opt_fullscreen = true;
        bool opt_padding = false;
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        Arc<Easy::Framebuffer> m_SceneFramebuffer;
        OrthographicCamera m_EditorCamera;

        ImVec2 m_ViewportSize = {1280.f / 2, 720.f / 2};

        Scene m_Scene;
        Entity m_TopSquareEntity;
        Entity m_BottomSquareEntity;
    };
}

/*export int main(int argc, char *argv[]) {
    auto app = Easy::ApplicationBuilder::Start()
            .Window<OpenGLWindow>()
            .ImGuiLayer<OpenGLImGuiLayer>()
            .CommandLineArgs({argc, argv})
            .Build();

    auto editorLayer = MakeArc<EditorLayer>();
    app->PushLayer(editorLayer);
    app->Run();
}*/

void providedNativePrintln
(JNIEnv *env, jclass clazz, jstring message) {
    const char *str = env->GetStringUTFChars(message, nullptr);
    std::cout << "Native print: " << str << std::endl;
    env->ReleaseStringUTFChars(message, str);
}

export int main();
