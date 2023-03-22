#include "EditorLayer.h"

#include "Utils/EditorResourceManager.h"
#include "NativeScripts.h"

#include "Hazel/Scene/SceneSerializer.h"
#include "Hazel/Utils/PlatformUtils.h"
#include "Hazel/Core/FileSystem.h"
#include "Hazel/Scripting/ScriptEngine.h"

#include <Box2D/include/box2d/b2_body.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <ImGuizmo.h>

#include <glm/gtc/type_ptr.hpp>

namespace Hazel
{
	EditorLayer::EditorLayer() : Layer("Hazel Editor") {}

	void EditorLayer::OnAttach()
	{
		HZ_PROFILE_FUNCTION();

		Utils::EditorResourceManager::Init();

		// Set Fonts
		auto* imGuiLayer = Application::Get().GetImGuiLayer();
		const auto normalFontPath = "Resources/Fonts/opensans/OpenSans-SemiBold.ttf";
		const auto boldFontPath = "Resources/Fonts/opensans/OpenSans-ExtraBold.ttf";
		imGuiLayer->SetFonts(normalFontPath, {boldFontPath});

		const auto framebufferSpecification = FramebufferSpecification
		{
			{FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth},
			1280,
			720
		};
		_framebuffer = Framebuffer::Create(framebufferSpecification);

		const auto commandLineArgs = Application::Get().GetSpecification().CommandLineArgs;
		if (commandLineArgs.Count > 1)
		{
			const auto projectFilePath = commandLineArgs[1];
			if (std::filesystem::exists(projectFilePath))
			{
				OpenProject(projectFilePath);
			}
			else
			{
				HZ_CORE_LERROR("No project found at {0}", projectFilePath);
			}
		}

		// TODO prompt file user select directory for new project.
		if (!Project::GetActive())
		{
			FileDialogs::MessagePopup("Open Project or Create a new one.", "Need Project!");
		}

		_editorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		Renderer2D::SetLineWidth(4.0f);

		Application::Get().GetImGuiLayer()->BlockEvents(false);
	}

	void EditorLayer::OnDetach()
	{
		HZ_PROFILE_FUNCTION();
		Utils::EditorResourceManager::Shutdown();
	}

	void EditorLayer::OnUpdate(Timestep timestep)
	{
		HZ_PROFILE_FUNCTION();
		_updateTimer.Reset();

		if (!_activeScene)
		{
			return;
		}

		_activeScene->OnViewportResize(static_cast<uint32_t>(_sceneViewportSize.x), static_cast<uint32_t>(_sceneViewportSize.y));

		auto& frameBufferSpec = _framebuffer->GetSpecification();
		if (_sceneViewportSize.x > 0.0f && _sceneViewportSize.y > 0.0f &&
			(frameBufferSpec.Width != static_cast<uint32_t>(_sceneViewportSize.x) || frameBufferSpec.Height != static_cast<uint32_t>(_sceneViewportSize.y)))
		{
			_framebuffer->Resize(static_cast<uint32_t>(_sceneViewportSize.x), static_cast<uint32_t>(_sceneViewportSize.y));

			_editorCamera.SetViewportSize(_sceneViewportSize.x, _sceneViewportSize.y);
		}

		CalculateFPS();

#if !HZ_PROFILE
		//SafetyShutdownCheck();
#endif // !HZ_PROFILE


		Renderer2D::ResetStats();
		// Render
		_framebuffer->Bind();
		RenderCommand::SetClearColor(_clearColor);
		RenderCommand::Clear();

		// Clear our entity ID attachment to -1
		_framebuffer->ClearAttachment(1, -1);

		switch (_sceneState)
		{
		case SceneState::Edit:
		{
			_editorCamera.IsEnable() =
				(_isSceneViewportHovered && _isSceneViewportFocused && !ImGuizmo::IsUsing())
				|| _editorCamera.IsAdjusting();
			_editorCamera.OnUpdate(timestep);

			// Update Scene
			_activeScene->OnUpdateEditor(timestep, _editorCamera);
			break;
		}
		case SceneState::Simulate:
		{
			_editorCamera.IsEnable() =
				(_isSceneViewportHovered && _isSceneViewportFocused && !ImGuizmo::IsUsing())
				|| _editorCamera.IsAdjusting();
			_editorCamera.OnUpdate(timestep);

			// Update Scene
			_activeScene->OnUpdateSimulation(timestep, _editorCamera);
			break;
		}
		case SceneState::Play:
		{
			_activeScene->OnUpdateRuntime(timestep);
			break;
		}
		}

		MousePicking();

		OnOverlayRender();

		_framebuffer->Unbind();

		_updateTimerElapsedMillis = _updateTimer.ElapsedMillis();
		if (_updateTimerElapsedMillis > _updateTimerSlowestElapsedMillis)
		{
			_updateTimerSlowestElapsedMillis = _updateTimerElapsedMillis;
		}

		if (_updateTimerElapsedMillis < _updateTimerFastestElapsedMillis)
		{
			_updateTimerFastestElapsedMillis = _updateTimerElapsedMillis;
		}
	}

	void EditorLayer::OnImGuiRender()
	{
		HZ_PROFILE_FUNCTION();

		_imGuiTimer.Reset();

		static bool dockSpaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace", &dockSpaceOpen, window_flags | ImGuiWindowFlags_NoNavInputs);
		ImGui::PopStyleVar();

		DrawToolbar();

		if (opt_fullscreen)
		{
			ImGui::PopStyleVar(2);
		}

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		auto originalWindowMinSize = style.WindowMinSize;
		style.WindowMinSize.x = 270.0f;

		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize = originalWindowMinSize;

		DrawFileMenu();
		DrawNewProjectPopup();
		DrawSceneViewport();
		DrawTools();

		_sceneHierarchyPanel.OnImGuiRender();
		if (_contentBrowserPanel)
		{
			_contentBrowserPanel->OnImGuiRender();
		}

		DrawStats();

		ImGui::End();

		_imGuiTimerElapsedMillis = _imGuiTimer.ElapsedMillis();
		if (_imGuiTimerElapsedMillis > _imGuiTimerSlowestElapsedMillis)
		{
			_imGuiTimerSlowestElapsedMillis = _imGuiTimerElapsedMillis;
		}

		if (_imGuiTimerElapsedMillis < _imGuiTimerFastestElapsedMillis)
		{
			_imGuiTimerFastestElapsedMillis = _imGuiTimerElapsedMillis;
		}
	}

