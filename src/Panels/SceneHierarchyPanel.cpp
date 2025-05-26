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
            m_SelectionContext = {};
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

        ImGui::SetNextItemOpen(false);
        bool opened = ImGui::TreeNodeEx(entity,
                                         (m_SelectionContext == entity ? ImGuiTreeNodeFlags_Selected : 0),
                                        tag.Tag.c_str());
        if (ImGui::IsItemClicked()) {
            if (m_SelectionContext == entity) {
                m_SelectionContext = {};
            } else {
                m_SelectionContext = entity;
            }
        }

        if (opened)
            ImGui::TreePop();
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
            return reinterpret_cast<const void *>(typeid(T).hash_code());
        }

        template<>
        void DrawComponentImpl<TagComponent>(Entity entity) {
            if (!entity.HasComponent<TagComponent>()) {
                return;
            }
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
            if (!entity.HasComponent<TransformComponent>()) {
                return;
            }
            auto &transform = entity.GetComponent<TransformComponent>();
            if (ImGui::TreeNodeEx(TypeHashOf<TransformComponent>(), ImGuiTreeNodeFlags_DefaultOpen, "Transform")) {
                ImGui::DragFloat3("Position", &transform.Translation.x, 0.1f);
                ImGui::DragFloat3("Rotation", &transform.Rotation.x, 0.1f);
                ImGui::DragFloat3("Scale", &transform.Scale.x, 0.1f);
                ImGui::TreePop();
            }
        }

        // camera
        template<>
        void DrawComponentImpl<CameraComponent>(Entity entity) {
            if (!entity.HasComponent<CameraComponent>()) {
                return;
            }
            auto &camera = entity.GetComponent<CameraComponent>();
            if (ImGui::TreeNodeEx(TypeHashOf<CameraComponent>(), ImGuiTreeNodeFlags_DefaultOpen, "Camera")) {
                // perspective / orthographic
                const char *items[] = {"Perspective", "Orthographic"};
                const char *currentItem = items[static_cast<int>(camera.Camera.GetProjectionType())];
                if (ImGui::BeginCombo("Projection", currentItem)) {
                    for (int i = 0; i < 2; i++) {
                        bool isSelected = (currentItem == items[i]);
                        if (ImGui::Selectable(items[i], isSelected)) {
                            camera.Camera.SetProjectionType(static_cast<SceneCamera::ProjectionType>(i));
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }

                if (camera.Camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective) {
                    float verticalFOV = camera.Camera.GetPerspectiveVerticalFOV();
                    if (ImGui::DragFloat("Vertical FOV", &verticalFOV, 0.1f)) {
                        camera.Camera.SetPerspectiveVerticalFOV(verticalFOV);
                    }
                    float nearClip = camera.Camera.GetPerspectiveNearClip();
                    if (ImGui::DragFloat("Near Clip", &nearClip, 0.1f)) {
                        camera.Camera.SetPerspectiveNearClip(nearClip);
                    }
                    float farClip = camera.Camera.GetPerspectiveFarClip();
                    if (ImGui::DragFloat("Far Clip", &farClip, 0.1f)) {
                        camera.Camera.SetPerspectiveFarClip(farClip);
                    }
                } else {
                    float size = camera.Camera.GetOrthographicSize();
                    if (ImGui::DragFloat("Size", &size, 0.1f)) {
                        camera.Camera.SetOrthographicSize(size);
                    }
                    float nearClip = camera.Camera.GetOrthographicNearClip();
                    if (ImGui::DragFloat("Near Clip", &nearClip, 0.1f)) {
                        camera.Camera.SetOrthographicNearClip(nearClip);
                    }
                    float farClip = camera.Camera.GetOrthographicFarClip();
                    if (ImGui::DragFloat("Far Clip", &farClip, 0.1f)) {
                        camera.Camera.SetOrthographicFarClip(farClip);
                    }
                    ImGui::Checkbox("Fixed Aspect Ratio", &camera.FixedAspectRatio);
                }

                ImGui::TreePop();
            }
        }
    }

    void SceneHierarchyPanel::DrawComponents(Entity entity) {
        Util::DrawComponentGroup<Util::AllImGuiPropertyRenderComponents>(entity);
    }
}
