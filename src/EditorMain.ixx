module;

#include "Core/MacroUtils.hpp"

export module EditorMain;
import Easy;
import Easy.Editor.SceneHierarchyPanel;

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

        void RenderProfiles();

    public:
        bool ShowDemoWindow = false;

    private:
        bool p_open = true;
        bool opt_fullscreen = true;
        bool opt_padding = false;

        bool is_blocking = false;

        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        Arc<Framebuffer> m_SceneFramebuffer;
        EditorCamera m_EditorCamera;

        ImVec2 m_ViewportSize = {1280.f / 2, 720.f / 2};

        Scene m_Scene;
        Entity m_TopSquareEntity;
        Entity m_BottomSquareEntity;

        Arc<SceneHierarchyPanel> m_SceneHierarchyPanel = MakeArc<SceneHierarchyPanel>();
    };
}


export int main(int argc, char *argv[]);