	void EditorLayer::OnEvent(Event& event)
	{
		if (_isSceneViewportHovered && _sceneState != SceneState::Play)
		{
			_editorCamera.OnEvent(event);
		}

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(HZ_BIND_EVENT_FN(OnKeyPressed));
		dispatcher.Dispatch<MouseButtonUpEvent>(HZ_BIND_EVENT_FN(OnMouseButtonUp));
	}

	bool EditorLayer::OnKeyPressed(const KeyPressedEvent& keyPressedEvent)
	{
		if (ImGui::GetIO().WantTextInput || Input::IsMouseButtonDown(MouseCode::ButtonRight))
		{
			return false;
		}

		const bool isImGuizmoInUse = ImGuizmo::IsUsing();
		const bool isControlPressed = Input::IsKeyDown(Key::LeftControl) || Input::IsKeyDown(Key::RightControl);
		const bool isShiftPressed = Input::IsKeyDown(Key::LeftShift) || Input::IsKeyDown(Key::RightShift);
		const bool isModiferPressed = isControlPressed || isShiftPressed;

		switch (keyPressedEvent.GetKeyCode())
		{
		// File Commands
		case Key::N:
		{
			if (isControlPressed && !isShiftPressed)
			{
				NewScene(true);
			}
			break;
		}
		case Key::O:
		{
			if (isControlPressed && !isShiftPressed)
			{
				OpenScene();
			}
			break;
		}
		case Key::S:
		{
			if (isControlPressed)
			{
				if (isShiftPressed)
				{
					SaveSceneAs(Project::GetAssetDirectory());
				}
				else
				{
					SaveScene();
				}
			}
			break;
		}
		// Gizmos Commands
		case Key::Q: // Nothing
		{
			if (!isModiferPressed && !isImGuizmoInUse)
			{
				_gizmoType = -1;
			}
			break;
		case Key::W: // Position
			if (!isModiferPressed && !isImGuizmoInUse)
			{
				_gizmoType = ImGuizmo::TRANSLATE;
			}
			break;
		}
		case Key::E: // Rotation
			if (!isModiferPressed && !isImGuizmoInUse)
			{
				_gizmoType = ImGuizmo::ROTATE;
			}
			break;
		case Key::R: // Scale
		{
			if (!isModiferPressed && !isImGuizmoInUse)
			{
				_gizmoType = ImGuizmo::SCALE;
				_gizmoSpace = ImGuizmo::LOCAL;
			}
			break;
		}
		case Key::X: // space toggle, Local or Global
		{
			if (!isModiferPressed && !isImGuizmoInUse && _gizmoType != ImGuizmo::SCALE)
			{
				_gizmoSpace = _gizmoSpace == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
			}
			break;
		}
		// Scene Commands
		case Key::D:
		{
			if (isControlPressed)
			{
				DuplicateEntity();
			}
			break;
		}
		// TODO make this work
		//case Key::Escape:
		//{
		//	auto payload = ImGui::GetDragDropPayload();
		//	if (payload)
		//	{
		//		// Cancel payload.
		//	}
		//	break;
		//}
		}

		return true;
	}

	bool EditorLayer::OnMouseButtonUp(const MouseButtonUpEvent& mouseButtonReleasedEvent)
	{
		// TODO Mouse picking, allow simulation, find bug where simulation stops?
		if (_isSceneViewportHovered && mouseButtonReleasedEvent.GetMouseButton() == Mouse::ButtonLeft && _sceneState != SceneState::Simulate)
		{
			if ((!_sceneHierarchyPanel.GetSelectedEntity() || !ImGuizmo::IsOver()) && !Input::IsKeyDown(Key::LeftAlt))
			{
				_sceneHierarchyPanel.SetSelectedEntity(_hoveredEntity);
			}
		}

		return false;
	}

	void EditorLayer::MousePicking()
	{
		if (!_isSceneViewportHovered)
		{
			_hoveredEntity = Entity();
			return;
		}

		auto [imMouseX, imMouseY] = ImGui::GetMousePos();
		imMouseX -= _sceneViewportBounds[0].x;
		imMouseY -= _sceneViewportBounds[0].y;

		const glm::vec2 sceneViewportSize = _sceneViewportBounds[1] - _sceneViewportBounds[0];
		imMouseY = sceneViewportSize.y - imMouseY;
		const auto mouseX = static_cast<int>(imMouseX);
		const auto mouseY = static_cast<int>(imMouseY);

		if (mouseX >= 0 && mouseY >= 0 && mouseX < static_cast<int>(sceneViewportSize.x) && mouseY < static_cast<int>(sceneViewportSize.y))
		{
			int pixelData = _framebuffer->ReadPixel(1, mouseX, mouseY);

			_hoveredEntity = pixelData == -1 ? Entity() : _activeScene->CheckEntityValidity(static_cast<entt::entity>(pixelData)) ? Entity(static_cast<entt::entity>(pixelData), _activeScene.get()) : Entity();
		}
	}

	void EditorLayer::OnOverlayRender() const
	{
		float cameraPositionZ = 1.0f;
		bool hasSceneBegun;

		if (_sceneState == SceneState::Play)
		{
			const auto primaryCamera = _activeScene->GetPrimaryCameraEntity();
			if (!primaryCamera)
			{
				return;
			}

			const auto& camera = primaryCamera.GetComponent<CameraComponent>().Camera;
			const auto& transformComponent = primaryCamera.Transform();
			const auto transform = transformComponent.GetWorldTransformMatrix();
			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				cameraPositionZ = transformComponent.Position.z;
			}

			hasSceneBegun = Renderer2D::BeginScene(camera, transform);
		}
		else
		{
			cameraPositionZ = _editorCamera.GetPosition().z;
			hasSceneBegun = Renderer2D::BeginScene(_editorCamera.GetViewProjection());
		}

