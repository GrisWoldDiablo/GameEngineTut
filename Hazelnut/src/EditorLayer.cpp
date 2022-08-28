#include "EditorLayer.h"
#include "NativeScripts.h"

#include "Hazel/Scene/SceneSerializer.h"
#include "Hazel/Utils/PlatformUtils.h"
#include "Hazel/Math/HMath.h"
#include "Hazel/Audio/AudioEngine.h"

#include <imgui/imgui.h>
#include "ImGuizmo.h"

#include <glm/gtc/type_ptr.hpp>

namespace Hazel
{
	EditorLayer::EditorLayer()
		: Layer("Hazel Editor")
	{
		// Create Gizmo Icons texture.
		_iconTextures =
		{
			{ Icons::Pan,		Texture2D::Create("Resources/Icons/Gizmo/PanIcon256White.png")			},
			{ Icons::Magnifier, Texture2D::Create("Resources/Icons/Gizmo/MagnifierIcon256White.png")	},
			{ Icons::Eye,		Texture2D::Create("Resources/Icons/Gizmo/EyeIcon256White.png")			},
			{ Icons::Nothing,	Texture2D::Create("Resources/Icons/Gizmo/NothingGizmo256White.png")		},
			{ Icons::Position,	Texture2D::Create("Resources/Icons/Gizmo/PositionGizmo256White.png")	},
			{ Icons::Rotation,	Texture2D::Create("Resources/Icons/Gizmo/RotationGizmo256White.png")	},
			{ Icons::Scale,		Texture2D::Create("Resources/Icons/Gizmo/ScaleGizmo256White.png")		},
			{ Icons::Local,		Texture2D::Create("Resources/Icons/Gizmo/LocalGizmo256White.png")		},
			{ Icons::Global,	Texture2D::Create("Resources/Icons/Gizmo/GlobalGizmo256White.png")		},
			{ Icons::Play,		Texture2D::Create("Resources/Icons/General/PlayButton256.png")			},
			{ Icons::Stop,		Texture2D::Create("Resources/Icons/General/StopButton256.png")			},
			{ Icons::Simulate,	Texture2D::Create("Resources/Icons/General/SimulateButton256.png")		},
		};

		_shaderLoadingTexture = Texture2D::Create("Resources/ShadersLoading.png");
	}

	void EditorLayer::OnAttach()
	{
		HZ_PROFILE_FUNCTION();

		// Set Fonts
		auto imGuiLayer = Application::Get().GetImGuiLayer();
		auto normalFontPath = "Resources/Fonts/opensans/OpenSans-SemiBold.ttf";
		auto boldFontPath = "Resources/Fonts/opensans/OpenSans-ExtraBold.ttf";
		imGuiLayer->SetFonts(normalFontPath, { boldFontPath });

		auto framebufferSpecification = FramebufferSpecification
		{
			{ FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth },
			1280,
			720
		};
		_framebuffer = Framebuffer::Create(framebufferSpecification);

		const auto commandLineArgs = Application::Get().GetSpecification().CommandLineArgs;
		if (commandLineArgs.Count > 1)
		{
			auto sceneFilePath = commandLineArgs[1];
			if (std::filesystem::exists(sceneFilePath))
			{
				OpenScene(sceneFilePath, false);
			}
			else
			{
				HZ_CORE_LERROR("No scene found at {0}", sceneFilePath);
				NewScene();
			}
		}
		else
		{
			NewScene();
		}

		_editorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		Renderer2D::SetLineWidth(4.0f);

		Application::Get().GetImGuiLayer()->BlockEvents(false);
	}

	void EditorLayer::OnDetach()
	{
		HZ_PROFILE_FUNCTION();
	}

