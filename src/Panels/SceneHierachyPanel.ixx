export module Easy.Editor.SceneHierarchyPanel;

import Easy.Core.Basic;
import Easy.ImGui.ImGui;
import Easy.Scene;

namespace Easy {
    export class SceneHierarchyPanel {
    public:
        virtual ~SceneHierarchyPanel() = default;

        SceneHierarchyPanel() = default;

        SceneHierarchyPanel(Scene *scene) : m_Scene(scene) {}

        void SetContext(Scene *scene) { m_Scene = scene; }

        virtual void OnImGuiRender();

    private:
        void DrawEntityNode(Entity entity);

        void DrawComponents(Entity entity);

    private:
        Scene *m_Scene = nullptr;
        Entity m_SelectionContext;
        Entity m_UnselectedContext;
    };
}