		if (!hasSceneBegun)
		{
			return;
		}

		const auto selectedEntity = _sceneHierarchyPanel.GetSelectedEntity();

		if (_shouldShowPhysicsColliders ||
			(selectedEntity &&
				(selectedEntity.HasComponent<BoxCollider2DComponent>() ||
					selectedEntity.HasComponent<CircleCollider2DComponent>())))
		{
			constexpr auto kIdentityMatrix = glm::mat4(1.0f);

			// Box Colliders
			{
				for (const auto&& [enttID, component, transform] : _activeScene->GetEntitiesViewWith<BoxCollider2DComponent, TransformComponent>().each())
				{
					if (_shouldShowPhysicsColliders || enttID == selectedEntity)
					{
						if (_sceneState != SceneState::Edit)
						{
							Entity entity = {enttID, _activeScene.get()};
							if (entity.HasComponent<Rigidbody2DComponent>())
							{
								const auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
								transform.Position.x = static_cast<b2Body*>(rb2d.RuntimeBody)->GetPosition().x;
								transform.Position.y = static_cast<b2Body*>(rb2d.RuntimeBody)->GetPosition().y;
								transform.Rotation.z = static_cast<b2Body*>(rb2d.RuntimeBody)->GetAngle();
							}
						}
						const float sign = cameraPositionZ > transform.Position.z ? 1.0f : -1.0f;

						glm::mat4 newTransform = glm::translate(kIdentityMatrix, transform.Position)
							* glm::toMat4(glm::quat(transform.Rotation))
							* glm::translate(kIdentityMatrix, glm::vec3(component.Offset, 0.001f * sign))
							* glm::toMat4(glm::quat(glm::vec3(0.0f, 0.0f, glm::radians(component.Rotation))))
							* glm::scale(kIdentityMatrix, transform.Scale * glm::vec3(component.Size * 2.0f, 1.0f));

						Renderer2D::DrawRect(newTransform, Color::Green);

						if (!_shouldShowPhysicsColliders)
						{
							break;
						}
					}
				}
			}

			// Circle Colliders
			{
				for (const auto&& [enttID, component, transform] : _activeScene->GetEntitiesViewWith<CircleCollider2DComponent, TransformComponent>().each())
				{
					if (_shouldShowPhysicsColliders || enttID == selectedEntity)
					{
						if (_sceneState != SceneState::Edit)
						{
							Entity entity = {enttID, _activeScene.get()};
							if (entity.HasComponent<Rigidbody2DComponent>())
							{
								const auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
								transform.Position.x = static_cast<b2Body*>(rb2d.RuntimeBody)->GetPosition().x;
								transform.Position.y = static_cast<b2Body*>(rb2d.RuntimeBody)->GetPosition().y;
								transform.Rotation.z = static_cast<b2Body*>(rb2d.RuntimeBody)->GetAngle();
							}
						}
						const float sign = cameraPositionZ > transform.Position.z ? 1.0f : -1.0f;

						glm::mat4 newTransform = glm::translate(kIdentityMatrix, transform.Position)
							* glm::toMat4(glm::quat(transform.Rotation))
							* glm::translate(kIdentityMatrix, glm::vec3(component.Offset, 0.001f * sign))
							* glm::scale(kIdentityMatrix, glm::vec3(glm::max(transform.Scale.x, transform.Scale.y)) * glm::vec3(component.Radius * 2.0f));

						Renderer2D::DrawCircle(newTransform, Color::Green, 0.01f);

						if (!_shouldShowPhysicsColliders)
						{
							break;
						}
					}
				}
			}
		}

		if (selectedEntity)
		{
			const auto& transform = selectedEntity.Transform();

			Renderer2D::DrawRect(transform.GetWorldTransformMatrix(), Color::Orange);
		}