	void EditorLayer::OnUpdate(Timestep timestep)
	{
		HZ_PROFILE_FUNCTION();
		_updateTimer.Reset();

		auto& frameBufferSpec = _framebuffer->GetSpecification();
		if (_sceneViewportSize.x > 0.0f && _sceneViewportSize.y > 0.0f &&
			(frameBufferSpec.Width != static_cast<uint32_t>(_sceneViewportSize.x) || frameBufferSpec.Height != static_cast<uint32_t>(_sceneViewportSize.y)))
		{
			_framebuffer->Resize(static_cast<uint32_t>(_sceneViewportSize.x), static_cast<uint32_t>(_sceneViewportSize.y));

			_editorCamera.SetViewportSize(_sceneViewportSize.x, _sceneViewportSize.y);
			_activeScene->OnViewportResize(static_cast<uint32_t>(_sceneViewportSize.x), static_cast<uint32_t>(_sceneViewportSize.y));
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
		}	break;
		case SceneState::Simulate:
		{
			_editorCamera.IsEnable() =
				(_isSceneViewportHovered && _isSceneViewportFocused && !ImGuizmo::IsUsing())
				|| _editorCamera.IsAdjusting();
			_editorCamera.OnUpdate(timestep);

			// Update Scene
			_activeScene->OnUpdateSimulation(timestep, _editorCamera);
		}	break;
		case SceneState::Play:
		{
			_activeScene->OnUpdateRuntime(timestep);
		}	break;
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
			ImGuiViewport* viewport = ImGui::GetMainViewport();
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
		DrawSceneViewport();
		DrawTools();

		_sceneHierarchyPanel.OnImGuiRender();
		_contentBrowserPanel.OnImGuiRender();

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
		if (_isSceneViewportFocused && _sceneState == SceneState::Edit)
		{
			_editorCamera.OnEvent(event);
		}

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(HZ_BIND_EVENT_FN(OnKeyPressed));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(HZ_BIND_EVENT_FN(OnMouseButtonReleased));
	}

	bool EditorLayer::OnKeyPressed(const KeyPressedEvent& keyPressedEvent)
	{
		if (ImGui::GetIO().WantTextInput || keyPressedEvent.IsRepeat() || Input::IsMouseButtonPressed(MouseCode::ButtonRight))
		{
			return false;
		}

		bool isImGuizmoInUse = ImGuizmo::IsUsing();
		bool isControlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool isShiftPressed = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
		bool isModiferPressed = isControlPressed || isShiftPressed;

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
					SaveSceneAs();
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
		// TODO make this work, requires to make events no blocked when scene view not in focus, or more like having exceptions.
		//case Key::Escape:
		//	auto payload = ImGui::GetDragDropPayload();
		//	if (payload != nullptr)
		//	{
		//		ImGui::AcceptDragDropPayload(payload->DataType);
		//	}
		//	break;
		}

