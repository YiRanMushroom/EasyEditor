export module Editor.EditorMain;

import Easy;

using namespace Easy;

namespace Easy {
    /*export class BackGroundLayer : public Layer {
    public:
        BackGroundLayer() : Layer("BackGroundLayer") {}

        void OnUpdate(float) override {
            RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
            RenderCommand::Clear();
        }
    };*/

    export class EditorLayer : public Layer {
    public:
        EditorLayer() : Layer("EditorLayer"), m_EditorCamera() {}

        void OnAttach() override;

        void OnUpdate(float) override;

        void OnImGuiRender() override;

        void RenderViewport();

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
    };
}

export int main(int argc, char *argv[]) {
    auto app = Easy::ApplicationBuilder::Start()
            .Window<OpenGLWindow>()
            .ImGuiLayer<OpenGLImGuiLayer>()
            .CommandLineArgs({argc, argv})
            .Build();

    auto editorLayer = MakeArc<EditorLayer>();
    app->PushLayer(editorLayer);

    // editorLayer->ShowDemoWindow = false;

    app->Run();
}
