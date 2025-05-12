module;

#include <Core/MacroUtils.hpp>

module EditorMain;

import Easy;
import Easy.Scripting.JniBind;
import Easy.Scripting.JTypes;
import Easy.Scripting.KNativeFunctions;

using namespace jni;

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

        constexpr static Class ImGuiDefinition{
            "com/easy/Test/ImGuiTests",
            Constructor{},
            Method{"Render", Return{}, Params{}}
        };

        static GlobalObject<ImGuiDefinition> ImGuiTests{};

        ImGuiTests.Call<"Render">();
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

void RunJavaTests();


using namespace ScriptingEngine::JTypes;

/*struct TestStruct : public InjectJObject {
public:
    constexpr static StringLiteral SimpleName = "com/easy/Test/BasicTests";
    constexpr static StringLiteral FullName = ScriptingEngine::JTypes::MakeFullName(SimpleName);

    constexpr static Class Definition{
        "com/easy/Test/BasicTests",
    };


    [[nodiscard]] jobject ToJava() const {
        return m_Ref.GetRawObject();
    }

    TestStruct() : m_Ref{
        s_Constructor3.Invoke(ScriptingEngine::GetEnv(), s_ClassRef.GetObjectAs<jclass>())
    } {
        EZ_CORE_ASSERT(m_Ref.NotNull(), "Failed to create TestStruct");
    }

    TestStruct(jobject obj) : m_Ref{obj} {
        EZ_CORE_ASSERT(m_Ref.NotNull(), "Failed to create TestStruct");
    }

    TestStruct(JString str, Jint i, JDouble d)
        : TestStruct(s_Constructor1.Invoke(ScriptingEngine::GetEnv(),
                                           s_ClassRef.GetObjectAs<jclass>(), str, i, d)) {
        EZ_CORE_ASSERT(m_Ref.NotNull(), "Failed to create TestStruct");
    }

    TestStruct(Jint i) : TestStruct(s_Constructor2.Invoke(ScriptingEngine::GetEnv(),
                                                          s_ClassRef.GetObjectAs<jclass>(), i)) {
        EZ_CORE_ASSERT(m_Ref.NotNull(), "Failed to create TestStruct");
    }

    TestStruct(JString str) : TestStruct(s_Constructor4.Invoke(ScriptingEngine::GetEnv(),
                                                               s_ClassRef.GetObjectAs<jclass>(),
                                                               str)) {
        EZ_CORE_ASSERT(m_Ref.NotNull(), "Failed to create TestStruct");
    }

    static void Init() {
        s_ClassRef.SetFrom(ScriptingEngine::Lib::GetClass("com.easy.Test.BasicTests"));

        s_Constructor1.Init(s_ClassRef.GetObjectAs<jclass>());
        s_Constructor2.Init(s_ClassRef.GetObjectAs<jclass>());
        s_Constructor3.Init(s_ClassRef.GetObjectAs<jclass>());
        s_Constructor4.Init(s_ClassRef.GetObjectAs<jclass>());
        s_ToStringMethod.Init(s_ClassRef.GetObjectAs<jclass>(), "ToString");

        s_StaticCreateMethod.Init(s_ClassRef.GetObjectAs<jclass>(), "Create");
    }

    static TestStruct Create(std::string str, int i, double d) {
        return s_StaticCreateMethod.Invoke(ScriptingEngine::GetEnv(),
                                           s_ClassRef.GetObjectAs<jclass>(),
                                           JString{str},
                                           Jint{i},
                                           Jdouble{d});
    }

    [[nodiscard]] JString ToString() const {
        return JString{
            NewRef{}, s_ToStringMethod.Invoke(ScriptingEngine::GetEnv(), m_Ref.GetObjectAs<jobject>())
        };
    }

private:
    ScriptingEngine::JavaGlobalArc<Definition> m_Ref;

private:
    inline static ScriptingEngine::JavaGlobalArc<JTClass> s_ClassRef;

    inline static ScriptingEngine::JConstructor<TestStruct(JString, Jint, JDouble)> s_Constructor1;
    inline static ScriptingEngine::JConstructor<TestStruct(Jint)> s_Constructor2;
    inline static ScriptingEngine::JConstructor<TestStruct()> s_Constructor3;
    inline static ScriptingEngine::JConstructor<TestStruct(JString)> s_Constructor4;
    inline static ScriptingEngine::JInstanceMethod<JString(TestStruct)> s_ToStringMethod;
    inline static ScriptingEngine::JStaticMethod<TestStruct(JString, Jint, Jdouble)> s_StaticCreateMethod;
};*/

struct TestReportIntNativeBuffer : ScriptingEngine::AutoManagedBufferBase {
    int value;

    virtual ~TestReportIntNativeBuffer() override {
        // EZ_CORE_INFO("TestReportIntNativeBuffer destructor called: {}", value);
    }

    TestReportIntNativeBuffer(int v) : value(v) {}
};

void RunJavaTests() {

    // ScriptingEngine::KNativeFunctions::KNativeFunction<JInteger(JInteger, JFloat)> function(
    //     [](JInteger i, JFloat f)-> JInteger {
    //         return i.Get().value() * f.Get().value();
    //     }
    // );
    //
    // for (int i = 0; i < 100000; ++i) {
    //     float f = rand() / 10000000;
    //     JInteger result = function(JInteger{i}, JFloat{f});
    //     EZ_ASSERT(result.Get() == i * f, "Expected {0}, got {1}", i * 0.5f, result.GetOrDefault());
    //
    //     if (i % 100000 == 0) {
    //         EZ_CORE_INFO("i: {0}", i);
    //         ScriptingEngine::Lib::CallGC();
    //     }
    // }
    //
    // ScriptingEngine::KNativeFunctions::KNativeFunction<void(JInteger)> printIntFunction(
    //     [](JInteger i) {
    //         EZ_CORE_INFO("PrintInt: {0}", i.Get().value());
    //     }
    // );
    //
    // for (int i = 0; i < 10; ++i) {
    //     printIntFunction(JInteger{i});
    // }

    JString space = JString{" "};

    ScriptingEngine::KNativeFunctions::KNativeFunction<JString(JString, JString)> concat(
        [&space](JString str1, JString str2) {
            return JString{str1.Get() + space.Get() + str2.Get()};
        }
    );

    {
        std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 1000000; i++) {
            JString res = concat(JString{"Hello"}, JString{"World"});
            if (i % 10000 == 0) {
                EZ_CORE_INFO("Concat: {0}", res.Get().c_str());
                ScriptingEngine::Lib::CallGC();
            }
        }
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        std::cout << "Time taken for 1000000 concatenations: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
    }
}

int main(int argc, char *argv[]) {
    Log::Init();

    ScriptingEngine::Init();

    RunJavaTests();

    ScriptingEngine::Shutdown();

    return 0;
}

/*int main(int argc, char *argv[]) {
    auto app = Easy::ApplicationBuilder::Start()
            .Window<OpenGLWindow>()
            .ImGuiLayer<OpenGLImGuiLayer>()
            .Build();

    auto editorLayer = MakeArc<EditorLayer>();
    app->PushLayer(editorLayer);
    app->GetWindow().SetVSync(true);
    app->Run();
}*/

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