		return true;
	}

	bool EditorLayer::OnMouseButtonReleased(const MouseButtonReleasedEvent& mouseButtonReleasedEvent)
	{
		// Mouse picking
		if (mouseButtonReleasedEvent.GetMouseButton() == Mouse::ButtonLeft && _sceneState == SceneState::Edit)
		{
			if (_isSceneViewportHovered && (!_sceneHierarchyPanel.GetSelectedEntity() || !ImGuizmo::IsOver()) && !Input::IsKeyPressed(Key::LeftAlt))
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

		glm::vec2 sceneViewportSize = _sceneViewportBounds[1] - _sceneViewportBounds[0];
		imMouseY = sceneViewportSize.y - imMouseY;
		auto mouseX = (int)imMouseX;
		auto mouseY = (int)imMouseY;

		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)sceneViewportSize.x && mouseY < (int)sceneViewportSize.y)
		{
			int pixelData = _framebuffer->ReadPixel(1, mouseX, mouseY);

			_hoveredEntity = pixelData == -1 ? Entity() : _activeScene->CheckEntityValidity((entt::entity)pixelData) ? Entity((entt::entity)pixelData, _activeScene.get()) : Entity();
		}
	}

	void EditorLayer::OnOverlayRender()
	{
		float cameraPositionZ = 1.0f;
		bool hasSceneBegun;

		if (_sceneState == SceneState::Play)
		{
			auto primaryCamera = _activeScene->GetPrimaryCameraEntity();
			if (!primaryCamera)
			{
				return;
			}

			auto& camera = primaryCamera.GetComponent<CameraComponent>().Camera;
			auto& transformComponent = primaryCamera.GetComponent<TransformComponent>();
			auto transform = transformComponent.GetTransformMatrix();
			if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				cameraPositionZ = transformComponent.Position.z;
			}

			hasSceneBegun = Renderer2D::BeginScene(camera, transform);
		}
		else
		{
			cameraPositionZ = _editorCamera.GetPosition().z;
			hasSceneBegun = Renderer2D::BeginScene(_editorCamera);
		}

		if (!hasSceneBegun)
		{
			return;
		}

		if (_shouldShowPhysicsColliders)
		{
			constexpr auto kIdentityMatrix = glm::mat4(1.0f);

			// Box Colliders
			{
				auto view = _activeScene->GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
				for (auto entity : view)
				{
					auto [tc, bc2d] = view.get<TransformComponent, BoxCollider2DComponent>(entity);

					float sign = cameraPositionZ > tc.Position.z ? 1.0f : -1.0f;

					glm::mat4 transform = glm::translate(kIdentityMatrix, tc.Position)
						* glm::toMat4(glm::quat(tc.Rotation))
						* glm::translate(kIdentityMatrix, glm::vec3(bc2d.Offset, 0.001f * sign))
						* glm::toMat4(glm::quat(glm::vec3(0.0f, 0.0f, glm::radians(bc2d.Rotation))))
						* glm::scale(kIdentityMatrix, tc.Scale * glm::vec3(bc2d.Size * 2.0f, 1.0f));


					Renderer2D::DrawRect(transform, Color::Green);
				}
			}

			// Circle Colliders
			{
				auto view = _activeScene->GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
				for (auto entity : view)
				{
					auto [tc, cc2d] = view.get<TransformComponent, CircleCollider2DComponent>(entity);

					float sign = cameraPositionZ > tc.Position.z ? 1.0f : -1.0f;

					glm::mat4 transform = glm::translate(kIdentityMatrix, tc.Position)
						* glm::toMat4(glm::quat(tc.Rotation))
						* glm::translate(kIdentityMatrix, glm::vec3(cc2d.Offset, 0.001f * sign))
						* glm::scale(kIdentityMatrix, tc.Scale * glm::vec3(cc2d.Radius * 2.0f));

					Renderer2D::DrawCircle(transform, Color::Green, 0.01f);
				}
			}
		}

		if (auto selectedEntity = _sceneHierarchyPanel.GetSelectedEntity())
		{
			const auto& transform = selectedEntity.GetComponent<TransformComponent>();

			Renderer2D::DrawRect(transform.GetTransformMatrix(), Color::Orange);
		}

		Renderer2D::EndScene();
	}

	bool EditorLayer::ClearSceneCheck()
	{
		const char* message = "You will lose current scene's unsaved progress.\nDo you want to continue?";
		const char* title = "Warning!";
		return FileDialogs::QuestionPopup(message, title);
	}

	void EditorLayer::NewScene(bool withCheck)
	{
		if (withCheck && !ClearSceneCheck())
		{
			return;
		}

		if (_sceneState != SceneState::Edit)
		{
			OnSceneStop();
		}

		_editorScene = CreateRef<Scene>();
		_editorScene->SetName(kNewSceneName);

		SetWindowTitleSceneName();

		_activeScene = _editorScene;
		_sceneHierarchyPanel.SetScene(_activeScene);

		_editorScenePath = std::filesystem::path();
	}

	void EditorLayer::OpenScene()
	{
		auto filePath = FileDialogs::OpenFile("Hazel Scene (*.hazel)\0*.hazel\0");
		if (!filePath.empty())
		{
			OpenScene(filePath, true);
		}
	}

	void EditorLayer::OpenScene(const std::filesystem::path& path, bool withCheck)
	{
		if (path.extension().string() != ".hazel")
		{
			HZ_CORE_LWARN("Could not load {0} - not a scene file.", path.filename().string());
			return;
		}

		if (withCheck && !ClearSceneCheck())
		{
			return;
		}

		if (_sceneState != SceneState::Edit)
		{
			OnSceneStop();
		}

		_editorScene = CreateRef<Scene>();
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
			SaveSceneAs();
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		if (_sceneState != SceneState::Edit)
		{
			FileDialogs::MessagePopup("You can't save while in play mode!", "Error");
			return;
		}

		auto sceneName = _activeScene->GetName();
		auto filePath = FileDialogs::SaveFile("Hazel Scene (*.hazel)\0*.hazel\0", !sceneName.empty() ? sceneName.c_str() : nullptr);

		if (!filePath.empty())
		{
			_editorScenePath = std::filesystem::path(filePath);
			SerializeScene();
			SetWindowTitleSceneName(_editorScenePath);
		}
	}

	void EditorLayer::SerializeScene()
	{
		SceneSerializer serializer(_activeScene);
		serializer.Serialize(_editorScenePath.string());
	}

	void EditorLayer::SetWindowTitleSceneName(const std::filesystem::path& scenePath)
	{
		auto& window = Application::Get().GetWindow();
		auto fileName = scenePath.empty() ? "Unsaved" : scenePath.stem();
		std::stringstream ss;
		ss << window.GetTitle() << " " << fileName;
		window.SetTitle(ss.str());
	}

	void EditorLayer::DrawToolbar()
	{
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
					gizmoButtonTexture = _iconTextures[Icons::Pan];
				}
				else if (_editorCamera.IsZooming())
				{
					gizmoButtonTexture = _iconTextures[Icons::Magnifier];
				}
				else
				{
					gizmoButtonTexture = _iconTextures[Icons::Eye];
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
				gizmoButtonTexture = _iconTextures[Icons::Nothing];
				if (_hasStoredPreviousGizmoType)
				{
					_gizmoType = _previousGizmoType;
					_hasStoredPreviousGizmoType = false;
					_previousGizmoType = -1;
				}
			}

			bool isNothing = _gizmoType == -1;
			if (ImGui::ImageButton((ImTextureID)(intptr_t)gizmoButtonTexture->GetRendererID(), size, uv0, uv1, 3, isNothing ? selectedColor : normalColor, isNothing ? tintColor : whiteColor))
			{
				_gizmoType = -1;
			}
			AddTooltip("Nothing");

			ImGui::SameLine();

			bool isTranslate = _gizmoType == ImGuizmo::TRANSLATE;
			if (ImGui::ImageButton((ImTextureID)(intptr_t)_iconTextures[Icons::Position]->GetRendererID(), size, uv0, uv1, 3, isTranslate ? selectedColor : normalColor, isTranslate ? tintColor : whiteColor))
			{
				_gizmoType = ImGuizmo::TRANSLATE;
			}
			AddTooltip("Translate");

			ImGui::SameLine();
			bool isRotate = _gizmoType == ImGuizmo::ROTATE;
			if (ImGui::ImageButton((ImTextureID)(intptr_t)_iconTextures[Icons::Rotation]->GetRendererID(), size, uv0, uv1, 3, isRotate ? selectedColor : normalColor, isRotate ? tintColor : whiteColor))
			{
				_gizmoType = ImGuizmo::ROTATE;
			}
			AddTooltip("Rotate");

			ImGui::SameLine();
			bool isScale = _gizmoType == ImGuizmo::SCALE;
			if (ImGui::ImageButton((ImTextureID)(intptr_t)_iconTextures[Icons::Scale]->GetRendererID(), size, uv0, uv1, 3, isScale ? selectedColor : normalColor, isScale ? tintColor : whiteColor))
			{
				_gizmoType = ImGuizmo::SCALE;
				_gizmoSpace = ImGuizmo::LOCAL;
			}
			AddTooltip("Scale\nLocal Space Only");

			ImGui::TableNextColumn();
			if (_gizmoSpace == ImGuizmo::LOCAL)
			{
				if (ImGui::ImageButton((ImTextureID)(intptr_t)_iconTextures[Icons::Local]->GetRendererID(), size, uv0, uv1, 3, selectedColor, tintColor))
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
				if (ImGui::ImageButton((ImTextureID)(intptr_t)_iconTextures[Icons::Global]->GetRendererID(), size, uv0, uv1, 3, selectedColor, tintColor))
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

			bool isEditing = _sceneState == SceneState::Edit || _sceneState == SceneState::Simulate;
			auto& sceneStateButton = isEditing ? _iconTextures[Icons::Play] : _iconTextures[Icons::Stop];

			if (ImGui::ImageButton((ImTextureID)(intptr_t)sceneStateButton->GetRendererID(), size, uv0, uv1, 3, isEditing ? normalColor : selectedColor, isEditing ? whiteColor : tintColor))
			{
				switch (_sceneState)
				{
				case SceneState::Edit:
					OnScenePlay();
					break;
				case SceneState::Play:
					OnSceneStop();
					break;
				case SceneState::Simulate:
					OnSceneStop();
					OnScenePlay();
					break;
				}
			}

			ImGui::SameLine();
			if (ImGui::ImageButton((ImTextureID)(intptr_t)_iconTextures[Icons::Simulate]->GetRendererID(), size, uv0, uv1, 3, isEditing ? normalColor : selectedColor, _sceneState != SceneState::Simulate ? whiteColor : tintColor))
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

			ImGui::EndTable();
		}
	}

	void EditorLayer::DrawFileMenu()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Ctrl+N"))
				{
					NewScene(true);
				}

				if (ImGui::MenuItem("Open...", "Ctrl+O"))
				{
					OpenScene();
				}

				if (ImGui::MenuItem("Save", "Ctrl+S"))
				{
					SaveScene();
				}

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
				{
					SaveSceneAs();
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
						std::stringstream ss;
						const auto rendererShaderType = static_cast<RendererShader>(i);
						ss << "Reload " << sRendererShaderName[rendererShaderType];

						if (ImGui::MenuItem(ss.str().c_str()))
						{
							Renderer2D::ReloadShader(rendererShaderType);
						}
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Script Engine"))
				{
					switch (_sceneState)
					{
					case Hazel::EditorLayer::SceneState::Edit:
					{
						if (ImGui::MenuItem("Reload"))
						{
							Application::Get().ReloadScriptEngine();
						}
						break;
					}
					case Hazel::EditorLayer::SceneState::Play:
					case Hazel::EditorLayer::SceneState::Simulate:
					default:
						ImGui::TextDisabled("Reload");
						break;
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
	}

	void EditorLayer::DrawSceneViewport()
	{
		HZ_PROFILE_FUNCTION();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });

		ImGui::Begin("SceneViewport");

		auto sceneViewportOffset = ImGui::GetCursorPos(); // Include tab bar.

		_isSceneViewportFocused = ImGui::IsWindowFocused();
		_isSceneViewportHovered = ImGui::IsWindowHovered();
		if (_isSceneViewportHovered && (ImGui::GetIO().MouseClicked[1] || ImGui::GetIO().MouseClicked[2]))
		{
			ImGui::SetWindowFocus();
		}
		// TODO revisit
		//Application::Get().GetImGuiLayer()->BlockEvents(!_isSceneViewportFocused && !_isSceneViewportHovered);

		auto sceneViewportPanelSize = ImGui::GetContentRegionAvail();
		_sceneViewportSize = { sceneViewportPanelSize.x, sceneViewportPanelSize.y };

		if (Renderer2D::IsReady())
		{
			auto textureID = _framebuffer->GetColorAttachmentRenderID();
			ImGui::Image((void*)(uint64_t)textureID, { _sceneViewportSize.x, _sceneViewportSize.y }, ImVec2{ 0.0f, 1.0f }, ImVec2{ 1.0f, 0.0f });
		}
		else
		{
			ImGui::Image((void*)(uint64_t)_shaderLoadingTexture->GetRendererID(), { _sceneViewportSize.x, _sceneViewportSize.y }, ImVec2{ 0.0f, 1.0f }, ImVec2{ 1.0f, 0.0f });
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				std::filesystem::path filePath = (const wchar_t*)payload->Data;

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
		auto maxBound = ImVec2{ minBound.x + windowSize.x, minBound.y + windowSize.y };
		minBound.x += sceneViewportOffset.x;
		minBound.y += sceneViewportOffset.y;

		_sceneViewportBounds[0] = { minBound.x, minBound.y };
		_sceneViewportBounds[1] = { maxBound.x, maxBound.y };

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
						cameraView = glm::inverse(cameraEntity.Transform().GetTransformMatrix());
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

				auto windowWidth = (float)ImGui::GetWindowWidth();
				auto windowHeight = (float)ImGui::GetWindowHeight();
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);


				// Entity transform
				auto& transformComponent = selectedEntity.Transform();
				auto transform = transformComponent.GetTransformMatrix();

				// Snapping
				bool snap = Input::IsKeyPressed(Key::LeftControl);
				float snapValue = 0.5f; // Snap to 0.5m for position and scale.

				// Snap to 45 degrees for rotation.
				if (_gizmoType == ImGuizmo::ROTATE)
				{
					snapValue = 45.0f;
				}

				float snapValues[3] = { snapValue,snapValue,snapValue };
				int gizmoType = _hasStoredPreviousGizmoType ? _previousGizmoType : _gizmoType;
				ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
					(ImGuizmo::OPERATION)gizmoType, (ImGuizmo::MODE)_gizmoSpace, glm::value_ptr(transform),
					nullptr, snap ? snapValues : nullptr);

				if (ImGuizmo::IsUsing() && !_hasStoredPreviousGizmoType)
				{
					glm::vec3 position, rotation, scale;
					if (HMath::DecomposeTransform(transform, position, rotation, scale))
					{
						transformComponent.Position = position;
						auto rotationDelta = rotation - transformComponent.Rotation;
						transformComponent.Rotation += rotationDelta;
						transformComponent.Scale = scale;
					}
				}
			}
		}
