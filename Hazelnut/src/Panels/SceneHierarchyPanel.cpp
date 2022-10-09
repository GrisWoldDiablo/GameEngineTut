#include "SceneHierarchyPanel.h"
#include "Hazel/Scene/Components.h"
#include "Hazel/Scene/ScriptableEntity.h"
#include "Hazel/Utils/PlatformUtils.h"
#include "Hazel/Scripting/ScriptEngine.h"
#include "Hazel/Scripting/ScriptClass.h"
#include "Hazel/Scripting/ScriptInstance.h"
#include "Hazel/Audio/AudioEngine.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <filesystem>

#include <Box2D/include/box2d/b2_body.h>

namespace Hazel
{
	template<typename PopupFunction>
	static void DrawYesNoPopup(const std::string& question, PopupFunction popupFunction)
	{
		if (ImGui::BeginPopupContextItem())
		{
			ImGui::Text(question.c_str());
			ImGui::Separator();
			if (ImGui::Button("Yes"))
			{
				popupFunction();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			ImGui::Text(" ");
			ImGui::SameLine();
			if (ImGui::Button("No"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	template<typename T>
	static bool ResetButton(const std::string& label, T& values, T resetValue, ImVec2 size)
	{
		bool hasValueChanged = false;
		if (ImGui::Button(label.c_str(), size))
		{
			ImGui::OpenPopup("reset");
		}

		if (ImGui::BeginPopup("reset"))
		{
			ImGui::Text("Reset %s?", label.c_str());
			ImGui::Separator();
			if (ImGui::Button("Yes"))
			{
				values = resetValue;
				hasValueChanged = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			ImGui::Text(" ");
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		return hasValueChanged;
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(Entity entity, const std::string& name, UIFunction uiFunction)
	{
		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			constexpr auto treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding;

			auto contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4,4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool isTreeOpened = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();

			ImGui::PushID(name.c_str());
			bool shouldRemoveComponent = false;
			if (typeid(T) != typeid(TransformComponent))
			{
				ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
				if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
				{
					ImGui::OpenPopup("ComponentSettings");
				}

				if (ImGui::BeginPopup("ComponentSettings"))
				{
					if (ImGui::MenuItem("Remove Component"))
					{
						shouldRemoveComponent = true;
					}

					ImGui::EndPopup();
				}
			}
			ImGui::PopID();

			if (isTreeOpened)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (shouldRemoveComponent)
			{
				entity.RemoveComponent<T>();
			}
		}
	}

	template<typename T>
	static bool DrawFloatField(const char* label, T& value, T resetValue, const Color& color, int precision = 2)
	{
		bool hasValueChanged = false;
		auto boldFont = ImGui::GetIO().Fonts->Fonts[0];
		float lineHeight = boldFont->FontSize + GImGui->Style.FramePadding.y * 2.0f;

		auto buttonSize = ImVec2{ lineHeight + 3.0f, lineHeight };

		auto buttonColor = ImVec4(color.r, color.g, color.b, color.a);
		auto buttonHoveredColor = buttonColor;
		buttonHoveredColor.x *= 1.125f;
		buttonHoveredColor.y *= 2.0f;
		buttonHoveredColor.z *= 1.333f;

		ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, buttonHoveredColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, buttonColor);
		ImGui::PushFont(boldFont);
		if (ImGui::Button(label, buttonSize))
		{
			value = resetValue;
			hasValueChanged = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		auto floatLabel = std::string("##") + label;
		std::stringstream ss;
		ss << "%." << precision << "f";
		return hasValueChanged | ImGui::DragFloat(floatLabel.c_str(), &value, 0.1f, 0.0f, 0.0f, ss.str().c_str());
	}

	template<typename T>
	static bool DrawVecControls(const std::string& label, T& values, T resetValue = T(0.0f), float columnWidth = 100.0f)
	{
		int valuesSize = sizeof(values) / sizeof(values[0]);
		bool hasValueChanged = false;
		ImGui::PushID(label.c_str());

		ImGui::Columns(2, label.c_str(), false);
		ImGui::SetColumnWidth(0, columnWidth);

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

#pragma region ResetButton
		hasValueChanged = ResetButton(label, values, resetValue, ImVec2{ columnWidth, lineHeight });
#pragma endregion

		ImGui::NextColumn();

#pragma region FloatField
		const char* vecLabels[4] = { "X", "Y", "Z", "W" };
		const Color vecColors[4] =
		{
			{ 0.8f, 0.1f, 0.15f, 1.0f }, // Red
			{ 0.2f, 0.7f, 0.3f, 1.0f }, // Green
			{ 0.1f ,0.25f, 0.8f, 1.0f }, // Blue
			{ 0.666f ,0.745f, 0.098f, 1.0f }, // Yellow
		};

		ImGui::PushMultiItemsWidths(valuesSize, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0.0f, 0.0f });

		for (int i = 0; i < valuesSize; i++)
		{
			ImGui::SameLine();
			if (DrawFloatField(vecLabels[i], values[i], resetValue[i], vecColors[i]))
			{
				hasValueChanged = true;
			}
			ImGui::PopItemWidth();
		}

		ImGui::PopStyleVar();

#pragma endregion

		ImGui::Columns(1);
		ImGui::PopID();

		return hasValueChanged;
	}

	void SceneHierarchyPanel::SetScene(const Ref<Scene>& scene)
	{
		SetSelectedEntity(Entity());
		_scene = scene;
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy", nullptr, ImGuiWindowFlags_MenuBar);

		DrawSceneName();

		_scene->_registry.each([&](auto entityID)
		{
			Entity entity{ entityID, _scene.get() };
			DrawEntityNode(entity);
		});

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			SetSelectedEntity(Entity());
		}

		// Right-Click on blank space.
		if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create New Entity"))
			{
				_scene->CreateEntity();
			}
			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin("Properties");
		if (_selectedEntity)
		{
			DrawComponents(_selectedEntity);
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::EditRuntimeRigidbody(Entity entity, bool shouldClearVelocity)
	{
		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			auto& component = entity.GetComponent<Rigidbody2DComponent>();
			if (auto* body = static_cast<b2Body*>(component.RuntimeBody))
			{
				// TODO recalculate velocity instead of zeroing it.
				if (shouldClearVelocity)
				{
					body->SetLinearVelocity(b2Vec2_zero);
					body->SetAngularVelocity(0.0f);
				}

				if (Rigidbody2DComponent::Box2DBodyToType(body->GetType()) != component.Type)
				{
					body->SetType(static_cast<b2BodyType>(Rigidbody2DComponent::TypeToBox2DBody(component.Type)));
				}

				body->SetFixedRotation(component.IsFixedRotation);
				auto position = entity.Transform().Position;
				body->SetTransform(b2Vec2(position.x, position.y), entity.Transform().Rotation.z);
			}
		}
	}

	void SceneHierarchyPanel::DrawSceneName()
	{
		if (ImGui::BeginMenuBar())
		{
			char buffer[256] = {};
			strcpy_s(buffer, sizeof(buffer), _scene->_name.c_str());
			if (ImGui::InputText(" : Scene", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				const auto newName = std::string(buffer);
				if (!newName.empty())
				{
					_scene->SetName(newName);
				}
			}
			ImGui::Checkbox("Debug", &_isDebug);
			ImGui::EndMenuBar();
		}
		ImGui::Separator();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		ImGuiTreeNodeFlags flags = ((_selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

		std::string displayName = entity.Name();
		if (_isDebug)
		{
			std::stringstream ss;
			ss << displayName << "<" << entity.GetUUID() << ">";
			displayName = ss.str();
		}

		bool expanded = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, displayName.c_str());

		if (ImGui::IsItemClicked())
		{
			SetSelectedEntity(entity);
		}

		bool shouldDeleteEntity = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Create New Entity"))
			{
				_scene->CreateEntity();
			}
			ImGui::Separator();

			if (ImGui::MenuItem(fmt::format("Delete Entity : [{0}]", entity.Name()).c_str()))
			{
				shouldDeleteEntity = true;
			}
			ImGui::EndPopup();
		}

		if (expanded)
		{
			ImGui::TreePop();
		}

		if (shouldDeleteEntity)
		{
			_scene->DestroyEntity(entity);
			if (_selectedEntity == entity)
			{
				SetSelectedEntity(Entity());
			}
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (entity)
		{
			char buffer[256] = {};
			strcpy_s(buffer, sizeof(buffer), entity.Name().c_str());
			if (ImGui::InputText("##:Name", buffer, sizeof(buffer)))
			{
				entity.Name() = std::string(buffer);
			}
			ImGui::SameLine();
			ImGui::PushItemWidth(-1);
			if (ImGui::Button("Add Component"))
			{
				ImGui::OpenPopup("AddComponent");
			}

			if (ImGui::BeginPopup("AddComponent"))
			{
				AddComponentMenu<ScriptComponent>();
				AddComponentMenu<CameraComponent>();
				AddComponentMenu<SpriteRendererComponent>();
				AddComponentMenu<CircleRendererComponent>();
				AddComponentMenu<Rigidbody2DComponent>();
				AddComponentMenu<BoxCollider2DComponent>();
				AddComponentMenu<CircleCollider2DComponent>();
				AddComponentMenu<AudioSourceComponent>();
				AddComponentMenu<AudioListenerComponent>();
				ImGui::EndPopup();
			}
			ImGui::PopItemWidth();

			const char* tags[] = { "Default" }; // TODO Keep tags list somewhere else
			ImGui::Combo("##Tag", &entity.Tag(), tags, static_cast<int>(std::size(tags)));

			ImGui::PushItemWidth(-1);
			ImGui::SameLine();
			const char* layers[] = { "Default" }; // TODO Keep layers list somewhere else
			ImGui::Combo("##Layer", &entity.Layer(), layers, static_cast<int>(std::size(layers)));
			ImGui::PopItemWidth();
		}

#pragma region TransformComponent
		DrawComponent<TransformComponent>(entity, "Transform", [&](TransformComponent& component)
		{
			DrawVecControls("Position", component.Position);

			glm::vec3 rotation = glm::degrees(component.Rotation);
			DrawVecControls("Rotation", rotation);
			component.Rotation = glm::radians(rotation);

			DrawVecControls("Scale", component.Scale, glm::vec3(1.0f));
		});
#pragma endregion

#pragma region ScriptComponent
		DrawComponent<ScriptComponent>(entity, "Script", [&](ScriptComponent& component)
		{
			bool scriptClassExists = ScriptEngine::EntityClassExist(component.ClassName);

			if (!scriptClassExists)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(Color::Red.r, Color::Red.g, Color::Red.b, Color::Red.a));
			}

			const auto* className = component.ClassName.empty() ? "No Script" : component.ClassName.c_str();
			if (ImGui::Button(className))
			{
				if (!_scene->IsRunning())
				{
					ImGui::OpenPopup(className);
				}
			}

			if (!scriptClassExists)
			{
				ImGui::PopStyleColor();
			}

			if (ImGui::BeginPopup(className))
			{
				if (!component.ClassName.empty())
				{
					if (ImGui::Selectable(fmt::format("{0} [X]", component.ClassName).c_str()))
					{
						component.ClassName.clear();
						scriptClassExists = false;
					}
					ImGui::Separator();
				}

				if (ImGui::BeginListBox("##Classes Name"))
				{
					for (const auto& scriptClass : ScriptEngine::GetEntityClasses())
					{
						if (component.ClassName == scriptClass.first)
						{
							continue;
						}

						const auto* itemName = scriptClass.first.c_str();
						if (ImGui::Selectable(itemName))
						{
							component.ClassName = itemName;
							ImGui::CloseCurrentPopup();
						}
					}

					ImGui::EndListBox();
				}

				ImGui::EndPopup();
			}

			if (!scriptClassExists)
			{
				return;
			}

			// If scene Running
			if (_scene->IsRunning())
			{
				if (auto scriptInstance = ScriptEngine::GetEntityScriptInstance(entity.GetUUID()))
				{
					const auto& fields = scriptInstance->GetScriptClass()->GetFields();
					for (const auto& [name, field] : fields)
					{
						switch (field.Type)
						{
						case ScriptFieldType::Float:
						{
							auto data = scriptInstance->GetFieldValue<float>(name);
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_Float, &data, 0.1f))
							{
								scriptInstance->SetFieldValue(name, data);
							}
							break;
						}
						case ScriptFieldType::Double:
						{
							auto data = scriptInstance->GetFieldValue<double>(name);
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_Double, &data, 0.1f))
							{
								scriptInstance->SetFieldValue(name, data);
							}
							break;
						}
						case ScriptFieldType::Char:
						{
							auto data = scriptInstance->GetFieldValue<uint16_t>(name);
							char buffer[2] = { static_cast<char>(data) };
							if (ImGui::InputText(name.c_str(), buffer, sizeof(buffer)))
							{
								data = *buffer;
								scriptInstance->SetFieldValue(name, data);
							}
							break;
						}
						case ScriptFieldType::Bool:
						{
							auto data = scriptInstance->GetFieldValue<bool>(name);
							if (ImGui::Checkbox(name.c_str(), &data))
							{
								scriptInstance->SetFieldValue(name, data);
							}
							break;
						}
						case ScriptFieldType::SByte:
						{
							auto data = scriptInstance->GetFieldValue<int8_t>(name);
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_S8, &data))
							{
								scriptInstance->SetFieldValue(name, data);
							}
							break;
						}
						case ScriptFieldType::Short:
						{
							auto data = scriptInstance->GetFieldValue<int16_t>(name);
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_S16, &data))
							{
								scriptInstance->SetFieldValue(name, data);
							}
							break;
						}
						case ScriptFieldType::Int:
						{
							auto data = scriptInstance->GetFieldValue<int32_t>(name);
							if (ImGui::DragInt(name.c_str(), &data, 0.1f))
							{
								scriptInstance->SetFieldValue(name, data);
							}
							break;
						}
						case ScriptFieldType::Long:
						{
							auto data = scriptInstance->GetFieldValue<int64_t>(name);
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_S64, &data))
							{
								scriptInstance->SetFieldValue(name, data);
							}
							break;
						}
						case ScriptFieldType::Byte:
						{
							auto data = scriptInstance->GetFieldValue<uint8_t>(name);
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U8, &data))
							{
								scriptInstance->SetFieldValue(name, data);
							}
							break;
						}
						case ScriptFieldType::UShort:
						{
							auto data = scriptInstance->GetFieldValue<uint16_t>(name);
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U16, &data))
							{
								scriptInstance->SetFieldValue(name, data);
							}
							break;
						}
						case ScriptFieldType::UInt:
						{
							auto data = scriptInstance->GetFieldValue<uint32_t>(name);
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U32, &data))
							{
								scriptInstance->SetFieldValue(name, data);
							}
							break;
						}
						case ScriptFieldType::ULong:
						{
							auto data = scriptInstance->GetFieldValue<uint64_t>(name);
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U64, &data))
							{
								scriptInstance->SetFieldValue(name, data);
							}
							break;
						}
						case ScriptFieldType::Vector2:
						{
							const auto defaultValue = field.GetDefaultValue<glm::vec2>();
							auto data = scriptInstance->GetFieldValue<glm::vec2>(name);
							if (DrawVecControls(name, data, defaultValue))
							{
								scriptInstance->SetFieldValue(name, data);
							}
							break;
						}
						case ScriptFieldType::Vector3:
						{
							const auto defaultValue = field.GetDefaultValue<glm::vec3>();
							auto data = scriptInstance->GetFieldValue<glm::vec3>(name);
							if (DrawVecControls(name, data, defaultValue))
							{
								scriptInstance->SetFieldValue(name, data);
							}
							break;
						}
						case ScriptFieldType::Vector4:
						{
							const auto defaultValue = field.GetDefaultValue<glm::vec4>();
							auto data = scriptInstance->GetFieldValue<glm::vec4>(name);
							if (DrawVecControls(name, data, defaultValue))
							{
								scriptInstance->SetFieldValue(name, data);
							}
							break;
						}
						case ScriptFieldType::Color:
						{
							auto data = scriptInstance->GetFieldValue<Color>(name);
							if (ImGui::ColorEdit4(name.c_str(), data.GetValuePtr()))
							{
								scriptInstance->SetFieldValue(name, data);
							}
							break;
						}
						case ScriptFieldType::Entity:
						{
							break;
						}
						case ScriptFieldType::String:
						{
							char buffer[256] = {};
							auto data = scriptInstance->GetFieldStringValue(name);
							strcpy_s(buffer, sizeof(buffer), data.c_str());
							if (ImGui::InputText(name.c_str(), buffer, sizeof(buffer)))
							{
								scriptInstance->SetFieldStringValue(name, buffer);
							}
							break;
						}
						}
					}
				}
			}
			else
			{
				DrawYesNoPopup("Reset Class Fields?", [&]
				{
					ScriptEngine::EraseFromScriptFieldMap(entity);
				});

				auto entityClass = ScriptEngine::GetEntityClass(component.ClassName);
				const auto& fields = entityClass->GetFields();

				auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);

				for (const auto& [name, field] : fields)
				{
					if (entityFields.find(name) != entityFields.end())
					{
						auto& scriptField = entityFields[name];

						switch (field.Type)
						{
						case ScriptFieldType::Float:
						{
							auto data = scriptField.GetValue<float>();
							if (ImGui::DragFloat(name.c_str(), &data, 0.1f))
							{
								scriptField.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Double:
						{
							auto data = scriptField.GetValue<double>();
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_Double, &data, 0.1f))
							{
								scriptField.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Char:
						{
							auto data = scriptField.GetValue<uint16_t>();
							char buffer[2] = { static_cast<char>(data) };
							if (ImGui::InputText(name.c_str(), buffer, sizeof(buffer)))
							{
								data = *buffer;
								scriptField.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Bool:
						{
							auto data = scriptField.GetValue<bool>();
							if (ImGui::Checkbox(name.c_str(), &data))
							{
								scriptField.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::SByte:
						{
							auto data = scriptField.GetValue<int8_t>();
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_S8, &data))
							{
								scriptField.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Short:
						{
							auto data = scriptField.GetValue<int16_t>();
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_S16, &data))
							{
								scriptField.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Int:
						{
							auto data = scriptField.GetValue<int32_t>();
							if (ImGui::DragInt(name.c_str(), &data, 0.1f))
							{
								scriptField.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Long:
						{
							auto data = scriptField.GetValue<int64_t>();
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_S64, &data))
							{
								scriptField.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Byte:
						{
							auto data = scriptField.GetValue<uint8_t>();
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U8, &data))
							{
								scriptField.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::UShort:
						{
							auto data = scriptField.GetValue<uint16_t>();
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U16, &data))
							{
								scriptField.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::UInt:
						{
							auto data = scriptField.GetValue<uint32_t>();
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U32, &data))
							{
								scriptField.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::ULong:
						{
							auto data = scriptField.GetValue<uint64_t>();
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U64, &data))
							{
								scriptField.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Vector2:
						{
							const auto defaultValue = scriptField.Field.GetDefaultValue<glm::vec2>();
							auto data = scriptField.GetValue<glm::vec2>();
							if (DrawVecControls(name, data, defaultValue))
							{
								scriptField.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Vector3:
						{
							const auto defaultValue = scriptField.Field.GetDefaultValue<glm::vec3>();
							auto data = scriptField.GetValue<glm::vec3>();
							if (DrawVecControls(name, data, defaultValue))
							{
								scriptField.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Vector4:
						{
							const auto defaultValue = scriptField.Field.GetDefaultValue<glm::vec4>();
							auto data = scriptField.GetValue<glm::vec4>();
							if (DrawVecControls(name, data, defaultValue))
							{
								scriptField.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Color:
						{
							auto data = scriptField.GetValue<Color>();
							if (ImGui::ColorEdit4(name.c_str(), data.GetValuePtr()))
							{
								scriptField.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Entity:
						{
							break;
						}
						case ScriptFieldType::String:
						{
							char buffer[256] = {};
							auto data = scriptField.GetStringValue();
							strcpy_s(buffer, sizeof(buffer), data.c_str());
							if (ImGui::InputText(name.c_str(), buffer, sizeof(buffer)))
							{
								scriptField.SetStringValue(buffer);
							}
							break;
						}
						}
					}
					else
					{
						switch (field.Type)
						{
						case ScriptFieldType::Float:
						{
							auto data = field.GetDefaultValue<float>();
							if (ImGui::DragFloat(name.c_str(), &data, 0.1f))
							{
								auto& scriptFieldInstance = entityFields[name];
								scriptFieldInstance.Field = field;
								scriptFieldInstance.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Double:
						{
							auto data = field.GetDefaultValue<double>();
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_Double, &data, 0.1f))
							{
								auto& scriptFieldInstance = entityFields[name];
								scriptFieldInstance.Field = field;
								scriptFieldInstance.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Char:
						{
							auto data = field.GetDefaultValue<uint16_t>();
							char buffer[2] = { static_cast<char>(data) };
							if (ImGui::InputText(name.c_str(), buffer, sizeof(buffer)))
							{
								auto& scriptFieldInstance = entityFields[name];
								scriptFieldInstance.Field = field;
								data = *buffer;
								scriptFieldInstance.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Bool:
						{
							auto data = field.GetDefaultValue<bool>();
							if (ImGui::Checkbox(name.c_str(), &data))
							{
								auto& scriptFieldInstance = entityFields[name];
								scriptFieldInstance.Field = field;
								scriptFieldInstance.SetValue(data);
							}
							break;
						}

						case ScriptFieldType::SByte:
						{
							auto data = field.GetDefaultValue<int8_t>();
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_S8, &data))
							{
								auto& scriptFieldInstance = entityFields[name];
								scriptFieldInstance.Field = field;
								scriptFieldInstance.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Short:
						{
							auto data = field.GetDefaultValue<int16_t>();
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_S16, &data))
							{
								auto& scriptFieldInstance = entityFields[name];
								scriptFieldInstance.Field = field;
								scriptFieldInstance.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Int:
						{
							auto data = field.GetDefaultValue<int32_t>();
							if (ImGui::DragInt(name.c_str(), &data, 0.1f))
							{
								auto& scriptFieldInstance = entityFields[name];
								scriptFieldInstance.Field = field;
								scriptFieldInstance.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Long:
						{
							auto data = field.GetDefaultValue<int64_t>();
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_S64, &data))
							{
								auto& scriptFieldInstance = entityFields[name];
								scriptFieldInstance.Field = field;
								scriptFieldInstance.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Byte:
						{
							auto data = field.GetDefaultValue<uint8_t>();
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U8, &data))
							{
								auto& scriptFieldInstance = entityFields[name];
								scriptFieldInstance.Field = field;
								scriptFieldInstance.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::UShort:
						{
							auto data = field.GetDefaultValue<uint16_t>();
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U16, &data))
							{
								auto& scriptFieldInstance = entityFields[name];
								scriptFieldInstance.Field = field;
								scriptFieldInstance.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::UInt:
						{
							auto data = field.GetDefaultValue<uint32_t>();
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U32, &data))
							{
								auto& scriptFieldInstance = entityFields[name];
								scriptFieldInstance.Field = field;
								scriptFieldInstance.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::ULong:
						{
							auto data = field.GetDefaultValue<uint64_t>();
							if (ImGui::DragScalar(name.c_str(), ImGuiDataType_U64, &data))
							{
								auto& scriptFieldInstance = entityFields[name];
								scriptFieldInstance.Field = field;
								scriptFieldInstance.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Vector2:
						{
							auto data = field.GetDefaultValue<glm::vec2>();
							if (DrawVecControls(name, data, data))
							{
								auto& scriptFieldInstance = entityFields[name];
								scriptFieldInstance.Field = field;
								scriptFieldInstance.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Vector3:
						{
							auto data = field.GetDefaultValue<glm::vec3>();
							if (DrawVecControls(name, data, data))
							{
								auto& scriptFieldInstance = entityFields[name];
								scriptFieldInstance.Field = field;
								scriptFieldInstance.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Vector4:
						{
							auto data = field.GetDefaultValue<glm::vec4>();
							if (DrawVecControls(name, data, data))
							{
								auto& scriptFieldInstance = entityFields[name];
								scriptFieldInstance.Field = field;
								scriptFieldInstance.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Color:
						{
							auto data = field.GetDefaultValue<Color>();
							if (ImGui::ColorEdit4(name.c_str(), data.GetValuePtr()))
							{
								auto& scriptFieldInstance = entityFields[name];
								scriptFieldInstance.Field = field;
								scriptFieldInstance.SetValue(data);
							}
							break;
						}
						case ScriptFieldType::Entity:
						{
							break;
						}
						case ScriptFieldType::String:
						{
							char buffer[256] = {};
							auto data = field.GetDefaultStringValue();
							strcpy_s(buffer, sizeof(buffer), data.c_str());
							if (ImGui::InputText(name.c_str(), buffer, sizeof(buffer)))
							{
								auto& scriptFieldInstance = entityFields[name];
								scriptFieldInstance.Field = field;
								scriptFieldInstance.SetStringValue(buffer);
							}
							break;
						}
						}
					}
				}
			}
		});
#pragma endregion

#pragma region CameraComponent
		DrawComponent<CameraComponent>(entity, "Camera", [&](CameraComponent& component)
		{
			auto& camera = component.Camera;
			ImGui::Checkbox("Primary", &component.IsPrimary);
			const char* projectionTypeStrings[] = { "Perspective","Orthographic" };
			const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
			if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
			{
				for (int i = 0; i < 2; i++)
				{
					bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
					if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
					{
						currentProjectionTypeString = projectionTypeStrings[i];
						camera.SetProjectionType((SceneCamera::ProjectionType)i);
					}

					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			switch (camera.GetProjectionType())
			{
			case SceneCamera::ProjectionType::Perspective:
			{
				float perspectiveFOV = glm::degrees(camera.GetPerspectiveVerticalFOV());
				if (ImGui::DragFloat("FOV", &perspectiveFOV))
				{
					camera.SetPerspectiveVerticalFOV(glm::radians(perspectiveFOV));
				}

				float perspectiveNearClip = camera.GetPerspectiveNearClip();
				if (ImGui::DragFloat("NearClip", &perspectiveNearClip))
				{
					camera.SetPerspectiveNearClip(perspectiveNearClip);
				}

				float perspectiveFarClip = camera.GetPerspectiveFarClip();
				if (ImGui::DragFloat("FarClip", &perspectiveFarClip))
				{
					camera.SetPerspectiveFarClip(perspectiveFarClip);
				}

			}	break;
			case SceneCamera::ProjectionType::Orthographic:
			{
				float orthographicSize = camera.GetOrthographicSize();
				if (ImGui::DragFloat("Size", &orthographicSize, 0.1f))
				{
					camera.SetOrthographicSize(orthographicSize);
				}

				float orthographicNearClip = camera.GetOrthographicNearClip();
				if (ImGui::DragFloat("NearClip", &orthographicNearClip))
				{
					camera.SetOrthographicNearClip(orthographicNearClip);
				}

				float orthographicFarClip = camera.GetOrthographicFarClip();
				if (ImGui::DragFloat("FarClip", &orthographicFarClip))
				{
					camera.SetOrthographicFarClip(orthographicFarClip);
				}
			}	break;
			}

			if (ImGui::Checkbox("Fixed Aspect Ratio", &component.IsFixedAspectRatio) && !component.IsFixedAspectRatio)
			{
				camera.SetViewportSize(_scene->_viewportWidth, _scene->_viewportHeight);
			}
		});
#pragma endregion

#pragma region SpriteRendererComponent
		DrawComponent<SpriteRendererComponent>(entity, "Sprite Renderer", [](SpriteRendererComponent& component)
		{
			ImGui::Text("Sprite");
			ImGui::SameLine();
			bool isSpritePressed;
			if (component.Texture != nullptr)
			{
				isSpritePressed = ImGui::ImageButton((ImTextureID)(intptr_t)component.Texture->GetRendererID(), ImVec2(50.0f, 50.0f), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 3);
			}
			else
			{
				auto& color = component.Color;
				isSpritePressed = ImGui::ColorButton("None", ImVec4(color.r, color.g, color.b, color.a), 0, ImVec2(56.0f, 56.0f));
			}

			std::filesystem::path textureFilePath;

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					std::filesystem::path fileSystemPath = (const wchar_t*)payload->Data;

					if (fileSystemPath.extension() == ".png")
					{
						textureFilePath = fileSystemPath;
					}
				}
				ImGui::EndDragDropTarget();
			}

			if (isSpritePressed)
			{
				textureFilePath = FileDialogs::OpenFile("PNG (*.png)\0*.png\0");
				textureFilePath = std::filesystem::relative(textureFilePath);
			}

			if (component.Texture != nullptr)
			{
				DrawYesNoPopup("Clear Texture?", [&]
				{
					component.Texture = nullptr;
				});
			}

			if (!textureFilePath.empty())
			{
				uint32_t magFilter = 0;
				if (component.Texture)
				{
					magFilter = component.Texture->GetMagFilter();
				}

				component.Texture = Texture2D::Create(textureFilePath);
				if (magFilter && component.Texture != nullptr)
				{
					component.Texture->SetMagFilter(magFilter);
				}
			}

			if (component.Texture != nullptr)
			{
				if (ImGui::Button("Mag Filter Toggle"))
				{
					// TODO serialize Mag Filter
					auto currentMagFilter = component.Texture->GetMagFilter();
					component.Texture->ToggleMagFilter(currentMagFilter);
				}
				ImGui::SameLine();
				ImGui::Text("%s", component.Texture->IsMagFilterLinear() ? "Linear" : "Nearest");
				DrawVecControls("Tiling", component.Tiling, glm::vec2(1.0f));
			}

			auto& color = component.Color;
			auto& newColor = color;
			if (ImGui::ColorEdit4("Color", newColor.GetValuePtr()))
			{
				color = newColor;
			}
		});
#pragma endregion

#pragma region CircleRendererComponent
		DrawComponent<CircleRendererComponent>(entity, "Circle Renderer", [](CircleRendererComponent& component)
		{
			ImGui::ColorEdit4("Color", component.Color.GetValuePtr());
			ImGui::DragFloat("Thickness", &component.Thickness, 0.025f, 0.0f, 1.0f);
			ImGui::DragFloat("Fade", &component.Fade, 0.00025f, 0.0f);
		});
#pragma endregion

#pragma region Rigidbody2DComponent
		DrawComponent<Rigidbody2DComponent>(entity, "Rigidbody 2D", [&](Rigidbody2DComponent& component)
		{
			const char* bodyType[] = { "Static","Dynamic","Kinematic" };
			const char* currentBodyType = bodyType[(int)component.Type];
			if (ImGui::BeginCombo("Type", currentBodyType))
			{
				for (int i = 0; i < 3; i++)
				{
					bool isSelected = currentBodyType == bodyType[i];
					if (ImGui::Selectable(bodyType[i], isSelected))
					{
						currentBodyType = bodyType[i];
						component.Type = (Rigidbody2DComponent::BodyType)i;
					}

					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			ImGui::Checkbox("Fixed Rotation", &component.IsFixedRotation);

			EditRuntimeRigidbody(entity);
		});
#pragma endregion

#pragma region BoxCollider2DComponent
		DrawComponent<BoxCollider2DComponent>(entity, "Box Collider 2D", [](BoxCollider2DComponent& component)
		{
			DrawVecControls("Offset", component.Offset);
			DrawVecControls("Size", component.Size, glm::vec2(0.5f));
			ImGui::DragFloat("Rotation", &component.Rotation, 0.01f, 0.0f, 360.0f);
			ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
		});
#pragma endregion

#pragma region CircleCollider2DComponent
		DrawComponent<CircleCollider2DComponent>(entity, "Circle Collider 2D", [](CircleCollider2DComponent& component)
		{
			DrawVecControls("Offset", component.Offset);
			ImGui::DragFloat("Radius", &component.Radius, 0.01f, 0.0f, 0.0f);
			ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
		});
#pragma endregion

#pragma region NativeScriptComponent
		DrawComponent<NativeScriptComponent>(entity, "Native Script", [](const NativeScriptComponent& component)
		{
			ImGui::Checkbox("Active", &component.Instance->IsEnable);
			std::string classFilePath = component.Instance->GetClassFilePath();
			auto lastSlash = classFilePath.find("\\src\\");
			auto count = classFilePath.size() - lastSlash;
			auto fileName = classFilePath.substr(lastSlash, count);
			ImGui::SameLine();
			ImGui::TextDisabled(fileName.c_str());
			std::string result;
			std::ifstream inputFileStream(classFilePath, std::ios::in);
			if (!inputFileStream.bad())
			{
				inputFileStream.seekg(0, std::ios::end);
				result.resize(inputFileStream.tellg());
				inputFileStream.seekg(0, std::ios::beg);
				inputFileStream.read(&result[0], result.size());
				inputFileStream.close();
				ImGui::BeginChild("Source", ImVec2(ImGui::GetWindowWidth() - 50, 500), true, ImGuiWindowFlags_HorizontalScrollbar);
				ImGui::Text(result.c_str());
				ImGui::EndChild();
			}
		});
#pragma endregion

#pragma region  AudioSourceComponent
		DrawComponent<AudioSourceComponent>(entity, "Audio Source", [&](AudioSourceComponent& component)
		{
			bool isButtonPressed;
			auto& audioSource = component.AudioSource;
			if (audioSource)
			{
				isButtonPressed = ImGui::Button(audioSource->GetPath().filename().string().c_str());
			}
			else
			{
				isButtonPressed = ImGui::Button("Select Clip");
			}

			if (audioSource != nullptr)
			{
				DrawYesNoPopup("Clear Audio Clip?", [&]
				{
					AudioEngine::ReleaseAudioSource(audioSource);
					audioSource.reset();
				});
			}

			std::filesystem::path audioClipFilePath;

			if (isButtonPressed)
			{
				audioClipFilePath = FileDialogs::OpenFile("Audio Clip (*.ogg,*.mp3)\0*.ogg;*.mp3\0");
				audioClipFilePath = std::filesystem::relative(audioClipFilePath);
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					std::filesystem::path fileSystemPath = (const wchar_t*)payload->Data;
					const auto extension = fileSystemPath.extension();
					if (extension == ".ogg" || extension == ".mp3")
					{
						audioClipFilePath = fileSystemPath;
					}
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::SameLine();
			ImGui::Text("Audio Clip");

			if (!audioClipFilePath.empty())
			{
				if (audioSource)
				{
					if (audioSource->GetPath() != audioClipFilePath)
					{
						AudioEngine::ReleaseAudioSource(audioSource);
						audioSource = AudioSource::Create(audioClipFilePath);
					}
				}
				else
				{
					audioSource = AudioSource::Create(audioClipFilePath);
				}
			}

			if (audioSource == nullptr)
			{
				return;
			}

			_previousAudioSource = audioSource;

			auto floatValueField = [&](const char* label, float min, float max, auto getValue, auto setValue)
			{
				float value = getValue();
				if (ImGui::DragFloat(label, &value, 0.01f, min, max, "%.3f"))
				{
					setValue(value);
				}
			};

			floatValueField("Gain (Volume)", 0.0f, 1.0f,
				[&]() { return audioSource->GetGain(); },
				[&](float valueToSet) { audioSource->SetGain(valueToSet); });
			floatValueField("Pitch (Speed)", 0.0f, 0.0f,
				[&]() { return audioSource->GetPitch(); },
				[&](float valueToSet) { audioSource->SetPitch(valueToSet); });

			auto boolValueField = [&](const char* label, auto getValue, auto setValue)
			{
				bool value = getValue();
				if (ImGui::Checkbox(label, &value))
				{
					setValue(value);
				}
			};

			boolValueField("Loop",
				[&]() { return audioSource->GetLoop(); },
				[&](auto valueToSet) { audioSource->SetLoop(valueToSet); });
			boolValueField("3D",
				[&]() { return audioSource->Get3D(); },
				[&](auto valueToSet) { audioSource->Set3D(valueToSet); });

			if (audioSource->Get3D())
			{
				audioSource->SetPosition(entity.Transform().Position);
			}

			ImGui::Text("Lenght: %.3f sec", audioSource->GetLength());

			auto state = audioSource->GetState();
			switch (state)
			{
			case AudioSourceState::INITIAL:
			case AudioSourceState::STOPPED:
			{
				if (ImGui::Button("Play"))
				{
					audioSource->Play();
				}
				break;
			}
			case AudioSourceState::PAUSED:
			{
				if (ImGui::Button("Unpause"))
				{
					audioSource->Play();
				}
				break;
			}
			case AudioSourceState::PLAYING:
			{
				if (ImGui::Button("Pause"))
				{
					audioSource->Pause();
				}
				break;
			}
			default:
				break;
			}

			ImGui::SameLine();
			if (ImGui::Button("Stop"))
			{
				audioSource->Stop();
			}

			float offset = audioSource->GetOffset();
			float lenght = audioSource->GetLength();
			int min = static_cast<int>(offset / 60);
			float secs = (offset - (min * 60));
			int sec = static_cast<int>(secs);
			int ms = static_cast<int>((secs - sec) * 1000);

			auto trackValue = fmt::format("{}m:{:02d}s:{:02d}ms", min, sec, ms);
			if (ImGui::SliderFloat("Track", &offset, 0.0f, lenght, trackValue.c_str(), ImGuiSliderFlags_NoInput))
			{
				if (state == AudioSourceState::PAUSED || state == AudioSourceState::PLAYING)
				{
					audioSource->SetOffset(offset);
				}
			}

			ImGui::Checkbox("Visible In Game##AudioSource", &component.IsVisibleInGame);
		});
#pragma endregion

#pragma region  AudioListenerComponent
		DrawComponent<AudioListenerComponent>(entity, "Audio Listener", [&](AudioListenerComponent& component)
		{
			ImGui::TextWrapped("AudioListener are only required if you have 3D Audio Sources.");
			ImGui::Checkbox("Visible In Game##AudioListener", &component.IsVisibleInGame);

			for (auto listener : _scene->GetAllEntitiesWith<AudioListenerComponent>())
			{
				if (listener != entity)
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(Color::Red.r, Color::Red.g, Color::Red.b, Color::Red.a));
					ImGui::TextWrapped("ERROR!");
					ImGui::TextWrapped("There is more than one listeners in the scene!");
					ImGui::TextWrapped("Please remove one!");
					ImGui::PopStyleColor();
					return;
				}
			}

			AudioEngine::SetListenerPosition(entity.Transform().Position);
		});
#pragma endregion

	}

	template<typename T, typename Function>
	static void CleanUpComponent(Entity entity, Function function)
	{
		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			function(component);
		}
	}

	void SceneHierarchyPanel::CleanUpComponents(Entity entity)
	{
		if (!entity)
		{
			return;
		}

		CleanUpComponent<AudioSourceComponent>(entity, [&](AudioSourceComponent& component)
		{
			if (_shouldKeepPlaying)
			{
				return;
			}

			if (component.AudioSource && !_previousAudioSource.expired())
			{
				if (component.AudioSource == std::shared_ptr(_previousAudioSource))
				{
					component.AudioSource->Stop();
					_previousAudioSource.reset();
				}
			}
		});
	}

	template<typename T>
	void SceneHierarchyPanel::AddComponentMenu()
	{
		auto nameId = std::string(typeid(T).name());
		nameId = nameId.erase(0, nameId.find_last_of(':') + 1);
		nameId = nameId.erase(nameId.find("Component"), nameId.length());

		if (!_selectedEntity.HasComponent<T>())
		{
			if (ImGui::MenuItem(nameId.c_str()))
			{
				_selectedEntity.AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
		else
		{
			ImGui::TextDisabled(nameId.c_str());
		}
	}

	void SceneHierarchyPanel::SetShouldKeepPlaying(bool value)
	{
		if (_shouldKeepPlaying == value)
		{
			return;
		}

		_shouldKeepPlaying = value;
		if (!_shouldKeepPlaying)
		{
			AudioEngine::StopAllAudioSources();
		}
	}
}
