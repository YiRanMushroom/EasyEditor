module;

module EditorMain;

import <Core/MacroUtils.hpp>;
import Easy;

namespace Easy {
    void EditorLayer::OnAttach() {
        Layer::OnAttach();
        // 1280x720
        float aspectRatio = 1280.0f / 720.0f;
        m_EditorCamera = OrthographicCamera(-aspectRatio, aspectRatio, -1.0f, 1.0f);
        FramebufferSpecification fbSpec;
        fbSpec.Attachments = {
            FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth
        };
        fbSpec.Width = 1280 / 2;
        fbSpec.Height = 720 / 2;
        m_SceneFramebuffer = Framebuffer::Create(fbSpec);

        m_TopSquareEntity = m_Scene.CreateEntity("Top Square");
        // m_TopSquareEntity.AddComponent<TransformComponent>(glm::vec3(0.0f, 0.5f, 0.0f));
        m_TopSquareEntity.GetComponent<TransformComponent>().Translation = {0.0f, 0.5f, 0.0f};
        m_TopSquareEntity.AddComponent<SpriteRendererComponent>(glm::vec4(1.0f, 0.3f, 1.0f, 1.0f));

        m_BottomSquareEntity = m_Scene.CreateEntity("Bottom Square");
        // m_BottomSquareEntity.AddComponent<TransformComponent>(glm::vec3(0.0f, -0.5f, 0.0f));
        m_BottomSquareEntity.GetComponent<TransformComponent>().Translation = {0.0f, -0.5f, 0.0f};
        m_BottomSquareEntity.AddComponent<SpriteRendererComponent>(glm::vec4(0.3f, 1.0f, 0.3f, 1.0f));
    }

    void EditorLayer::OnUpdate(float x) {
        Layer::OnUpdate(x);
        m_SceneFramebuffer->Bind();

        RenderCommand::SetClearColor({0.3f, 0.3f, 0.3f, 1.0f});
        RenderCommand::Clear();

        Renderer2D::BeginScene(m_EditorCamera);
        OnSceneUpdate(x);
        Renderer2D::EndScene();

        m_SceneFramebuffer->Unbind();
    }


    void RenderProfiles();

    void EditorLayer::OnImGuiRender() {
        Layer::OnImGuiRender();
        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen) {
            const ImGuiViewport *viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        } else {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &p_open, window_flags);
        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO &io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Options")) {
                // Disabling fullscreen would allow the window to be moved to the front of other windows,
                // which we can't undo at the moment without finer window depth/z control.
                ImGui::MenuItem("Fullscreen", nullptr, &opt_fullscreen);
                ImGui::MenuItem("Padding", nullptr, &opt_padding);
                ImGui::Separator();