#pragma endregion

#pragma region CameraDriving
		// TODO Draw Magnifying and Hand cursor when zooming and panning
		// Draw Eye cursor and move speed when driving the camera
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

			Ref<Texture2D> cursorTexture = _editorCamera.IsPanning() ? _iconTextures[Icons::Pan] : _editorCamera.IsZooming() ? _iconTextures[Icons::Magnifier] : _iconTextures[Icons::Eye];

			if (_editorCamera.IsDriving())
			{
				// Move Speed
				std::stringstream ss;
				ss << _editorCamera.GetDrivingSpeed();
				if (Input::IsKeyPressed(Key::LeftShift))
				{
					ss << "*";
				}
				mousePos.y += 10.0f;
				mousePos.x += 10.0f;
				ImGui::GetForegroundDrawList()->AddText(mousePos, IM_COL32_WHITE, ss.str().c_str());
			}

			ImGui::GetForegroundDrawList()->AddImage((ImTextureID)(intptr_t)cursorTexture->GetRendererID(), upperLeftPos, lowerRightPos, { 0.0f, 1.0f }, { 1.0f, 0.0f }, IM_COL32_WHITE);
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
		bool shouldKeepPlaying = _sceneHierarchyPanel.GetShouldKeepPlaying();
		ImGui::Checkbox("Play AudioSource without selection", &shouldKeepPlaying);
		_sceneHierarchyPanel.SetShouldKeepPlaying(shouldKeepPlaying);

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
		/*auto& mainCamera = _mainCamera.GetComponent<CameraComponent>();
		auto& secondaryCamera = _secondaryCamera.GetComponent<CameraComponent>();

		if (ImGui::Checkbox("Secondary Camera", &_isOnSecondCamera))
		{
			mainCamera.IsPrimary = !_isOnSecondCamera;
			secondaryCamera.IsPrimary = _isOnSecondCamera;
		}*/
		ImGui::Separator();

		if (ImGui::Button("Create 50 squares"))
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
		if (_sceneHierarchyPanel.GetShouldKeepPlaying())
		{
			AudioEngine::StopAllAudioSources();
		}

		_sceneState = SceneState::Play;

		_activeScene = Scene::Copy(_editorScene);

		_activeScene->OnViewportResize((uint32_t)_sceneViewportSize.x, (uint32_t)_sceneViewportSize.y);
		_activeScene->OnRuntimeStart();

		_sceneHierarchyPanel.SetScene(_activeScene);
	}

	void EditorLayer::OnSceneSimulate()
	{
		AudioEngine::StopAllAudioSources();

		_sceneState = SceneState::Simulate;

		_activeScene = Scene::Copy(_editorScene);

		_activeScene->OnViewportResize((uint32_t)_sceneViewportSize.x, (uint32_t)_sceneViewportSize.y);
		_activeScene->OnSimulationStart();

		_sceneHierarchyPanel.SetScene(_activeScene);
	}

	void EditorLayer::OnSceneStop()
	{
		AudioEngine::StopAllAudioSources();

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
		if (auto selectedEntity = _sceneHierarchyPanel.GetSelectedEntity())
		{
			auto newEntity = _activeScene->DuplicateEntity(selectedEntity);
			_sceneHierarchyPanel.SetSelectedEntity(newEntity);
		}
	}
}
