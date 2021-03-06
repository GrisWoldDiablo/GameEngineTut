#include "EditorLayer.h"
#include "NativeScripts.h"

#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "Hazel/Scene/SceneSerializer.h"
#include "Hazel/Utils/PlatformUtils.h"

#include "ImGuizmo.h"
#include "Hazel/Math/Math.h"

namespace Hazel
{
	EditorLayer::EditorLayer()
		: Layer("Hazel Editor")
	{}

	void EditorLayer::OnAttach()
	{
		HZ_PROFILE_FUNCTION();

		// Set Fonts
		auto imGuiLayer = Application::Get().GetImGuiLayer();
		auto normalFontPath = "assets/fonts/opensans/OpenSans-SemiBold.ttf";
		auto boldFontPath = "assets/fonts/opensans/OpenSans-ExtraBold.ttf";
		imGuiLayer->SetFonts(normalFontPath, { boldFontPath });

		// Create Gizmo Icons texture.
		_panIconTexture = Texture2D::Create("assets/icons/PanIcon256White.png");
		_eyeIconTexture = Texture2D::Create("assets/icons/EyeIcon256White.png");
		_nothingGizmoIconTexture = Texture2D::Create("assets/icons/NothingGizmo256White.png");
		_positionGizmoIconTexture = Texture2D::Create("assets/icons/PositionGizmo256White.png");
		_rotationGizmoIconTexture = Texture2D::Create("assets/icons/RotationGizmo256White.png");
		_scaleGizmoIconTexture = Texture2D::Create("assets/icons/ScaleGizmo256White.png");
		_localGizmoIconTexture = Texture2D::Create("assets/icons/LocalGizmo256White.png");
		_globalGizmoIconTexture = Texture2D::Create("assets/icons/GlobalGizmo256White.png");

		_unwrapTexture = Texture2D::Create("assets/textures/unwrap_helper.png");

		auto framebufferSpecification = FramebufferSpecification
		{
			{ FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth },
			1280,
			720
		};
		_framebuffer = Framebuffer::Create(framebufferSpecification);

		// Create an empty scene.
		_activeScene = CreateRef<Scene>();
		_activeScene->SetName(_kNewSceneName);

		auto commandLineArgs = Application::Get().GetCommandLineArgs();
		if (commandLineArgs.Count > 1)
		{
			auto sceneFilePath = commandLineArgs[1];
			if (std::filesystem::exists(sceneFilePath))
			{
				SceneSerializer serializer(_activeScene);
				if (serializer.Deserialize(sceneFilePath))
				{
					SetWindowTitle(sceneFilePath);
				}
			}
			else
			{
				HZ_CORE_LERROR("No scene found at {0}", sceneFilePath);
			}
		}


		_sceneHierarchyPanel.SetScene(_activeScene);
		_editorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);

	}

	void EditorLayer::OnDetach()
	{
		HZ_PROFILE_FUNCTION();
	}

	void EditorLayer::OnUpdate()
	{
		HZ_PROFILE_FUNCTION();
		_updateTimer.Reset();

		auto& frameBufferSpec = _framebuffer->GetSpecification();
		if (_sceneViewportSize.x > 0.0f && _sceneViewportSize.y > 0.0f &&
			(frameBufferSpec.Width != _sceneViewportSize.x || frameBufferSpec.Height != _sceneViewportSize.y))
		{
			_framebuffer->Resize((uint32_t)_sceneViewportSize.x, (uint32_t)_sceneViewportSize.y);

			_editorCamera.SetViewpostSize(_sceneViewportSize.x, _sceneViewportSize.y);
			_activeScene->OnViewportResize((uint32_t)_sceneViewportSize.x, (uint32_t)_sceneViewportSize.y);
		}

		CalculateFPS();

#if !HZ_PROFILE
		//SafetyShutdownCheck();
#endif // !HZ_PROFILE

		_editorCamera.IsEnable() =
			(_isSceneViewportHovered && _isSceneViewportFocused && !ImGuizmo::IsUsing())
			|| _editorCamera.IsAdjusting();
		_editorCamera.OnUpdate();

		Renderer2D::ResetStats();
		// Render
		_framebuffer->Bind();
		RenderCommand::SetClearColor(_clearColor);
		RenderCommand::Clear();

		// Clear our entity ID attachment to -1
		_framebuffer->ClearAttachment(1, -1);

		// Update Scene
		_activeScene->OnUpdateEditor(_editorCamera);

		MousePicking();

		_framebuffer->Unbind();

		_updateTimerElapsedMillis = _updateTimer.ElapsedMillis();
	}

	void EditorLayer::OnImGuiRender()
	{
		HZ_PROFILE_FUNCTION();

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
		style.WindowMinSize.x = 370.0f;

		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize = originalWindowMinSize;

		DrawFileMenu();
		DrawStats();
		DrawSceneViewport();
		DrawTools();

		_sceneHierarchyPanel.OnImGuiRender();
		_contentBrowserPanel.OnImGuiRender();

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& event)
	{
		_editorCamera.OnEvent(event);

		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<KeyPressedEvent>(HZ_BIND_EVENT_FN(OnKeyPressed));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(HZ_BIND_EVENT_FN(OnMouseButtonReleased));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& event)
	{
		if (ImGui::GetIO().WantTextInput || event.GetRepeatCount() > 0 || Input::IsMouseButtonPressed(MouseCode::ButtonRight))
		{
			return false;
		}

		bool isImGuizmoInUse = ImGuizmo::IsUsing();
		bool isControlPressed = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool isShiftPressed = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);
		bool isModiferPressed = isControlPressed || isShiftPressed;

		switch (event.GetKeyCode())
		{
		case Key::N:
			if (isControlPressed && !isShiftPressed)
			{
				NewScene();
			}
			break;
		case Key::O:
			if (isControlPressed && !isShiftPressed)
			{
				OpenScene();
			}
			break;
		case Key::S:
			if (isControlPressed && isShiftPressed)
			{
				SaveSceneAs();
			}
			break;
			// Gizmos 
		case Key::Q: // Nothing
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
		case Key::E: // Rotation
			if (!isModiferPressed && !isImGuizmoInUse)
			{
				_gizmoType = ImGuizmo::ROTATE;
			}
			break;
		case Key::R: // Scale
			if (!isModiferPressed && !isImGuizmoInUse)
			{
				_gizmoType = ImGuizmo::SCALE;
				_gizmoSpace = ImGuizmo::LOCAL;
			}
			break;
		case Key::X: // space toggle, Local or Global
			if (!isModiferPressed && !isImGuizmoInUse && _gizmoType != ImGuizmo::SCALE)
			{
				_gizmoSpace = _gizmoSpace == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
			}
			break;
		}

		return true;
	}

	bool EditorLayer::OnMouseButtonReleased(MouseButtonReleasedEvent& event)
	{
		// Mouse picking
		if (event.GetMouseButton() == Mouse::ButtonLeft)
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

		glm::vec2 sceneViewpostSize = _sceneViewportBounds[1] - _sceneViewportBounds[0];
		imMouseY = sceneViewpostSize.y - imMouseY;
		auto mouseX = (int)imMouseX;
		auto mouseY = (int)imMouseY;

		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)sceneViewpostSize.x && mouseY < (int)sceneViewpostSize.y)
		{
			int pixelData = _framebuffer->ReadPixel(1, mouseX, mouseY);
			_hoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, _activeScene.get());
		}
	}

	bool EditorLayer::NewScene(const std::string& newSceneName)
	{
		if (FileDialogs::NewFile())
		{
			_activeScene = CreateRef<Scene>();

			if (newSceneName.empty() || std::all_of(newSceneName.begin(), newSceneName.end(), [](char c)
			{
				return std::isspace(static_cast<unsigned char>(c));
			}))
			{
				_activeScene->SetName(newSceneName);
			}
			else
			{
				_activeScene->SetName(_kNewSceneName);
			}

			_activeScene->OnViewportResize((uint32_t)_sceneViewportSize.x, (uint32_t)_sceneViewportSize.y);
			_sceneHierarchyPanel.SetScene(_activeScene);
			// TODO save camera position in scene and reload it.
			_editorCamera.Reset();

			return true;
		}

		return false;
	}

	void EditorLayer::OpenScene()
	{
		auto filePath = FileDialogs::OpenFile("Hazel Scene (*.hazel)\0*.hazel\0");
		if (!filePath.empty())
		{
			if (!NewScene())
			{
				return;
			}

			SceneSerializer serializer(_activeScene);
			if (serializer.Deserialize(filePath))
			{
				SetWindowTitle(filePath);
			}
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		auto sceneName = _activeScene->GetName();
		auto filePath = FileDialogs::SaveFile("Hazel Scene (*.hazel)\0*.hazel\0", !sceneName.empty() ? sceneName.c_str() : nullptr);

		if (!filePath.empty())
		{
			SetWindowTitle(filePath);

			SceneSerializer serializer(_activeScene);
			serializer.Serialize(filePath);
		}
	}

	void EditorLayer::SetWindowTitle(const std::string& filePath)
	{
		auto& window = Application::Get().GetWindow();
		std::stringstream ss;
		ss << window.GetTitle() << " " << filePath;
		window.SetTitle(ss.str());
	}

	void EditorLayer::DrawToolbar()
	{
		auto size = ImVec2(25.0f, 25.0f);
		auto uv0 = ImVec2(0.0f, 1.0f);
		auto uv1 = ImVec2(1.0f, 0.0f);

		auto selectedColor = ImVec4(0.196f, 0.196f, 0.5f, 1.0f);
		auto tintColor = ImVec4(0.396f, 0.396f, 0.8f, 1.0f);
		auto normalColor = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		auto whiteColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

		auto tableFlags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;
		if (ImGui::BeginTable("Toolbar", 3, tableFlags))
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TableNextColumn();

			ImTextureID toolbarButtonOneTexture;
			if (_editorCamera.IsAdjusting())
			{
				toolbarButtonOneTexture = (ImTextureID)(intptr_t)_eyeIconTexture->GetRendererID();
				if (!_hasStoredPreviousGizmoType)
				{
					_previousGizmoType = _gizmoType;
					_hasStoredPreviousGizmoType = true;
				}
				_gizmoType = -1;
			}
			else
			{
				toolbarButtonOneTexture = (ImTextureID)(intptr_t)_nothingGizmoIconTexture->GetRendererID();
				if (_hasStoredPreviousGizmoType)
				{
					_gizmoType = _previousGizmoType;
					_hasStoredPreviousGizmoType = false;
					_previousGizmoType = -1;
				}
			}

			bool isNothing = _gizmoType == -1;
			if (ImGui::ImageButton(toolbarButtonOneTexture, size, uv0, uv1, 3, isNothing ? selectedColor : normalColor, isNothing ? tintColor : whiteColor))
			{
				_gizmoType = -1;
			}
			AddTooltip("Nothing");

			ImGui::SameLine();

			bool isTranslate = _gizmoType == ImGuizmo::TRANSLATE;
			if (ImGui::ImageButton((ImTextureID)(intptr_t)_positionGizmoIconTexture->GetRendererID(), size, uv0, uv1, 3, isTranslate ? selectedColor : normalColor, isTranslate ? tintColor : whiteColor))
			{
				_gizmoType = ImGuizmo::TRANSLATE;
			}
			AddTooltip("Translate");

			ImGui::SameLine();
			bool isRotate = _gizmoType == ImGuizmo::ROTATE;
			if (ImGui::ImageButton((ImTextureID)(intptr_t)_rotationGizmoIconTexture->GetRendererID(), size, uv0, uv1, 3, isRotate ? selectedColor : normalColor, isRotate ? tintColor : whiteColor))
			{
				_gizmoType = ImGuizmo::ROTATE;
			}
			AddTooltip("Rotate");

			ImGui::SameLine();
			bool isScale = _gizmoType == ImGuizmo::SCALE;
			if (ImGui::ImageButton((ImTextureID)(intptr_t)_scaleGizmoIconTexture->GetRendererID(), size, uv0, uv1, 3, isScale ? selectedColor : normalColor, isScale ? tintColor : whiteColor))
			{
				_gizmoType = ImGuizmo::SCALE;
				_gizmoSpace = ImGuizmo::LOCAL;
			}
			AddTooltip("Scale\nLocal Space Only");

			ImGui::TableNextColumn();
			bool isLocal = _gizmoSpace == ImGuizmo::LOCAL;
			if (isLocal)
			{
				if (ImGui::ImageButton((ImTextureID)(intptr_t)_localGizmoIconTexture->GetRendererID(), size, uv0, uv1, 3, selectedColor, tintColor))
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
				if (ImGui::ImageButton((ImTextureID)(intptr_t)_globalGizmoIconTexture->GetRendererID(), size, uv0, uv1, 3, selectedColor, tintColor))
				{
					_gizmoSpace = ImGuizmo::LOCAL;
				}

				AddTooltip("Change to Local");

				ImGui::SameLine();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Global");
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
					NewScene();
				}

				if (ImGui::MenuItem("Open...", "Ctrl+O"))
				{
					OpenScene();
				}

				//if (ImGui::MenuItem("Save", "Ctrl+S"))
				//{
				//	SceneSerializer serializer(_activeScene);
				//	serializer.Serialize("assets/scenes/Example.hazel");
				//}

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
					if (ImGui::MenuItem("Load"))
					{
						auto filePath = FileDialogs::OpenFile("Shader (*.glsl)\0*.glsl\0");
						if (!filePath.empty())
						{
							Renderer2D::LoadShader(filePath);
						}
					}

					if (ImGui::MenuItem("Load & Recompile"))
					{
						auto filePath = FileDialogs::OpenFile("Shader (*.glsl)\0*.glsl\0");
						if (!filePath.empty())
						{
							Renderer2D::LoadShader(filePath, true);
						}
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
		Application::Get().GetImGuiLayer()->BlockEvents(!_isSceneViewportFocused && !_isSceneViewportHovered);

		auto sceneViewportPanelSize = ImGui::GetContentRegionAvail();
		_sceneViewportSize = { sceneViewportPanelSize.x, sceneViewportPanelSize.y };

		auto textureID = _framebuffer->GetColorAttachmentRenderID();
		ImGui::Image((void*)((uint64_t)textureID), { _sceneViewportSize.x, _sceneViewportSize.y }, ImVec2{ 0.0f, 1.0f }, ImVec2{ 1.0f, 0.0f });

		auto windowSize = ImGui::GetWindowSize();
		auto minBound = ImGui::GetWindowPos();
		auto maxBound = ImVec2{ minBound.x + windowSize.x, minBound.y + windowSize.y };
		minBound.x += sceneViewportOffset.x;
		minBound.y += sceneViewportOffset.y;

		_sceneViewportBounds[0] = { minBound.x, minBound.y };
		_sceneViewportBounds[1] = { maxBound.x, maxBound.y };

#pragma region Gizmo
		if (auto selectedEntity = _sceneHierarchyPanel.GetSelectedEntity();	selectedEntity &&
			(_gizmoType != -1 || (_hasStoredPreviousGizmoType && _previousGizmoType != -1)))
		{
			//// Runtime Camera;
			//if (auto cameraEntity = _activeScene->GetPrimaryCameraEntity();
			//	cameraEntity != Entity::Null)
			//{
			//	const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
			//	const auto& cameraProjection = camera.GetProjection();
			//	glm::mat4 cameraView = glm::inverse(cameraEntity.Transform().GetTransform());
			//}

			// Editor Camera
			const auto& cameraProjection = _editorCamera.GetProjection();
			glm::mat4 cameraView = _editorCamera.GetViewMatrix();

			ImGuizmo::SetOrthographic(false);
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
				if (Math::DecomposeTransform(transform, position, rotation, scale))
				{
					transformComponent.Position = position;
					auto rotationDelta = rotation - transformComponent.Rotation;
					transformComponent.Rotation += rotationDelta;
					transformComponent.Scale = scale;
				}
			}
		}
#pragma endregion

#pragma region CameraDriving
		// TODO Draw Magnifying and Hand cursor when zooming and panning
// Draw Eye cursor and move speed when driving the camera
		if (_editorCamera.IsDriving() && ImGui::IsItemHovered())
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_None);

			auto mousePos = ImGui::GetMousePos();

			// Cursor
			auto upperLeftPos = mousePos;
			upperLeftPos.y -= 10.0f;
			upperLeftPos.x -= 10.0f;
			auto lowerRightPos = upperLeftPos;
			lowerRightPos.y += 25.0f;
			lowerRightPos.x += 25.0f;
			auto uv0 = ImVec2(0.0f, 1.0f);
			auto uv1 = ImVec2(1.0f, 0.0f);
			ImGui::GetForegroundDrawList()->AddImage((ImTextureID)(intptr_t)_eyeIconTexture->GetRendererID(), upperLeftPos, lowerRightPos, uv0, uv1, IM_COL32_WHITE);

			// Move Speed
			std::stringstream ss;
			ss << _editorCamera.GetDrivingSpeed();
			mousePos.y += 10.0f;
			mousePos.x += 10.0f;
			ImGui::GetForegroundDrawList()->AddText(mousePos, IM_COL32_WHITE, ss.str().c_str());
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
		ImGui::Text("Ms per frame: %.3f", _updateTimerElapsedMillis);

		ImGui::End();
	}

	void EditorLayer::DrawTools()
	{
		ImGui::Begin("Tools");

		if (ImGui::Button("Show Demo Window"))
		{
			_isDemoWidowOpen = !_isDemoWidowOpen;
		}

		if (_isDemoWidowOpen)
		{
			ImGui::ShowDemoWindow(&_isDemoWidowOpen);
		}
		ImGui::Separator();

		//Since we can remove and add component need to disable this for now, to revamp later.
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
				auto& newEntity = _activeScene->CreateEntity("Square " + std::to_string(i) + ":" + color.GetHexValue());
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
		const float kRequiredTime = 0.5f;
		if (ImGui::IsItemHovered())
		{
			_timeSpentHovering += Time::GetTimestep();
			if (_timeSpentHovering >= kRequiredTime)
			{
				ImGui::SetTooltip(tooltipMessage.c_str());
			}
		}

		if (!ImGui::IsAnyItemHovered())
		{
			_timeSpentHovering = 0.0f;
		}
	}
}