                if (ImGui::MenuItem("Flag: NoDockingOverCentralNode", "",
                                    (dockspace_flags & ImGuiDockNodeFlags_NoDockingOverCentralNode) != 0)) {
                    dockspace_flags ^= ImGuiDockNodeFlags_NoDockingOverCentralNode;
                }
                if (ImGui::MenuItem("Flag: NoDockingSplit", "",
                                    (dockspace_flags & ImGuiDockNodeFlags_NoDockingSplit) != 0)) {
                    dockspace_flags ^= ImGuiDockNodeFlags_NoDockingSplit;
                }
                if (ImGui::MenuItem("Flag: NoUndocking", "",
                                    (dockspace_flags & ImGuiDockNodeFlags_NoUndocking) != 0)) {
                    dockspace_flags ^= ImGuiDockNodeFlags_NoUndocking;
                }
                if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) {
                    dockspace_flags ^= ImGuiDockNodeFlags_NoResize;
                }
                if (ImGui::MenuItem("Flag: AutoHideTabBar", "",
                                    (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) {
                    dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar;
                }
                if (ImGui::MenuItem("Flag: PassthruCentralNode", "",
                                    (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0,
                                    opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }

                ImGui::MenuItem("Show Demo Window", nullptr, &ShowDemoWindow);

                ImGui::Separator();

                if (ImGui::MenuItem("Close", nullptr, false, p_open != false))
                    p_open = false;
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        ImGui::End();

        if (ShowDemoWindow)
            ImGui::ShowDemoWindow(&ShowDemoWindow);

        RenderViewport();

        RenderJavaTests();

        RenderProfiles();
    }

    void EditorLayer::OnEvent(Event &event) {
        Layer::OnEvent(event);
        EZ_CORE_INFO("Event: {0}", event.ToString());
    }

    void EditorLayer::OnDetach() {
        m_Scene.DestroyEntity(m_TopSquareEntity);
        m_Scene.DestroyEntity(m_BottomSquareEntity);

        Layer::OnDetach();
    }

    void EditorLayer::RenderViewport() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("ViewPort");
        if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered()) {
            Application::Get().GetImGuiLayer()->BlockEvents(false);
        } else {
            Application::Get().GetImGuiLayer()->BlockEvents(true);
        }
        ImGui::PopStyleVar();
        auto currentSize = ImGui::GetContentRegionAvail();

        if (m_SceneFramebuffer->GetColorAttachmentRendererID())
            ImGui::Image(m_SceneFramebuffer->GetColorAttachmentRendererID(),
                         ImVec2(m_ViewportSize.x, m_ViewportSize.y),
                         {0, 1}, {1, 0});

        if (currentSize.x != m_ViewportSize.x || currentSize.y != m_ViewportSize.y) {
            Application::Get().SubmitToMainThread(
                [currentSize, this]() {
                    m_ViewportSize = currentSize;
                    m_SceneFramebuffer->Resize(static_cast<uint32_t>(m_ViewportSize.x),
                                               static_cast<uint32_t>(m_ViewportSize.y));
                    float aspectRatio = m_ViewportSize.x / m_ViewportSize.y;
                    m_EditorCamera.SetProjection(-aspectRatio, aspectRatio, -1.0f, 1.0f);
                });
        }

        ImGui::End();
    }

    void EditorLayer::OnSceneUpdate(float ts) {
        auto view = m_Scene.GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();
        for (const auto &entity: view) {
            auto [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);
            Renderer2D::DrawQuad(transform.Translation, transform.Scale, sprite.Color);
        }
    }

    void EditorLayer::RenderJavaTests() {
        EZ_PROFILE_FUNCTION();
    }

    void EditorLayer::RenderProfiles() {
        ImGui::Begin("ProfileInfo");
        for (const auto &info: g_LastProfileInfos) {
            char buffer[256];
            info.writeTo(buffer);
            ImGui::Text("%s", buffer);
        }
        ImGui::End();
    }
}




/*int main(int argc, char *argv[]) {
    Log::Init();

    ScriptingEngine::Init();

    RunJavaTests();

    ScriptingEngine::Shutdown();

    return 0;
}*/

int main(int argc, char *argv[]) {
    auto app = Easy::ApplicationBuilder::Start()
            .Window<OpenGLWindow>()
            .ImGuiLayer<OpenGLImGuiLayer>()
            .Build();

    auto editorLayer = MakeArc<EditorLayer>();
    app->PushLayer(editorLayer);
    app->GetWindow().SetVSync(true);
    app->Run();
    return 0;
}

/*
void RunJavaTests() {
    using namespace ScriptingEngine;
    using namespace Lib;

    using FunctionType1 = SpecializedKNativeFunctionInterface<JInteger(JInteger)>;
    using FunctionType2 = SpecializedKNativeFunctionInterface<void(JString, JString)>;

    constexpr static Class FunctionTestsDefinition{
        "com/easy/Test/FunctionTests",
        Static{
            Method{"TestInvokeIntInt", Return{FunctionType1::Definition}, Params{}},
            Method{"TestStringStringVoid", Return{FunctionType2::Definition}, Params{}},
        }
    };

    FunctionType1 function(
        Owned{}, static_cast<jobject>(jni::StaticRef<FunctionTestsDefinition>().template Call<"TestInvokeIntInt">()));

    JInteger result = function.Get().value()(JInteger{5});

    FunctionType2 function2(
        Owned{},
        static_cast<jobject>(jni::StaticRef<FunctionTestsDefinition>().template Call<"TestStringStringVoid">()));

    function2.Get().value()(JString{"Hello"}, JString{"World"});

    EZ_ASSERT(result.Get() == 6, "Expected 6, got {}", result.GetOrDefault());
}
*/


// JString space = JString{" "};
//
// auto concat =
//         ScriptingEngine::KNativeFunctions::KNativeFunction<JString(JString, JString)>(
//             [&space](JString str1, JString str2) {
//                 return JString{str1.Get() + space.Get() + str2.Get()};
//             }
//         ).CastToInterface();
// // Scoped.
// {
//     std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
//     for (int i = 0; i < 1000000; i++) {
//         JString res = concat(JString{"Hello"}, JString{"World"});
//         if (i % 10000 == 0) {
//             EZ_CORE_INFO("Concat: {0}", res.Get().c_str());
//             ScriptingEngine::Lib::CallGC();
//         }
//     }
//     std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
//     std::cout << "Time taken for 1000000 concatenations: "
//             << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
// }