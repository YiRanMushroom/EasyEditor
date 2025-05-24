module Easy.Editor.SceneHierarchyPanel;

import Easy.Core.Basic;
import Easy.ImGui.ImGui;
import Easy.Scene;

namespace Easy {
    void SceneHierarchyPanel::OnImGuiRender() {
        ImGui::Begin("Scene Hierarchy");
        auto &registry = m_Scene->GetRegistry();

        registry.each([&](entt::entity entityID) {
            auto entity = Entity(entityID, m_Scene);
            DrawEntityNode(entity);
        });

        if (ImGui::IsMouseClicked(0) && ImGui::IsWindowHovered()) {
            m_UnselectedContext = m_SelectionContext;
        }

        ImGui::End();

        // Properties Panel
        if (m_SelectionContext) {
            bool open = true;

            ImGui::Begin("Properties", &open);
            if (m_SelectionContext) {
                DrawComponents(m_SelectionContext);
            }
            ImGui::End();

            if (!open) {
                m_SelectionContext = {};
            }
        }
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity) {
        const auto &tag = entity.GetComponent<TagComponent>();

        if (m_UnselectedContext == entity) {
            ImGui::SetNextItemOpen(false);
            m_SelectionContext = {};
            m_UnselectedContext = {};
        }

        if (ImGui::TreeNodeEx(entity,
                              ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow
                              |ImGuiTreeNodeFlags_OpenOnDoubleClick
                              | (m_SelectionContext == entity ? ImGuiTreeNodeFlags_Selected : 0),
                              tag.Tag.c_str())) {
            if (ImGui::IsItemClicked()) {
                m_SelectionContext = entity;
            }

            ImGui::TreePop();
        } else {
            if (ImGui::IsItemClicked()) {
                m_UnselectedContext = entity;
            }
        }
    }

    namespace Util {
        using AllImGuiPropertyRenderComponents = ComponentGroup<TagComponent, TransformComponent,
            SpriteRendererComponent, CircleRendererComponent, CameraComponent, ScriptComponent,
            NativeScriptComponent, Rigidbody2DComponent>;

        template<typename>
        void DrawComponentImpl(Entity entity) {}

        template<typename>
        struct DrawComponentGroupImpl;

        template<typename... Ts>
        struct DrawComponentGroupImpl<ComponentGroup<Ts...>> {
            static void Draw(Entity entity) {
                (DrawComponentImpl<Ts>(entity), ...);
            }
        };

        template<typename T>
        void DrawComponentGroup(Entity entity) {
            DrawComponentGroupImpl<T>::Draw(entity);
        }

        template<typename T>
        const void *TypeHashOf() {
            return reinterpret_cast<const void*>(typeid(T).hash_code());
        }

        template<>
        void DrawComponentImpl<TagComponent>(Entity entity) {
            auto &tag = entity.GetComponent<TagComponent>();
            if (ImGui::TreeNodeEx(TypeHashOf<TagComponent>(), ImGuiTreeNodeFlags_DefaultOpen, "Tag")) {
                char buffer[256];
                std::strcpy(buffer, tag.Tag.c_str());
                if (ImGui::InputText("Tag", buffer, sizeof(buffer))) {
                    tag.Tag = buffer;
                }
                ImGui::TreePop();
            }
        }

        template<>
        void DrawComponentImpl<TransformComponent>(Entity entity) {
            auto &transform = entity.GetComponent<TransformComponent>();
            if (ImGui::TreeNodeEx(TypeHashOf<TransformComponent>(), ImGuiTreeNodeFlags_DefaultOpen, "Transform")) {
                ImGui::DragFloat3("Position", &transform.Translation.x, 0.1f);
                ImGui::DragFloat3("Rotation", &transform.Rotation.x, 0.1f);
                ImGui::DragFloat3("Scale", &transform.Scale.x, 0.1f);
                ImGui::TreePop();
            }
        }
    }

    void SceneHierarchyPanel::DrawComponents(Entity entity) {
        Util::DrawComponentGroup<Util::AllImGuiPropertyRenderComponents>(entity);
    }
}