		Renderer2D::EndScene();
	}

	bool EditorLayer::ClearSceneCheck() const
	{
		const char* message = "You will lose current scene's unsaved progress.\nDo you want to continue?";
		const char* title = "Warning!";
		return FileDialogs::QuestionPopup(message, title);
	}

	void EditorLayer::NewScene(bool withCheck)
	{
		if (_sceneState != SceneState::Edit)
		{
			HZ_CORE_LWARN("Stop Current scene before creating a new one.");
			return;
		}

		if (withCheck && !ClearSceneCheck())
		{
			return;
		}

		_editorScene = CreateRef<Scene>(kNewSceneName);

		SetWindowTitleSceneName();

		_activeScene = _editorScene;
		_sceneHierarchyPanel.SetScene(_activeScene);

		_editorScenePath.clear();
	}

	void EditorLayer::OpenScene()
	{
		const auto filePath = FileDialogs::OpenFile("Hazel Scene (*.hazel)\0*.hazel\0");
		if (!filePath.empty())
		{
			OpenScene(filePath, true);
		}
	}

	void EditorLayer::OpenScene(const std::filesystem::path& path, bool withCheck)
	{
		if (path.extension().string() != ".hazel")
		{
			HZ_CORE_LERROR("Could not load {0} - not a scene file.", path.filename().string());
			return;
		}

		if (_sceneState != SceneState::Edit)
		{
			HZ_CORE_LWARN("Stop Current scene before opening a new one.");
			return;
		}

		if (withCheck && !ClearSceneCheck())
		{
			return;
		}

		_editorScene = CreateRef<Scene>();
		_editorScenePath.clear();
		SceneSerializer serializer(_editorScene);
		if (serializer.Deserialize(path.string(), withCheck))
		{
			_editorScenePath = path;

			SetWindowTitleSceneName(_editorScenePath);

			_activeScene = _editorScene;
			_sceneHierarchyPanel.SetScene(_activeScene);
		}
	}

	void EditorLayer::SaveScene()
	{
		if (_sceneState != SceneState::Edit)
		{
			FileDialogs::MessagePopup("You can't save while in play mode!", "Error");
			return;
		}

		if (!_editorScenePath.empty())
		{
			SerializeScene();
		}
		else
		{
			SaveSceneAs(Project::GetAssetDirectory());
		}
	}

	bool EditorLayer::SaveSceneAs(const std::filesystem::path& defaultPath)
	{
		if (_sceneState != SceneState::Edit)
		{
			FileDialogs::MessagePopup("You can't save while in play mode!", "Error");
			return false;
		}

		const auto sceneName = _activeScene->GetName();
		const auto filePath = FileDialogs::SaveFile("Hazel Scene (*.hazel)\0*.hazel\0", !sceneName.empty() ? sceneName.c_str() : nullptr, defaultPath);

		if (!filePath.empty())
		{
			_editorScenePath = filePath;
			SerializeScene();
			SetWindowTitleSceneName(_editorScenePath);

			return true;
		}

		return false;
	}

	void EditorLayer::SerializeScene() const
	{
		SceneSerializer serializer(_activeScene);
		serializer.Serialize(_editorScenePath.string());
	}

	void EditorLayer::NewProject()
	{
		if (!ClearSceneCheck())
		{
			return;
		}

		if (_newProjectPath.empty() || _newProjectName.empty())
		{
			return;
		}

		_newProjectPath /= _newProjectName;
		if (!std::filesystem::exists(_newProjectPath) && !std::filesystem::create_directory(_newProjectPath))
		{
			HZ_CORE_LERROR("Failed to create folder at path {0}", _newProjectPath);
			return;
		}

		const std::filesystem::path assetsPath = _newProjectPath / "Assets";
		const std::filesystem::path scriptPath = assetsPath / "Scripts";
		std::filesystem::create_directory(assetsPath);
		std::filesystem::create_directory(scriptPath);

		// TODO cleanup this logic.
		// This will generate a premake file and create a visual studio 2022 project
		const std::filesystem::path outputFilePathTemp(scriptPath / "premake5Temp.lua");
		FileSystem::ReplaceInFile("Resources/NewProjectTemplate/NewProjectTemplate.lua", outputFilePathTemp, "newprojectname", _newProjectName);
		const std::filesystem::path outputFilePathFinal(scriptPath / "premake5.lua");
		FileSystem::ReplaceInFile(outputFilePathTemp, outputFilePathFinal, "../../../..", FileSystem::GetApplicationPath().parent_path().string());
		std::filesystem::remove(outputFilePathTemp);
		const std::filesystem::path premakePath(FileSystem::GetApplicationPath().parent_path() / "vendor/premake/bin/premake5.exe");
		const std::string executionPath = fmt::format("{0} --file=\"{1}\" vs2022", premakePath.string().c_str(), outputFilePathFinal.string().c_str());
		system(executionPath.c_str());

		Project::New(_newProjectPath);
		auto& config = Project::GetActive()->GetConfig();
		config.Name = _newProjectName;
		config.AssetDirectory = std::filesystem::relative(assetsPath, _newProjectPath);
		config.ScriptModulePath = std::filesystem::relative(scriptPath, assetsPath) / fmt::format("Binaries/{0}.dll", _newProjectName);

		NewScene();

		_contentBrowserPanel = CreateScope<ContentBrowserPanel>();
		ScriptEngine::TryReload();
		Project::SaveActive();
	}

	void EditorLayer::OpenProject()
	{
		const auto filePath = FileDialogs::OpenFile("Hazel Project (*.hproj)\0*.hproj\0");
		if (!filePath.empty())
		{
			OpenProject(filePath);
		}
	}

	void EditorLayer::OpenProject(const std::filesystem::path& path)
	{
		if (const auto project = Project::Load(path))
		{
			_editorScenePath.clear();
			const auto startScenePath = project->GetAssetFileSystemPath(project->GetConfig().StartScene);
			OpenScene(startScenePath);
			if (_editorScenePath.empty())
			{
				HZ_CORE_LWARN("New Scene created!");
				NewScene(false);
			}

			_contentBrowserPanel = CreateScope<ContentBrowserPanel>();
			ScriptEngine::TryReload(true);
		}
	}

	void EditorLayer::SaveProject()
	{
		const auto projectConfig = Project::GetActive()->GetConfig();
		if (projectConfig.StartScene.empty())
		{
			SetProjectStartSceneToCurrent();
		}

		Project::SaveActive();
	}

	void EditorLayer::SetProjectStartSceneToCurrent()
	{
		const std::string message = fmt::format("Do you want to make {0} the starting scene for project {1}", _activeScene->GetName(), Project::GetActive()->GetConfig().Name);
		if (FileDialogs::QuestionPopup(message.c_str(), "Set Startup Scene"))
		{
			if (_editorScenePath.empty())
			{
				SaveScene();
			}

			Project::GetActive()->GetConfig().StartScene = std::filesystem::relative(_editorScenePath, Project::GetAssetDirectory());
			SaveProject();
		}
	}

	void EditorLayer::SetWindowTitleSceneName(const std::filesystem::path& scenePath) const
	{
		auto& window = Application::Get().GetWindow();
		auto fileName = scenePath.empty() ? "Unsaved" : scenePath.stem();
		window.SetTitle(fmt::format("{0} [{1}] [{2}]", window.GetTitle(), Project::GetActive()->GetConfig().Name, fileName));
	}

	void EditorLayer::DrawToolbar()
	{
		if (!_activeScene)
		{
			return;
		}

		const auto size = ImVec2(25.0f, 25.0f);
		const auto uv0 = ImVec2(0.0f, 1.0f);
		const auto uv1 = ImVec2(1.0f, 0.0f);

		const auto selectedColor = ImVec4(0.196f, 0.196f, 0.5f, 1.0f);
		const auto tintColor = ImVec4(0.396f, 0.396f, 0.8f, 1.0f);
		const auto normalColor = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		const auto whiteColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

		auto tableFlags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;
		if (ImGui::BeginTable("Toolbar", 4, tableFlags))
		{
			ImGui::TableSetupColumn("Blank");
			ImGui::TableSetupColumn("Gizmo");
			ImGui::TableSetupColumn("Space");
			ImGui::TableSetupColumn("SceneState", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextColumn();
			ImGui::TableNextColumn();

			Ref<Texture2D> gizmoButtonTexture;

			if (_editorCamera.IsAdjusting())
			{
				if (_editorCamera.IsPanning())
				{
					gizmoButtonTexture = Utils::ERM::GetTexture(Utils::Icon_Pan);
				}
				else if (_editorCamera.IsZooming())
				{
					gizmoButtonTexture = Utils::ERM::GetTexture(Utils::Icon_Magnifier);
				}
				else
				{
					gizmoButtonTexture = Utils::ERM::GetTexture(Utils::Icon_Eye);
				}

				if (!_hasStoredPreviousGizmoType)
				{
					_previousGizmoType = _gizmoType;
					_hasStoredPreviousGizmoType = true;
				}
				_gizmoType = -1;
			}
			else
			{
				gizmoButtonTexture = Utils::ERM::GetTexture(Utils::Icon_Nothing);
				if (_hasStoredPreviousGizmoType)
				{
					_gizmoType = _previousGizmoType;
					_hasStoredPreviousGizmoType = false;
					_previousGizmoType = -1;
				}
			}

			bool isNothing = _gizmoType == -1;
			if (ImGui::ImageButton(gizmoButtonTexture->GetRawID(), size, uv0, uv1, 3, isNothing ? selectedColor : normalColor, isNothing ? tintColor : whiteColor))
			{
				_gizmoType = -1;
			}
			AddTooltip("Nothing");

			ImGui::SameLine();

			bool isTranslate = _gizmoType == ImGuizmo::TRANSLATE;
			if (ImGui::ImageButton(Utils::ERM::GetTexture(Utils::Icon_Position)->GetRawID(), size, uv0, uv1, 3, isTranslate ? selectedColor : normalColor, isTranslate ? tintColor : whiteColor))
			{
				_gizmoType = ImGuizmo::TRANSLATE;
			}
			AddTooltip("Translate");

			ImGui::SameLine();
			bool isRotate = _gizmoType == ImGuizmo::ROTATE;
			if (ImGui::ImageButton(Utils::ERM::GetTexture(Utils::Icon_Rotation)->GetRawID(), size, uv0, uv1, 3, isRotate ? selectedColor : normalColor, isRotate ? tintColor : whiteColor))
			{
				_gizmoType = ImGuizmo::ROTATE;
			}
			AddTooltip("Rotate");

			ImGui::SameLine();
			bool isScale = _gizmoType == ImGuizmo::SCALE;
			if (ImGui::ImageButton(Utils::ERM::GetTexture(Utils::Icon_Scale)->GetRawID(), size, uv0, uv1, 3, isScale ? selectedColor : normalColor, isScale ? tintColor : whiteColor))
			{
				_gizmoType = ImGuizmo::SCALE;
				_gizmoSpace = ImGuizmo::LOCAL;
			}
			AddTooltip("Scale\nLocal Space Only");

			ImGui::TableNextColumn();
			if (_gizmoSpace == ImGuizmo::LOCAL)
			{
				if (ImGui::ImageButton(Utils::ERM::GetTexture(Utils::Icon_Local)->GetRawID(), size, uv0, uv1, 3, selectedColor, tintColor))
				{
					if (!isScale)
					{
						_gizmoSpace = ImGuizmo::WORLD;
					}
				}

				AddTooltip("Change to Global");

				ImGui::SameLine();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Local");
			}
			else
			{
				if (ImGui::ImageButton(Utils::ERM::GetTexture(Utils::Icon_Global)->GetRawID(), size, uv0, uv1, 3, selectedColor, tintColor))
				{
					_gizmoSpace = ImGuizmo::LOCAL;
				}

				AddTooltip("Change to Local");

				ImGui::SameLine();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Global");
			}

			ImGui::TableNextColumn();

			float centeredCursorPositionX = (ImGui::GetContentRegionMax().x * 0.5f) - (size.x * 0.5f);
			if (centeredCursorPositionX > ImGui::GetCursorPosX())
			{
				ImGui::SetCursorPosX(centeredCursorPositionX);
			}

#pragma region Play.Stop.Step Button
			const bool isPlayMode = _sceneState == SceneState::Play;
			const bool isSimulateMode = _sceneState == SceneState::Simulate;
			const bool isPaused = _activeScene->IsPaused();
			Ref<Texture2D> sceneStateButton;

			if (isPlayMode)
			{
				if (isPaused)
				{
					sceneStateButton = Utils::ERM::GetTexture(Utils::Icon_Step);
				}
				else
				{
					sceneStateButton = Utils::ERM::GetTexture(Utils::Icon_Stop);
				}
			}
			else
			{
				if (isPaused)
				{
					sceneStateButton = Utils::ERM::GetTexture(Utils::Icon_Step);
				}
				else
				{
					if (isSimulateMode)
					{
						sceneStateButton = Utils::ERM::GetTexture(Utils::Icon_Stop);
					}
					else
					{
						sceneStateButton = Utils::ERM::GetTexture(Utils::Icon_Play);
					}
				}
			}

			if (ImGui::ImageButton(sceneStateButton->GetRawID(), size, uv0, uv1, 3, isPlayMode ? selectedColor : normalColor, isPlayMode && !isPaused ? tintColor : whiteColor))
			{
				switch (_sceneState)
				{
				case SceneState::Play:
				case SceneState::Simulate:
				{
					if (isPaused)
					{
						_activeScene->Step();
					}
					else
					{
						OnSceneStop();
					}
					break;
				}
				case SceneState::Edit:
					OnScenePlay();
					break;
				}
			}

			switch (_sceneState)
			{
			case SceneState::Play:
			case SceneState::Simulate:
			{
				if (isPaused)
				{
					AddTooltip("Step");
				}
				else
				{
					AddTooltip("Stop");
				}
				break;
			}
			case SceneState::Edit:
				AddTooltip("Play");
				break;
			}
#pragma endregion


#pragma region Pause Button
			ImGui::SameLine();
			if (ImGui::ImageButton(Utils::ERM::GetTexture(Utils::Icon_Pause)->GetRawID(), size, uv0, uv1, 3, isPaused ? selectedColor : normalColor, isPaused ? tintColor : whiteColor))
			{
				_activeScene->SetPause(!isPaused);
			}

			if (!isPaused)
			{
				AddTooltip("Pause");
			}
			else
			{
				AddTooltip("Unpause");
			}
#pragma endregion

#pragma region Simulate Button
			if (!isPlayMode)
			{
				ImGui::SameLine();
				if (ImGui::ImageButton(Utils::ERM::GetTexture(Utils::Icon_Simulate)->GetRawID(), size, uv0, uv1, 3, isPlayMode ? normalColor : selectedColor, !isSimulateMode ? whiteColor : tintColor))
				{
					switch (_sceneState)
					{
					case SceneState::Edit:
						OnSceneSimulate();
						break;
					case SceneState::Simulate:
						OnSceneStop();
						break;
					}
				}

				switch (_sceneState)
				{
				case SceneState::Simulate:
					AddTooltip("Stop");
					break;
				case SceneState::Edit:
					AddTooltip("Simulate");
					break;
				}
			}
#pragma endregion

			ImGui::EndTable();
		}
	}

	void EditorLayer::DrawFileMenu()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				{
					const bool shouldDisable = _sceneState != SceneState::Edit || !Project::GetActive();
					if (shouldDisable)
					{
						ImGui::PushDisabled();
					}

					if (ImGui::MenuItem("New Scene", "Ctrl+N"))
					{
						NewScene(true);
					}

					if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
					{
						OpenScene();
					}

					if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
					{
						SaveScene();
					}

					if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
					{
						SaveSceneAs(Project::GetAssetDirectory());
					}

					if (shouldDisable)
					{
						ImGui::PopDisabled();
					}
				}

				{
					ImGui::Separator();
					bool shouldDisable = _sceneState != SceneState::Edit;
					if (shouldDisable)
					{
						ImGui::PushDisabled();
					}

					if (ImGui::MenuItem("New Project"))
					{
						_shouldOpenNewProjectModal = true;
					}

					if (ImGui::MenuItem("Open Project..."))
					{
						OpenProject();
					}

					if (shouldDisable)
					{
						ImGui::PopDisabled();
					}

					shouldDisable |= !Project::GetActive();
					if (shouldDisable)
					{
						ImGui::PushDisabled();
					}

					if (ImGui::MenuItem("Save Project"))
					{
						SaveProject();
					}

					if (ImGui::MenuItem("Set Project Start Scene to current"))
					{
						SetProjectStartSceneToCurrent();
					}

					if (shouldDisable)
					{
						ImGui::PopDisabled();
					}
				}

				ImGui::Separator();
				if (ImGui::MenuItem("Exit"))
				{
					HZ_LCRITICAL("Exiting application.");
					Application::Get().Stop();
				}
				ImGui::EndMenu();
			}
			ImGui::Text("\tFPS : %i", _currentFPS);

			ImGui::Spacing();

			if (ImGui::BeginMenu("Extra"))
			{
				if (ImGui::BeginMenu("Shader"))
				{
					for (int i = 0; i < std::size(sRendererShaderName); i++)
					{
						const auto rendererShaderType = static_cast<RendererShader>(i);
						if (ImGui::MenuItem(fmt::format("Reload {0}", sRendererShaderName[rendererShaderType]).c_str()))
						{
							Renderer2D::ReloadShader(rendererShaderType);
						}
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Script Engine"))
				{
					const bool shouldDisable = _sceneState != SceneState::Edit || !Project::GetActive();
					if (shouldDisable)
					{
						ImGui::PushDisabled();
					}

					if (ImGui::MenuItem("Reload"))
					{
						Application::Get().ReloadScriptEngine();
					}

					if (shouldDisable)
					{
						ImGui::PopDisabled();
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
	}

	void EditorLayer::DrawNewProjectPopup()
	{
		if (_shouldOpenNewProjectModal)
		{
			ImGui::OpenPopup("NewProject");
			_shouldOpenNewProjectModal = false;
			_newProjectName.clear();
			_newProjectPath.clear();
		}

		const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("NewProject", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Details");

			char buffer[256] = {};
			strcpy_s(buffer, sizeof(buffer), _newProjectName.c_str());
			ImGui::Text("Project Name :");
			ImGui::SameLine();
			if (ImGui::InputText("##ProjectName", buffer, sizeof(buffer), ImGuiInputTextFlags_CharsNoBlank))
			{
				_newProjectName = buffer;
			}

			ImGui::Text("Project Path :");
			ImGui::SameLine();
			const std::filesystem::path actualProjectPath(_newProjectPath / _newProjectName);
			ImGui::Text(actualProjectPath.string().c_str());

			ImGui::SameLine();
			if (ImGui::Button("Browse..."))
			{
				_newProjectPath = FileDialogs::SelectFolder();
			}

			const bool isDisabled = _newProjectPath.empty() || _newProjectName.empty();
			if (isDisabled)
			{
				ImGui::PushDisabled();
			}

			if (ImGui::Button("Accept"))
			{
				NewProject();
				ImGui::CloseCurrentPopup();
			}

			if (isDisabled)
			{
				ImGui::PopDisabled();
			}

			ImGui::SetItemDefaultFocus();

			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	void EditorLayer::DrawSceneViewport()
	{
		HZ_PROFILE_FUNCTION();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.0f, 0.0f});

		ImGui::Begin("SceneViewport");

		auto sceneViewportOffset = ImGui::GetCursorPos(); // Include tab bar.

		_isSceneViewportFocused = ImGui::IsWindowFocused();
		_isSceneViewportHovered = ImGui::IsWindowHovered();
		if (_isSceneViewportHovered && (ImGui::GetIO().MouseClicked[1] || ImGui::GetIO().MouseClicked[2]))
		{
			ImGui::SetWindowFocus();
		}

		//Application::Get().GetImGuiLayer()->BlockEvents(!_isSceneViewportHovered);

		auto sceneViewportPanelSize = ImGui::GetContentRegionAvail();
		_sceneViewportSize = {sceneViewportPanelSize.x, sceneViewportPanelSize.y};

		if (Renderer2D::IsReady())
		{
			auto textureID = reinterpret_cast<void*>(static_cast<intptr_t>(_framebuffer->GetColorAttachmentRenderID()));
			ImGui::Image(textureID, {_sceneViewportSize.x, _sceneViewportSize.y}, ImVec2{0.0f, 1.0f}, ImVec2{1.0f, 0.0f});
		}
		else
		{
			ImGui::Image(Utils::ERM::GetTexture(Utils::Image_ShaderLoading)->GetRawID(), {_sceneViewportSize.x, _sceneViewportSize.y}, ImVec2{0.0f, 1.0f}, ImVec2{1.0f, 0.0f});
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const std::filesystem::path filePath = static_cast<const wchar_t*>(payload->Data);

				if (filePath.extension() == ".hazel")
				{
					OpenScene(filePath, true);
				}

				if (filePath.extension() == ".png")
				{
					auto newEntity = _activeScene->CreateEntity(filePath.filename().string());
					newEntity.AddComponent<SpriteRendererComponent>().Texture = Texture2D::Create(filePath.string());
					_sceneHierarchyPanel.SetSelectedEntity(newEntity);
				}
			}
			ImGui::EndDragDropTarget();
		}

		auto windowSize = ImGui::GetWindowSize();
		auto minBound = ImGui::GetWindowPos();
		auto maxBound = ImVec2{minBound.x + windowSize.x, minBound.y + windowSize.y};
		minBound.x += sceneViewportOffset.x;
		minBound.y += sceneViewportOffset.y;

		_sceneViewportBounds[0] = {minBound.x, minBound.y};
		_sceneViewportBounds[1] = {maxBound.x, maxBound.y};

#pragma region Gizmo
		if (auto selectedEntity = _sceneHierarchyPanel.GetSelectedEntity())
		{
			if (_gizmoType != -1 || (_hasStoredPreviousGizmoType && _previousGizmoType != -1))
			{
				glm::mat4 cameraProjection;
				glm::mat4 cameraView;
				if (_sceneState == SceneState::Play)
				{
					if (auto cameraEntity = _activeScene->GetPrimaryCameraEntity())
					{
						// Runtime Camera;
						const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
						cameraProjection = camera.GetProjection();
						cameraView = glm::inverse(cameraEntity.Transform().GetWorldTransformMatrix());
						ImGuizmo::SetOrthographic(camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic);
					}
				}
				else
				{
					// Editor Camera
					cameraProjection = _editorCamera.GetProjection();
					cameraView = _editorCamera.GetViewMatrix();
					ImGuizmo::SetOrthographic(false);
				}

				ImGuizmo::SetDrawlist();

				auto windowWidth = ImGui::GetWindowWidth();
				auto windowHeight = ImGui::GetWindowHeight();
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);


				// Entity transform
				auto& transformComponent = selectedEntity.Transform();
				auto worldTransform = transformComponent.GetWorldTransformMatrix();

				// Snapping
				const bool snap = Input::IsKeyDown(Key::LeftControl);
				float snapValue = 0.5f; // Snap to 0.5m for position and scale.

				// Snap to 45 degrees for rotation.
				if (_gizmoType == ImGuizmo::ROTATE)
				{
					snapValue = 45.0f;
				}

				const float snapValues[3] = {snapValue, snapValue, snapValue};
				int gizmoType = _hasStoredPreviousGizmoType ? _previousGizmoType : _gizmoType;
				ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
					static_cast<ImGuizmo::OPERATION>(gizmoType), static_cast<ImGuizmo::MODE>(_gizmoSpace),
					glm::value_ptr(worldTransform), nullptr, snap ? snapValues : nullptr);

				if (ImGuizmo::IsUsing() && !_hasStoredPreviousGizmoType)
				{
					transformComponent.SetWorldTransform(worldTransform);
					// glm::vec3 position, rotation, scale;
					// if (HMath::DecomposeTransform(transform, position, rotation, scale))
					// {
					// 	transformComponent.Position = position;
					// 	auto rotationDelta = rotation - transformComponent.Rotation;
					// 	transformComponent.Rotation += rotationDelta;
					// 	transformComponent.Scale = scale;
					//
					// 	_sceneHierarchyPanel.EditRuntimeRigidbody(selectedEntity, true);
					// }
				}
			}
		}
#pragma endregion

#pragma region CameraDriving
		if (_editorCamera.IsAdjusting() && ImGui::IsItemHovered())
		{
			// Cursor
			ImGui::SetMouseCursor(ImGuiMouseCursor_None);
			auto mousePos = ImGui::GetMousePos();

			auto upperLeftPos = mousePos;
			upperLeftPos.y -= 10.0f;
			upperLeftPos.x -= 10.0f;
			auto lowerRightPos = upperLeftPos;
			lowerRightPos.y += 25.0f;
			lowerRightPos.x += 25.0f;

			Ref<Texture2D> cursorTexture;
			if (_editorCamera.IsPanning())
			{
				cursorTexture = Utils::ERM::GetTexture(Utils::Icon_Pan);
			}
			else if (_editorCamera.IsZooming())
			{
				cursorTexture = Utils::ERM::GetTexture(Utils::Icon_Magnifier);
			}
			else
			{
				cursorTexture = Utils::ERM::GetTexture(Utils::Icon_Eye);
			}


			if (_editorCamera.IsDriving())
			{
				// Move Speed
				std::stringstream ss;
				ss << _editorCamera.GetDrivingSpeed();
				if (Input::IsKeyDown(Key::LeftShift))
				{
					ss << "*";
				}
				mousePos.y += 10.0f;
				mousePos.x += 10.0f;
				ImGui::GetForegroundDrawList()->AddText(mousePos, IM_COL32_WHITE, ss.str().c_str());
			}

			ImGui::GetForegroundDrawList()->AddImage(cursorTexture->GetRawID(), upperLeftPos, lowerRightPos, {0.0f, 1.0f}, {1.0f, 0.0f}, IM_COL32_WHITE);
		}

#pragma endregion

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void EditorLayer::DrawStats()
	{
		auto stats = Renderer2D::GetStats();

		ImGui::Begin("Stats");

		std::string name = "None";
		if (_hoveredEntity)
		{
			name = _hoveredEntity.Name();
		}
		ImGui::Text("Hovered Entity: %s", name.c_str());

		ImGui::Separator();
		ImGui::Text("Renderer 2D Stats:");
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quad Count: %d", stats.QuadCount);
		ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Indices: %d", stats.GetTotalIndexCount());

		ImGui::Separator();
		ImGui::Text("Viewport Update");
		ImGui::Text("Ms per frame: %.3f", _updateTimerElapsedMillis);
		ImGui::Text("Slowest frame: %.3f", _updateTimerSlowestElapsedMillis);
		ImGui::Text("Fasted frame: %.3f", _updateTimerFastestElapsedMillis);
		if (ImGui::Button("Reset##UPDATE"))
		{
			_updateTimerFastestElapsedMillis = FLT_MAX;
			_updateTimerSlowestElapsedMillis = -FLT_MAX;
		}

		ImGui::Separator();
		ImGui::Text("ImGui Render");
		ImGui::Text("Ms per frame: %.3f", _imGuiTimerElapsedMillis);
		ImGui::Text("Slowest frame: %.3f", _imGuiTimerSlowestElapsedMillis);
		ImGui::Text("Fasted frame: %.3f", _imGuiTimerFastestElapsedMillis);
		if (ImGui::Button("Reset##IMGUI"))
		{
			_imGuiTimerFastestElapsedMillis = FLT_MAX;
			_imGuiTimerSlowestElapsedMillis = -FLT_MAX;
		}

		ImGui::End();
	}

	void EditorLayer::DrawTools()
	{
		ImGui::Begin("Tools");

		ImGui::Checkbox("Show physics colliders", &_shouldShowPhysicsColliders);
		if (_activeScene)
		{
			ImGui::Checkbox("Clone AudioSource on Play", &_activeScene->ShouldCloneAudioSource());
		}

		if (ImGui::Button("Show Demo Window"))
		{
			_isDemoWidowOpen = !_isDemoWidowOpen;
		}

		if (_isDemoWidowOpen)
		{
			ImGui::ShowDemoWindow(&_isDemoWidowOpen);
		}
		ImGui::Separator();

		// TODO Since we can remove and add component need to disable this for now, to revamp later.
		// Add small secondary viewport for selected camera.
		/*auto& mainCamera = _mainCamera.GetComponent<CameraComponent>();
		auto& secondaryCamera = _secondaryCamera.GetComponent<CameraComponent>();

		if (ImGui::Checkbox("Secondary Camera", &_isOnSecondCamera))
		{
			mainCamera.IsPrimary = !_isOnSecondCamera;
			secondaryCamera.IsPrimary = _isOnSecondCamera;
		}*/
		ImGui::Separator();

		if (_activeScene && ImGui::Button("Create 50 squares"))
		{
			for (int i = 50 - 1; i >= 0; i--)
			{
				auto color = Color::Random();
				auto newEntity = _activeScene->CreateEntity("Square " + std::to_string(i) + ":" + color.GetHexValue());
				auto& spriteRendererComponent = newEntity.AddComponent<SpriteRendererComponent>();
				spriteRendererComponent.Color = color;
				newEntity.Transform().Position = Random::Vec3();
			}
		}

		ImGui::End();
	}

	/// <summary>
	/// If FPS goes below 2, shutdown.
	/// </summary>
	void EditorLayer::SafetyShutdownCheck()
	{
		// Safety shutdown 
		if (_currentFPS < 2)
		{
			if (_lowFrames == 0)
			{
				HZ_LERROR("Application will shutdown after 10 frames below 2 FPS.", _lowFrames);
			}
			_lowFrames++;
			HZ_LERROR("Frame #{0}", _lowFrames);
			if (_lowFrames >= 10)
			{
				HZ_LCRITICAL("Shuting down.");
				Application::Get().Stop();
			}
		}
		else if (_lowFrames != 0)
		{
			HZ_LINFO("Back to 2 FPS or above.");
			_lowFrames = 0;
		}
	}

	void EditorLayer::CalculateFPS()
	{
		HZ_PROFILE_FUNCTION();
		_oneSecondCountDown -= Time::GetTimestep();
		_frameCount++;
		if (_oneSecondCountDown <= 0.0f)
		{
			_currentFPS = _frameCount;
			_oneSecondCountDown = 1.0f;
			_frameCount = 0;
		}
	}

	void EditorLayer::AddTooltip(const std::string& tooltipMessage)
	{
		if (ImGui::IsItemHovered())
		{
			_timeSpentHovering += Time::GetTimestep();
			if (_timeSpentHovering >= 0.5f)
			{
				ImGui::SetTooltip(tooltipMessage.c_str());
			}
		}

		if (!ImGui::IsAnyItemHovered())
		{
			_timeSpentHovering = 0.0f;
		}
	}

	void EditorLayer::OnScenePlay()
	{
		_sceneState = SceneState::Play;
		_sceneHierarchyPanel.SetSelectedEntity(Entity(), true);

		_activeScene = Scene::Copy(_editorScene);

		_activeScene->OnRuntimeStart();

		_sceneHierarchyPanel.SetScene(_activeScene);
	}

	void EditorLayer::OnSceneSimulate()
	{
		_sceneState = SceneState::Simulate;

		_activeScene = Scene::Copy(_editorScene);

		_activeScene->OnSimulationStart();

		_sceneHierarchyPanel.SetScene(_activeScene);
	}

	void EditorLayer::OnSceneStop()
	{
		HZ_CORE_ASSERT(_sceneState == SceneState::Play || _sceneState == SceneState::Simulate, "Invalid Scene State.");

		switch (_sceneState)
		{
		case SceneState::Play:
			_activeScene->OnRuntimeStop();
			break;
		default:
			_activeScene->OnSimulationStop();
			break;
		}

		_sceneState = SceneState::Edit;
		_activeScene = _editorScene;

		_sceneHierarchyPanel.SetScene(_activeScene);
	}

	void EditorLayer::DuplicateEntity()
	{
		if (_sceneState != SceneState::Edit)
		{
			return;
		}

		if (auto selectedEntity = _sceneHierarchyPanel.GetSelectedEntity())
		{
			auto newEntity = _activeScene->DuplicateEntity(selectedEntity);
			_sceneHierarchyPanel.SetSelectedEntity(newEntity);
		}
	}
}
