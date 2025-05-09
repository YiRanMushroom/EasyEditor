module;

#include <Core/MacroUtils.hpp>

module EditorMain;

import Easy;
import Easy.Scripting.JniBind;

import Easy.Scripting.JTypes;

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
}

void RunJavaTests();

int main() {
    Log::Init();

    JavaVMOption options[1];
    options[0].optionString = const_cast<char *>("-Djava.class.path=./easy-core-lib-1.0.jar");

    ScriptingEngine::Init({.version = JNI_VERSION_1_6},
                          options);

    // ScriptingEngine::Lib::PrintClassInfo(ScriptingEngine::Lib::GetClass("com.easy.Lib"));

    RunJavaTests();

    ScriptingEngine::Shutdown();

    return 0;
}

template<typename>
class JConstructor {
    static_assert(false, "JConstructor is not specialized");
};

// constructor hold a methodId
template<typename ClassType, typename... Args>
class JConstructor<ClassType(Args...)> {
public:
    JConstructor() = default;

    JConstructor(jmethodID id) : id(id) {}

    jobject Invoke(JNIEnv *env, jclass cls, Args... args) {
        // EZ_CORE_ASSERT(id != nullptr, "Constructor not initialized");
        // return env->NewObject(cls, id, args.ToJava()...);
        std::array<jvalue, sizeof...(Args)> jniArgs{
            jvalue{args.ToJvalue()}...
        };

        return env->NewObjectA(
            cls, id,
            jniArgs.data());
    }

    consteval static auto GetSignature() {
        return ScriptingEngine::MethodResolver::ResolveSigExact<void(Args...)>();
    }

private:
    jmethodID id = nullptr;
};

using namespace ScriptingEngine::JTypes;

struct TestStruct {
public:
    constexpr static Class Definition{
        "com/easy/Test",
        Method{"ToString", Return{jstring{}}, Params{}}
    };

    using JavaType = jobject;

    jobject ToJava() const {
        return static_cast<jobject>(m_Object);
    }

    TestStruct() : m_Object{
        NewRef{},
        s_Constructor3.Invoke(ScriptingEngine::GetEnv(), static_cast<jclass>(static_cast<jobject>(*s_ClassPtr)))
    } {
        EZ_CORE_ASSERT(static_cast<jobject>(m_Object) != nullptr, "Failed to create TestStruct");
    }

    TestStruct(jobject obj) : m_Object{NewRef{}, obj} {}

    TestStruct(JString str, Jint i, JDouble d)
        : TestStruct(s_Constructor1.Invoke(ScriptingEngine::GetEnv(),
                                           static_cast<jclass>(static_cast<jobject>(*s_ClassPtr)), str, i, d)) {
        EZ_CORE_ASSERT(static_cast<jobject>(m_Object) != nullptr, "Failed to create TestStruct");
    }

    TestStruct(Jint i) : TestStruct(s_Constructor2.Invoke(ScriptingEngine::GetEnv(),
                                                          static_cast<jclass>(static_cast<jobject>(*s_ClassPtr)), i)) {
        EZ_CORE_ASSERT(static_cast<jobject>(m_Object) != nullptr, "Failed to create TestStruct");
    }

    static void Init() {
        s_ClassPtr = std::make_unique<GlobalObject<JTClass>>(
            PromoteToGlobal{}, static_cast<jobject>(ScriptingEngine::Lib::GetClass("com/easy/Test"))
        );

        constexpr auto sig1 = decltype(s_Constructor1)::GetSignature();
        static_assert(sig1 == "(Ljava/lang/String;ILjava/lang/Double;)V"_sl);
        jmethodID constructorId1 = ScriptingEngine::GetEnv()->GetMethodID(
            static_cast<jclass>(static_cast<jobject>(*s_ClassPtr)),
            "<init>", sig1.Data);
        EZ_ASSERT(constructorId1 != nullptr, "Constructor not found");
        s_Constructor1 = {constructorId1};

        constexpr auto sig2 = decltype(s_Constructor2)::GetSignature();
        static_assert(sig2 == "(I)V"_sl);
        jmethodID constructorId2 = ScriptingEngine::GetEnv()->GetMethodID(
            static_cast<jclass>(static_cast<jobject>(*s_ClassPtr)),
            "<init>", sig2.Data);
        EZ_ASSERT(constructorId2 != nullptr, "Constructor not found");
        s_Constructor2 = {constructorId2};

        constexpr auto sig3 = decltype(s_Constructor3)::GetSignature();
        static_assert(sig3 == "()V"_sl);
        jmethodID constructorId3 = ScriptingEngine::GetEnv()->GetMethodID(
            static_cast<jclass>(static_cast<jobject>(*s_ClassPtr)),
            "<init>", sig3.Data);
        EZ_ASSERT(constructorId3 != nullptr, "Constructor not found");
        s_Constructor3 = {constructorId3};
    }

private:
    inline static JConstructor<TestStruct(JString, Jint, JDouble)> s_Constructor1;
    inline static JConstructor<TestStruct(Jint)> s_Constructor2;
    inline static JConstructor<TestStruct()> s_Constructor3;

    inline static std::unique_ptr<GlobalObject<JTClass>> s_ClassPtr;

    LocalObject<Definition> m_Object;
};

void RunJavaTests() {
    using namespace ScriptingEngine;
    using namespace Lib;
    ScriptingEngine::Lib::PrintClassInfo(ScriptingEngine::Lib::GetClass("com.easy.Test"));

    TestStruct::Init();

    TestStruct testStruct{
        42
    };

    LocalObject<TestStruct::Definition> m_Local{testStruct.ToJava()};

    EZ_ASSERT(static_cast<jobject>(m_Local) != nullptr, "Failed to create TestStruct");

    LocalString toStringRes = m_Local.Call<"ToString">();

    EZ_ASSERT(static_cast<jobject>(static_cast<jstring>(toStringRes)) != nullptr, "Failed to call ToString");

    std::cout << "Result: " << toStringRes.Pin().ToString() << std::endl;
}
