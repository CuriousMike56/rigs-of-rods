/*
    This source file is part of Rigs of Rods (CM fork)
    Copyright 2022 Petr Ohlidal
    Copyright 2025 CuriousMike ft. GitHub Copilot

    For more information, see http://www.rigsofrods.org/

    Rigs of Rods is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 3, as
    published by the Free Software Foundation.

    Rigs of Rods is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rigs of Rods. If not, see <http://www.gnu.org/licenses/>.
*/

/// @file


#include "GUI_FlexbodyDebug.h"

#include "Actor.h"
#include "Application.h"
#include "SimData.h"
#include "Collisions.h"
#include "FlexBody.h"
#include "GameContext.h"
#include "GUIManager.h"
#include "GUIUtils.h"
#include "Language.h"
#include "Terrain.h"
#include "Utils.h"
#include "ApproxMath.h"

using namespace RoR;
using namespace GUI;

void FlexbodyDebug::Draw()
{
    ImGui::SetNextWindowPosCenter(ImGuiCond_FirstUseEver);
    ImGuiWindowFlags win_flags = ImGuiWindowFlags_NoCollapse;
    bool keep_open = true;
    ImGui::Begin(_LC("FlexbodyDebug", "Flexbody/Prop debug"), &keep_open, win_flags);

    ActorPtr actor = App::GetGameContext()->GetPlayerActor();
    if (!actor)
    {
        ImGui::Text("%s", _LC("FlexbodyDebug", "You are on foot."));
        ImGui::End();
        return;
    }

    if (actor->GetGfxActor()->GetFlexbodies().size() == 0
        && actor->GetGfxActor()->getProps().size() == 0)
    {
        ImGui::Text("%s", _LC("FlexbodyDebug", "This vehicle has no flexbodies or props."));
        ImGui::End();
        return;
    }

    if (ImGui::Combo(_LC("FlexbodyDebug", "Select element"), &m_combo_selection, m_combo_items.c_str()))
    {
        this->UpdateVisibility();
        show_locator.resize(0);
        if (m_combo_selection < m_combo_props_start)
        {
            show_locator.resize(actor->GetGfxActor()->GetFlexbodies()[m_combo_selection]->getVertexCount(), false);
        }
        // NEW: Reset UI state and load the selected element's transform
        m_values_initialized = false;
        if (m_combo_props_start == -1 || m_combo_selection < m_combo_props_start)
        {
            FlexBody* flexbody = actor->GetGfxActor()->GetFlexbodies()[m_combo_selection];
            m_edit_offset = flexbody->GetInitialOffset();
            m_edit_rotation = flexbody->GetInitialRotation();
        }
        else
        {
            Prop& prop = actor->GetGfxActor()->getProps()[m_combo_selection - m_combo_props_start];
            m_edit_offset = prop.pp_offset;
            m_raw_angles.x = (prop.pp_rot.x - 0.5f) * 180.0f;
            m_raw_angles.y = prop.pp_rot.y * 180.0f;
            m_raw_angles.z = prop.pp_rot.z * 180.0f;
            m_edit_rotation =
                Ogre::Quaternion(Ogre::Degree(m_raw_angles.x), Ogre::Vector3::UNIT_X) *
                Ogre::Quaternion(Ogre::Degree(m_raw_angles.y), Ogre::Vector3::UNIT_Y) *
                Ogre::Quaternion(Ogre::Degree(m_raw_angles.z), Ogre::Vector3::UNIT_Z);
        }
    }
    if (ImGui::Checkbox("Hide other (note: pauses reflections)", &this->hide_other_elements))
    {
        this->UpdateVisibility();
    }
    ImGui::Separator();

    // Fetch the element (prop or flexbody)
    FlexBody* flexbody = nullptr;
    Prop* prop = nullptr;
    Ogre::MaterialPtr mat; // Assume one submesh (=> subentity); can be NULL if the flexbody is a placeholder, see `FlexBodyPlaceholder_t`
    NodeNum_t node_ref = NODENUM_INVALID, node_x = NODENUM_INVALID, node_y = NODENUM_INVALID;
    std::string mesh_name;
    if (actor->GetGfxActor()->getProps().size() > 0
        && m_combo_selection >= m_combo_props_start)
    {
        prop = &actor->GetGfxActor()->getProps()[m_combo_selection - m_combo_props_start];
        node_ref = prop->pp_node_ref;
        node_x = prop->pp_node_x;
        node_y = prop->pp_node_y;
        if (prop->pp_mesh_obj // Aerial beacons 'L/R/w' have no mesh
            && prop->pp_mesh_obj->getLoadedMesh()) // Props are spawned even if meshes fail to load.
        {
            mat = prop->pp_mesh_obj->getEntity()->getSubEntity(0)->getMaterial();
            mesh_name = prop->pp_mesh_obj->getEntity()->getMesh()->getName();
        }
    }
    else
    {
        flexbody = actor->GetGfxActor()->GetFlexbodies()[m_combo_selection];
        if (flexbody->getPlaceholderType() == FlexBody::PlaceholderType::NOT_A_PLACEHOLDER)
        {
            mat = flexbody->getEntity()->getSubEntity(0)->getMaterial();
        }
        node_ref = flexbody->getRefNode();
        node_x = flexbody->getXNode();
        node_y = flexbody->getYNode();
        mesh_name = flexbody->getOrigMeshName();
    }

    ImGui::Text("Mesh: '%s'", mesh_name.c_str());
    if (mat)
    {
        ImGui::SameLine();
        if (ImGui::Checkbox("Wireframe (per material)", &this->draw_mesh_wireframe))
        {
            // Assume one technique and one pass
            if (mat->getTechniques().size() > 0 && mat->getTechniques()[0]->getPasses().size() > 0)
            {
                Ogre::PolygonMode mode = (this->draw_mesh_wireframe) ? Ogre::PM_WIREFRAME : Ogre::PM_SOLID;
                mat->getTechniques()[0]->getPasses()[0]->setPolygonMode(mode);
            }
        }
    }

    ImGui::Text("Base nodes: Ref=%d, X=%d, Y=%d", (int)node_ref, (int)node_x, (int)node_y);
    ImGui::SameLine();
    ImGui::Checkbox("Show##base", &this->show_base_nodes);

    bool flexbody_locators_visible = false;
    if (flexbody)
    {
        ImGui::Text("Forset nodes: (total %d)", (int)flexbody->getForsetNodes().size());
        ImGui::SameLine();
        ImGui::Checkbox("Show all##forset", &this->show_forset_nodes);

        ImGui::Text("Vertices: (total %d)", (int)flexbody->getVertexCount());
        ImGui::SameLine();
        ImGui::Checkbox("Show all (pick with mouse)##verts", &this->show_vertices);

        if (ImGui::CollapsingHeader("Vertex locators table"))
        {
            this->DrawLocatorsTable(flexbody, /*out:*/flexbody_locators_visible);
        }

        if (ImGui::CollapsingHeader("Vertex locators memory (experimental!)"))
        {
            this->DrawMemoryOrderGraph(flexbody);
        }
    }

    if (ImGui::CollapsingHeader("Mesh info"))
    {
        if (flexbody)
            this->DrawMeshInfo(flexbody);
        else
            this->DrawMeshInfo(prop);
    }

    if (ImGui::CollapsingHeader("Position & rotation")) 
    {
        bool values_changed = false;
        if (flexbody)
        {
            values_changed = this->DrawFlexbodyOffsetRotationEdit(flexbody);
            if (values_changed)
            {
                flexbody->computeFlexbody();
                flexbody->updateFlexbodyVertexBuffers();
            }
            this->DrawOffsetRotationReset(flexbody);
        }
        else if (prop)
        {
            values_changed = this->DrawPropOffsetRotationEdit(prop);
            this->DrawOffsetRotationReset(prop);
        }
        m_offset_rot_changed = m_offset_rot_changed || values_changed;
    }
    else
    {
        m_is_editing = false;
    }

    if (flexbody)
    {
        // Add ref node info
        ImGui::Separator();
        ImGui::TextDisabled("Reference nodes:"); 
        ImGui::Text("Center: %d", flexbody->getRefNode());
        ImGui::Text("X: %d", flexbody->getXNode());
        ImGui::Text("Y: %d", flexbody->getYNode());
    }

    m_is_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
    App::GetGuiManager()->RequestGuiCaptureKeyboard(m_is_hovered);
    ImGui::End();

    if (!keep_open)
    {
        this->SetVisible(false);
    }

    if (this->show_base_nodes || this->show_forset_nodes || this->show_vertices || flexbody_locators_visible)
    {
        this->DrawDebugView(flexbody, prop, node_ref, node_x, node_y);
    }
}

bool FlexbodyDebug::DrawOffsetRotationEdit(Ogre::Vector3& offset, Ogre::Quaternion& rotation)
{
    bool changed = false;
    
    // Position and rotation arrays
    static float pos[3] = {0,0,0}; 
    static float rot[3] = {0,0,0}; 
    bool pos_changed = false;
    bool rot_changed = false;

    // Initialize values when starting or resetting
    if (!m_values_initialized)
    {
        // Position
        pos[0] = offset.x;
        pos[1] = offset.y;
        pos[2] = offset.z;

        if (m_combo_selection >= m_combo_props_start)
        {
            // For props: use the stored raw angles (in degrees)
            rot[0] = m_raw_angles.x; // Pitch
            rot[1] = m_raw_angles.y; // Yaw
            rot[2] = m_raw_angles.z; // Roll
        }
        else
        {
            // Flexbodies - extract Euler angles from the quaternion
            Ogre::Matrix3 rot_matrix;
            rotation.ToRotationMatrix(rot_matrix);
            Ogre::Radian pitch, yaw, roll;
            rot_matrix.ToEulerAnglesXYZ(pitch, yaw, roll);
            rot[0] = pitch.valueDegrees();
            rot[1] = yaw.valueDegrees();
            rot[2] = roll.valueDegrees();
        }

        LOG(fmt::format("[RoR|DBG] DrawOffsetRotationEdit - Initial rotation angles: P={}, Y={}, R={}", 
            rot[0], rot[1], rot[2]));
        m_values_initialized = true;
    }

    // Position
    ImGui::Text("Position offset:");
    {
        // X axis
        ImGui::PushID("PosX");
        pos_changed |= ImGui::SliderFloat("X", &pos[0], -10.0f, 10.0f, "%.3f");
        {
            float btn_width = 30.0f;
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            float content_width = ImGui::GetWindowContentRegionWidth();
            // Position arrows at (content width - desired button group width)
            float arrow_x = content_width - (2 * btn_width + spacing);
            ImGui::SameLine(arrow_x);
            if (ImGui::Button("-##x", ImVec2(btn_width,0))) { pos[0] -= 0.001f; pos_changed = true; }
            ImGui::SameLine();
            if(ImGui::Button("+##x", ImVec2(btn_width,0))) { pos[0] += 0.001f; pos_changed = true; }
        }
        ImGui::PopID();
    }
    {
        // Y axis
        ImGui::PushID("PosY");
        pos_changed |= ImGui::SliderFloat("Y", &pos[1], -10.0f, 10.0f, "%.3f");
        {
            float btn_width = 30.0f;
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            float content_width = ImGui::GetWindowContentRegionWidth();
            // Position arrows at (content width - desired button group width)
            float arrow_x = content_width - (2 * btn_width + spacing);
            ImGui::SameLine(arrow_x);
            if (ImGui::Button("-##y", ImVec2(btn_width,0))) { pos[1] -= 0.001f; pos_changed = true; }
            ImGui::SameLine();
            if(ImGui::Button("+##y", ImVec2(btn_width,0))) { pos[1] += 0.001f; pos_changed = true; }
        }
        ImGui::PopID();
    }
    {
        // Z axis
        ImGui::PushID("PosZ");
        pos_changed |= ImGui::SliderFloat("Z", &pos[2], -10.0f, 10.0f, "%.3f");
        {
            float btn_width = 30.0f;
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            float content_width = ImGui::GetWindowContentRegionWidth();
            // Position arrows at (content width - desired button group width)
            float arrow_x = content_width - (2 * btn_width + spacing);
            ImGui::SameLine(arrow_x);
            if (ImGui::Button("-##z", ImVec2(btn_width,0))) { pos[2] -= 0.001f; pos_changed = true; }
            ImGui::SameLine();
            if(ImGui::Button("+##z", ImVec2(btn_width,0))) { pos[2] += 0.001f; pos_changed = true; }
        }
        ImGui::PopID();
    }

    // Rotation
    ImGui::Text("Rotation (degrees):");
    {
        // Pitch (X)
        ImGui::PushID("RotX");
        rot_changed |= ImGui::SliderFloat("Pitch (X)", &rot[0], -180.0f, 180.0f, "%.1f");
        {
            float btn_width = 30.0f;
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            float content_width = ImGui::GetWindowContentRegionWidth();
            // Position arrows at (content width - desired button group width)
            float arrow_x = content_width - (2 * btn_width + spacing);
            ImGui::SameLine(arrow_x);
            if (ImGui::Button("-##pitch", ImVec2(btn_width,0))) { rot[0] -= 0.001f; rot_changed = true; }
            ImGui::SameLine();
            if(ImGui::Button("+##pitch", ImVec2(btn_width,0))) { rot[0] += 0.001f; rot_changed = true; }
        }
        ImGui::PopID();
    }
    {
        // Yaw (Y)
        ImGui::PushID("RotY");
        rot_changed |= ImGui::SliderFloat("Yaw (Y)", &rot[1], -180.0f, 180.0f, "%.1f");
        {
            float btn_width = 30.0f;
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            float content_width = ImGui::GetWindowContentRegionWidth();
            // Position arrows at (content width - desired button group width)
            float arrow_x = content_width - (2 * btn_width + spacing);
            ImGui::SameLine(arrow_x);
            if (ImGui::Button("-##yaw", ImVec2(btn_width,0))) { rot[1] -= 0.001f; rot_changed = true; }
            ImGui::SameLine();
            if(ImGui::Button("+##yaw", ImVec2(btn_width,0))) { rot[1] += 0.001f; rot_changed = true; }
        }
        ImGui::PopID();
    }
    {
        // Roll (Z)
        ImGui::PushID("RotZ");
        rot_changed |= ImGui::SliderFloat("Roll (Z)", &rot[2], -180.0f, 180.0f, "%.1f");
        {
            float btn_width = 30.0f;
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            float content_width = ImGui::GetWindowContentRegionWidth();
            // Position arrows at (content width - desired button group width)
            float arrow_x = content_width - (2 * btn_width + spacing);
            ImGui::SameLine(arrow_x);
            if (ImGui::Button("-##roll", ImVec2(btn_width,0))) { rot[2] -= 0.001f; rot_changed = true; }
            ImGui::SameLine();
            if(ImGui::Button("+##roll", ImVec2(btn_width,0))) { rot[2] += 0.001f; rot_changed = true; }
        }
        ImGui::PopID();
    }

    // Only update if values changed
    if (pos_changed || rot_changed)
    {
        // Update position
        offset.x = pos[0];
        offset.y = pos[1];
        offset.z = pos[2];

        // For props, use the same order as originally parsed from the truck file:
        if (m_combo_selection >= m_combo_props_start)
        {
             rotation = Ogre::Quaternion(Ogre::Degree(rot[1]), Ogre::Vector3::UNIT_Y) *
                        Ogre::Quaternion(Ogre::Degree(rot[0]), Ogre::Vector3::UNIT_X) *
                        Ogre::Quaternion(Ogre::Degree(rot[2]), Ogre::Vector3::UNIT_Z);
        }
        else
        {
            rotation = 
                Ogre::Quaternion(Ogre::Degree(rot[0]), Ogre::Vector3::UNIT_X) * // Pitch
                Ogre::Quaternion(Ogre::Degree(rot[1]), Ogre::Vector3::UNIT_Y) * // Yaw  
                Ogre::Quaternion(Ogre::Degree(rot[2]), Ogre::Vector3::UNIT_Z);  // Roll
        }

        changed = true;

        LOG(fmt::format("[RoR|DBG] Prop edit - New offset ({:.3f}, {:.3f}, {:.3f}), rotation P={:.1f}, Y={:.1f}, R={:.1f}",
            offset.x, offset.y, offset.z, rot[0], rot[1], rot[2]));
    }

    // Update input fields when object is selected/reset
    if (!m_values_initialized)
    {
        // Position
        pos[0] = offset.x;
        pos[1] = offset.y;
        pos[2] = offset.z;

        // Convert current rotation to Euler angles in the correct order:
        // X (pitch), Y (yaw), Z (roll)
        Ogre::Matrix3 rot_matrix;
        rotation.ToRotationMatrix(rot_matrix);
        Ogre::Radian pitch, yaw, roll;
        rot_matrix.ToEulerAnglesXYZ(pitch, yaw, roll);
        rot[0] = pitch.valueDegrees();
        rot[1] = yaw.valueDegrees(); 
        rot[2] = roll.valueDegrees();

        m_values_initialized = true;
    }

    return changed;
}

void FlexbodyDebug::DrawOffsetRotationReset(FlexBody* flexbody)
{
    if (ImGui::Button("Reset position & rotation"))
    {
        flexbody->EndEditMode();
        m_is_editing = false;
        m_values_initialized = false;
        
        flexbody->UpdateFlexbodyPosition();
    }
}

void FlexbodyDebug::DrawOffsetRotationReset(Prop* prop)
{
    // Reset position button
    if (ImGui::Button("Reset position##pos"))
    {
        // Get prop index in stored vector
        int propIndex = m_combo_selection - m_combo_props_start;
        // Check bounds to avoid crash
        if (propIndex >= 0 && propIndex < (int)m_prop_initial_offsets.size())
        {
            m_edit_offset = m_prop_initial_offsets[propIndex];
            prop->pp_offset = m_edit_offset;
            
            // Update prop position
            if (prop->pp_scene_node)
            {
                prop->pp_scene_node->setPosition(m_edit_offset);
            }
            if (prop->pp_wheel_scene_node)
            {
                prop->pp_wheel_scene_node->setPosition(m_edit_offset);
            }

            m_offset_rot_changed = true;
            m_values_initialized = false; // Force input fields to update
        }
    }

    ImGui::SameLine();

    // Reset rotation button
    if (ImGui::Button("Reset rotation##rot"))
    {
        // Reset angles to default 
        prop->pp_rota = Ogre::Vector3::ZERO;

        // Build quaternion in same order as ActorSpawner (Z->Y->X) 
        prop->pp_rot = Ogre::Quaternion::IDENTITY;
        
        // Update prop rotation
        if (prop->pp_scene_node)
        {
            prop->pp_scene_node->setOrientation(prop->pp_rot);
        }
        if (prop->pp_wheel_scene_node)
        {
            prop->pp_wheel_scene_node->setOrientation(prop->pp_rot);
        }

        m_offset_rot_changed = true;
        m_values_initialized = false; // Force input fields to update
    }
}

void FlexbodyDebug::AnalyzeFlexbodies()
{
    // Reset
    m_combo_items = "";
    m_combo_props_start = -1;
    m_combo_selection = -1;

    // Var
    int num_combo_items = 0;

    // Analyze flexbodies
    ActorPtr actor = App::GetGameContext()->GetPlayerActor();
    if (actor && actor->GetGfxActor()->GetFlexbodies().size() > 0)
    {
        for (FlexBody* fb : actor->GetGfxActor()->GetFlexbodies())
        {
            if (fb->getPlaceholderType() == FlexBody::PlaceholderType::NOT_A_PLACEHOLDER)
            {
                ImAddItemToComboboxString(m_combo_items, fmt::format("{} ({} verts -> {} nodes)",
                    fb->getOrigMeshName(), fb->getVertexCount(), fb->getForsetNodes().size()));
            }
            else
            {
                ImAddItemToComboboxString(m_combo_items, fmt::format("{} ({})",
                    fb->getOrigMeshName(), FlexBody::PlaceholderTypeToString(fb->getPlaceholderType())));
                
            }
            num_combo_items++;
        }

        m_combo_selection = 0;

        show_locator.resize(0);
        show_locator.resize(actor->GetGfxActor()->GetFlexbodies()[m_combo_selection]->getVertexCount(), false);
    }

    // Analyze props as well
    if (actor && actor->GetGfxActor()->getProps().size() > 0)
    {
        m_combo_props_start = num_combo_items;
        for (Prop const& p: actor->GetGfxActor()->getProps())
        {
            std::string caption;
            if (p.pp_beacon_type == 'L' || p.pp_beacon_type == 'R' || p.pp_beacon_type == 'w')
            {
                caption = fmt::format("(special prop - aerial nav light '{}')", p.pp_beacon_type);
            }
            else if (p.pp_wheel_mesh_obj)
            {
                if (p.pp_mesh_obj->getLoadedMesh() && p.pp_wheel_mesh_obj->getLoadedMesh())
                {
                    caption = fmt::format("(special prop: dashboard '{}' + dirwheel '{}')", p.pp_mesh_obj->getLoadedMesh()->getName(), p.pp_wheel_mesh_obj->getLoadedMesh()->getName());
                }
                else if (!p.pp_mesh_obj->getLoadedMesh() && p.pp_wheel_mesh_obj->getLoadedMesh())
                {
                    caption = fmt::format("(special prop: no dashboard + dirwheel '{}')", p.pp_wheel_mesh_obj->getLoadedMesh()->getName());
                }
                else
                {
                    caption = "(corrupted dashboard prop - no meshes loaded)";
                }
            }
            else if (p.pp_mesh_obj && p.pp_mesh_obj->getLoadedMesh())
            {
                caption = fmt::format("{} (prop)", p.pp_mesh_obj->getLoadedMesh()->getName());
            }
            else
            {
                caption = "(corrupted prop - mesh not loaded)";
            }
            ImAddItemToComboboxString(m_combo_items, caption);
            num_combo_items++;
        }

        if (m_combo_selection == -1)
            m_combo_selection = 0;

        // NEW: Initialize stored initial offsets for props
        m_prop_initial_offsets.clear();
        for (Prop const& p : actor->GetGfxActor()->getProps())
        {
            m_prop_initial_offsets.push_back(p.pp_offset);
        }
    }

    ImTerminateComboboxString(m_combo_items);

    // Initialize offset/rotation editors
    if (actor && m_combo_selection >= 0)
    {
        if (m_combo_props_start == -1 || m_combo_selection < m_combo_props_start)
        {
            // Flexbody selected
            FlexBody* flexbody = actor->GetGfxActor()->GetFlexbodies()[m_combo_selection];
            m_edit_offset = flexbody->GetInitialOffset();
            m_edit_rotation = flexbody->GetInitialRotation();

            LOG(fmt::format("[RoR] DBG Flexbody init: mesh '{}' ", flexbody->getOrigMeshName()));

            // Log initial rotation details - let's check if it's what we expect
            Ogre::Matrix3 rot_matrix = Ogre::Matrix3::IDENTITY;
            m_edit_rotation.ToRotationMatrix(rot_matrix);
            Ogre::Radian pitch, yaw, roll;
            rot_matrix.ToEulerAnglesXYZ(pitch, yaw, roll); // Changed to XYZ to match truck file order

            LOG(fmt::format("[RoR] DBG Flexbody init: Raw constructor values - offset ({:.3f}, {:.3f}, {:.3f})", 
                flexbody->GetInitialOffset().x, flexbody->GetInitialOffset().y, flexbody->GetInitialOffset().z));

            LOG(fmt::format("[RoR] DBG Flexbody init: Processed values - offset ({:.3f}, {:.3f}, {:.3f}), rotation ({:.1f}, {:.1f}, {:.1f})", 
                m_edit_offset.x, m_edit_offset.y, m_edit_offset.z,
                pitch.valueDegrees(), yaw.valueDegrees(), roll.valueDegrees()));

            LOG(fmt::format("[RoR] DBG Flexbody init: Will reconstruct as: pitch({:.1f}) -> yaw({:.1f}) -> roll({:.1f})",
                pitch.valueDegrees(), yaw.valueDegrees(), roll.valueDegrees()));
        }
        else
        {
            // Prop selected  
            Prop& prop = actor->GetGfxActor()->getProps()[m_combo_selection - m_combo_props_start];
            m_edit_offset = prop.pp_offset;

            // Set the edit rotation from the corrected raw angles:
            m_edit_rotation =
                Ogre::Quaternion(Ogre::Degree(m_raw_angles.x), Ogre::Vector3::UNIT_X) *
                Ogre::Quaternion(Ogre::Degree(m_raw_angles.y), Ogre::Vector3::UNIT_Y) *
                Ogre::Quaternion(Ogre::Degree(m_raw_angles.z), Ogre::Vector3::UNIT_Z);

            LOG(fmt::format("[RoR|DBG] Prop init: raw angles from truck file - pitch:{}, yaw:{}, roll:{}",
                m_raw_angles.x, m_raw_angles.y, m_raw_angles.z));
        }
    }

    m_offset_rot_changed = false;
    m_values_initialized = false; // Force re-init of input fields

    // Initialize transform storage for all elements
    m_element_transforms.clear();
    if (actor)
    {
        size_t total_elements = actor->GetGfxActor()->GetFlexbodies().size() + 
                              actor->GetGfxActor()->getProps().size();
        m_element_transforms.resize(total_elements);
    }

    // Initialize the currently selected element's transform
    if (actor && m_combo_selection >= 0)
    {
        if (m_combo_props_start == -1 || m_combo_selection < m_combo_props_start)
        {
            // Flexbody selected
            FlexBody* flexbody = actor->GetGfxActor()->GetFlexbodies()[m_combo_selection];
            m_element_transforms[m_combo_selection].offset = flexbody->GetInitialOffset();
            m_element_transforms[m_combo_selection].rotation = flexbody->GetInitialRotation();
            m_element_transforms[m_combo_selection].initialized = true;
        }
        else
        {
            // Prop selected
            Prop& prop = actor->GetGfxActor()->getProps()[m_combo_selection - m_combo_props_start];
            m_element_transforms[m_combo_selection].offset = prop.pp_offset;
            
            // Build rotation from Euler angles
            m_raw_angles.x = (prop.pp_rot.x - 0.5f) * 180.0f;
            m_raw_angles.y = prop.pp_rot.y * 180.0f;
            m_raw_angles.z = prop.pp_rot.z * 180.0f;

            m_element_transforms[m_combo_selection].rotation =
                Ogre::Quaternion(Ogre::Degree(m_raw_angles.x), Ogre::Vector3::UNIT_X) *
                Ogre::Quaternion(Ogre::Degree(m_raw_angles.y), Ogre::Vector3::UNIT_Y) *
                Ogre::Quaternion(Ogre::Degree(m_raw_angles.z), Ogre::Vector3::UNIT_Z);
            m_element_transforms[m_combo_selection].initialized = true;
        }
    }

    m_offset_rot_changed = false;
}

const ImVec4 FORSETNODE_COLOR_V4(1.f, 0.87f, 0.3f, 1.f);
const ImU32 FORSETNODE_COLOR = ImColor(FORSETNODE_COLOR_V4);
const float FORSETNODE_RADIUS(2.f);
const ImU32 BASENODE_COLOR(0xff44a5ff); // ABGR format (alpha, blue, green, red)
const float BASENODE_RADIUS(3.f);
const float BEAM_THICKNESS(1.2f);
const float BLUE_BEAM_THICKNESS = BEAM_THICKNESS + 0.8f; // Blue beam looks a lot thinner for some reason
const ImU32 NODE_TEXT_COLOR(0xffcccccf); // node ID text color - ABGR format (alpha, blue, green, red)
const ImVec4 VERTEX_COLOR_V4(0.1f, 1.f, 1.f, 1.f);
const ImU32 VERTEX_COLOR = ImColor(VERTEX_COLOR_V4);
const ImU32 VERTEX_TEXT_COLOR = ImColor(171, 194, 186);
const float VERTEX_RADIUS(1.f);
const float LOCATOR_BEAM_THICKNESS(1.0f);
const ImVec4 LOCATOR_BEAM_COLOR_V4(0.05f, 1.f, 0.65f, 1.f);
const ImVec4 AXIS_X_BEAM_COLOR_V4(1.f, 0.f, 0.f, 1.f);
const ImVec4 AXIS_Y_BEAM_COLOR_V4(0.15f, 0.15f, 1.f, 1.f);
const ImU32 LOCATOR_BEAM_COLOR = ImColor(LOCATOR_BEAM_COLOR_V4);
const ImU32 AXIS_X_BEAM_COLOR = ImColor(AXIS_X_BEAM_COLOR_V4);
const ImU32 AXIS_Y_BEAM_COLOR = ImColor(AXIS_Y_BEAM_COLOR_V4);
const float VERT_HOVER_MAX_DISTANCE = 25.f;
const float MEMGRAPH_NODE_RADIUS(1.f);
const ImVec4 MEMGRAPH_NODEREF_COLOR_V4(1.f, 0.89f, 0.22f, 1.f);
const ImVec4 MEMGRAPH_NODEX_COLOR_V4(1.f, 0.21f, 0.21f, 1.f);
const ImVec4 MEMGRAPH_NODEY_COLOR_V4(0.27f, 0.76f, 1.f, 1.f);

void FlexbodyDebug::DrawDebugView(FlexBody* flexbody, Prop* prop, NodeNum_t node_ref, NodeNum_t node_x, NodeNum_t node_y)
{
    ROR_ASSERT(App::GetGameContext()->GetPlayerActor() != nullptr);
    NodeSB* nodes = App::GetGameContext()->GetPlayerActor()->GetGfxActor()->GetSimNodeBuffer();

    // Var
    ImVec2 screen_size = ImGui::GetIO().DisplaySize;
    World2ScreenConverter world2screen(
        App::GetCameraManager()->GetCamera()->getViewMatrix(true), App::GetCameraManager()->GetCamera()->getProjectionMatrix(), Ogre::Vector2(screen_size.x, screen_size.y));

    ImDrawList* drawlist = GetImDummyFullscreenWindow();

    const int LAYER_BEAMS = 0;
    const int LAYER_NODES = 1;
    const int LAYER_TEXT = 2;
    drawlist->ChannelsSplit(3);

    if (this->show_base_nodes)
    {
        drawlist->ChannelsSetCurrent(LAYER_NODES);
        Ogre::Vector3 refnode_pos = world2screen.Convert(nodes[node_ref].AbsPosition);
        Ogre::Vector3 xnode_pos = world2screen.Convert(nodes[node_x].AbsPosition);
        Ogre::Vector3 ynode_pos = world2screen.Convert(nodes[node_y].AbsPosition);
        // (z < 0) means "in front of the camera"
        if (refnode_pos.z < 0.f) {drawlist->AddCircleFilled(ImVec2(refnode_pos.x, refnode_pos.y), BASENODE_RADIUS, BASENODE_COLOR); }
        if (xnode_pos.z < 0.f) { drawlist->AddCircleFilled(ImVec2(xnode_pos.x, xnode_pos.y), BASENODE_RADIUS, BASENODE_COLOR); }
        if (ynode_pos.z < 0.f) { drawlist->AddCircleFilled(ImVec2(ynode_pos.x, ynode_pos.y), BASENODE_RADIUS, BASENODE_COLOR); }

        drawlist->ChannelsSetCurrent(LAYER_BEAMS);
        if (refnode_pos.z < 0)
        {
            if (xnode_pos.z < 0) { drawlist->AddLine(ImVec2(refnode_pos.x, refnode_pos.y), ImVec2(xnode_pos.x, xnode_pos.y), AXIS_X_BEAM_COLOR, BEAM_THICKNESS); }
            if (ynode_pos.z < 0) { drawlist->AddLine(ImVec2(refnode_pos.x, refnode_pos.y), ImVec2(ynode_pos.x, ynode_pos.y), AXIS_Y_BEAM_COLOR, BLUE_BEAM_THICKNESS); }
        }

        drawlist->ChannelsSetCurrent(LAYER_TEXT);
        drawlist->AddText(ImVec2(refnode_pos.x, refnode_pos.y), NODE_TEXT_COLOR, fmt::format("{}", node_ref).c_str());
        drawlist->AddText(ImVec2(xnode_pos.x, xnode_pos.y), NODE_TEXT_COLOR, fmt::format("{}", node_x).c_str());
        drawlist->AddText(ImVec2(ynode_pos.x, ynode_pos.y), NODE_TEXT_COLOR, fmt::format("{}", node_y).c_str());
    }

    if (flexbody && this->show_forset_nodes)
    {
        for (NodeNum_t node : flexbody->getForsetNodes())
        {
            drawlist->ChannelsSetCurrent(LAYER_NODES);
            Ogre::Vector3 pos = world2screen.Convert(nodes[node].AbsPosition);
            if (pos.z < 0.f) { drawlist->AddCircleFilled(ImVec2(pos.x, pos.y), FORSETNODE_RADIUS, FORSETNODE_COLOR); }

            drawlist->ChannelsSetCurrent(LAYER_TEXT);
            drawlist->AddText(ImVec2(pos.x, pos.y), NODE_TEXT_COLOR, fmt::format("{}", node).c_str());
        }
    }

    int hovered_vert = -1;
    float hovered_vert_dist_squared = FLT_MAX;
    ImVec2 mouse_pos = ImGui::GetMousePos();
    ImVec2 dbg_cursor_dist(0, 0);
    if (flexbody && this->show_vertices)
    {
        for (int i = 0; i < flexbody->getVertexCount(); i++)
        {
            Ogre::Vector3 vert_pos = world2screen.Convert(flexbody->getVertexPos(i));
            if (vert_pos.z < 0.f)
            {
                // Draw the visual dot
                drawlist->ChannelsSetCurrent(LAYER_NODES);
                drawlist->AddCircleFilled(ImVec2(vert_pos.x, vert_pos.y), VERTEX_RADIUS, VERTEX_COLOR);

                // Check mouse hover
                ImVec2 cursor_dist((vert_pos.x - mouse_pos.x), (vert_pos.y - mouse_pos.y));
                float dist_squared = (cursor_dist.x * cursor_dist.x) + (cursor_dist.y * cursor_dist.y);
                if (dist_squared < hovered_vert_dist_squared)
                {
                    hovered_vert = i;
                    hovered_vert_dist_squared = dist_squared;
                    dbg_cursor_dist = cursor_dist;
                }
            }
        }
    }

    // Validate mouse hover
    if (hovered_vert != -1
        && hovered_vert_dist_squared > VERT_HOVER_MAX_DISTANCE * VERT_HOVER_MAX_DISTANCE)
    {
        hovered_vert = -1;
    }

    if (flexbody)
    {
        for (int i = 0; i < flexbody->getVertexCount(); i++)
        {
            if (this->show_locator[i] || i == hovered_vert)
            {
                // The vertex
                Ogre::Vector3 vert_pos = world2screen.Convert(flexbody->getVertexPos(i));

                if (vert_pos.z < 0.f)
                {
                    if (!this->show_vertices) // don't draw twice
                    {
                        drawlist->ChannelsSetCurrent(LAYER_NODES);
                        drawlist->AddCircleFilled(ImVec2(vert_pos.x, vert_pos.y), VERTEX_RADIUS, VERTEX_COLOR);

                        // Check mouse hover
                        ImVec2 cursor_dist((vert_pos.x - mouse_pos.x), (vert_pos.y - mouse_pos.y));
                        float dist_squared = (cursor_dist.x * cursor_dist.x) + (cursor_dist.y * cursor_dist.y);
                        if (dist_squared < hovered_vert_dist_squared)
                        {
                            hovered_vert = i;
                            hovered_vert_dist_squared = dist_squared;
                            dbg_cursor_dist = cursor_dist;
                        }
                    }

                    drawlist->ChannelsSetCurrent(LAYER_TEXT);
                    drawlist->AddText(ImVec2(vert_pos.x, vert_pos.y), VERTEX_TEXT_COLOR, fmt::format("v{}", i).c_str());
                }

                // The locator nodes
                Locator_t& loc = flexbody->getVertexLocator(i);
                Ogre::Vector3 refnode_pos = world2screen.Convert(nodes[loc.ref].AbsPosition);
                Ogre::Vector3 xnode_pos = world2screen.Convert(nodes[loc.nx].AbsPosition);
                Ogre::Vector3 ynode_pos = world2screen.Convert(nodes[loc.ny].AbsPosition);
                if (!this->show_forset_nodes) // don't draw twice
                {
                    // (z < 0) means "in front of the camera"
                    if (refnode_pos.z < 0.f) { drawlist->AddCircleFilled(ImVec2(refnode_pos.x, refnode_pos.y), FORSETNODE_RADIUS, FORSETNODE_COLOR); }
                    if (xnode_pos.z < 0.f) { drawlist->AddCircleFilled(ImVec2(xnode_pos.x, xnode_pos.y), FORSETNODE_RADIUS, FORSETNODE_COLOR); }
                    if (ynode_pos.z < 0.f) { drawlist->AddCircleFilled(ImVec2(ynode_pos.x, ynode_pos.y), FORSETNODE_RADIUS, FORSETNODE_COLOR); }
                }

                drawlist->ChannelsSetCurrent(LAYER_BEAMS);
                if (refnode_pos.z < 0)
                {
                    if (xnode_pos.z < 0) { drawlist->AddLine(ImVec2(refnode_pos.x, refnode_pos.y), ImVec2(xnode_pos.x, xnode_pos.y), AXIS_X_BEAM_COLOR, BEAM_THICKNESS); }
                    if (ynode_pos.z < 0) { drawlist->AddLine(ImVec2(refnode_pos.x, refnode_pos.y), ImVec2(ynode_pos.x, ynode_pos.y), AXIS_Y_BEAM_COLOR, BLUE_BEAM_THICKNESS); }
                    if (vert_pos.z < 0) { drawlist->AddLine(ImVec2(refnode_pos.x, refnode_pos.y), ImVec2(vert_pos.x, vert_pos.y), LOCATOR_BEAM_COLOR, LOCATOR_BEAM_THICKNESS); }
                }

                if (!this->show_forset_nodes) // don't draw twice
                {
                    drawlist->AddText(ImVec2(refnode_pos.x, refnode_pos.y), NODE_TEXT_COLOR, fmt::format("{}", loc.ref).c_str());
                    drawlist->AddText(ImVec2(xnode_pos.x, xnode_pos.y), NODE_TEXT_COLOR, fmt::format("{}", loc.nx).c_str());
                    drawlist->AddText(ImVec2(ynode_pos.x, ynode_pos.y), NODE_TEXT_COLOR, fmt::format("{}", loc.ny).c_str());
                }

                if (i == hovered_vert && ImGui::IsMouseClicked(0))
                {
                    this->show_locator[i] = !this->show_locator[i];
                }
            }
        }
    }

    drawlist->ChannelsMerge();
}

void FlexbodyDebug::UpdateVisibility()
{
    // Both flexbodies and props use the same dynamic visibility mode, see `CameraMode_t` typedef and constants in file GfxData.h
    // ---------------------------------------------------------------------------------------------------------------------------

    ActorPtr actor = App::GetGameContext()->GetPlayerActor();
    if (!actor)
    {
        return;
    }

    if (this->hide_other_elements)
    {
        // Hide everything, even meshes scattered across gameplay components (wheels, wings...).
        // Note: Environment map (dynamic reflections) is halted while the "Hide other" mode is active - see `RoR::GfxEnvmap::UpdateEnvMap()`
        actor->GetGfxActor()->SetAllMeshesVisible(false);
        // Override prop dynamic visibility mode
        for (Prop& prop : actor->GetGfxActor()->getProps())
        {
            prop.pp_camera_mode_active = CAMERA_MODE_ALWAYS_HIDDEN;
        }
        // Override flexbody dynamic visibility mode
        for (FlexBody* flexbody: actor->GetGfxActor()->GetFlexbodies())
        {
            flexbody->fb_camera_mode_active = CAMERA_MODE_ALWAYS_HIDDEN;
        }

        // Then re-display what we need manually.
        auto& flexbody_vec = actor->GetGfxActor()->GetFlexbodies();
        const int combo_flexbody_selection = m_combo_selection;
        if (combo_flexbody_selection >= 0 && combo_flexbody_selection < (int)flexbody_vec.size())
        {
            flexbody_vec[combo_flexbody_selection]->fb_camera_mode_active = CAMERA_MODE_ALWAYS_VISIBLE;
        }

        auto& prop_vec = actor->GetGfxActor()->getProps();
        const int combo_prop_selection = m_combo_selection - (int)flexbody_vec.size();
        if (combo_prop_selection >= 0 && combo_prop_selection < (int)prop_vec.size())
        {
            prop_vec[combo_prop_selection].pp_camera_mode_active = CAMERA_MODE_ALWAYS_VISIBLE;
            if (prop_vec[combo_prop_selection].pp_wheel_mesh_obj)
            {
                // Special case: the steering wheel mesh visibility is not controlled by 'camera mode'
                prop_vec[combo_prop_selection].pp_wheel_mesh_obj->setVisible(true);
            }
        }
    }
    else
    {
        // Show everything, `GfxActor::UpdateProps()` will update visibility as needed.
        actor->GetGfxActor()->SetAllMeshesVisible(true);
        // Restore prop dynamic visibility mode
        for (Prop& prop : actor->GetGfxActor()->getProps())
        {
            prop.pp_camera_mode_active = prop.pp_camera_mode_orig;
        }
        // Restore flexbody dynamic visibility mode
        for (FlexBody* flexbody: actor->GetGfxActor()->GetFlexbodies())
        {
            flexbody->fb_camera_mode_active = flexbody->fb_camera_mode_orig;
        }
    }
}

void FlexbodyDebug::DrawLocatorsTable(FlexBody* flexbody, bool& locators_visible)
{
    const float content_height =
        (2.f * ImGui::GetStyle().WindowPadding.y)
        + (5.f * ImGui::GetItemsLineHeightWithSpacing())
        + ImGui::GetStyle().ItemSpacing.y * 5;
    const float child_height = ImGui::GetWindowHeight() - (content_height + 100);


    ImGui::BeginChild("FlexbodyDebug-scroll", ImVec2(0.f, child_height), false);

    // Begin table
    ImGui::Columns(5);
    ImGui::TextDisabled("Vert#");
    ImGui::NextColumn();
    ImGui::TextDisabled("REF node");
    ImGui::NextColumn();
    ImGui::TextDisabled("VX node");
    ImGui::NextColumn();
    ImGui::TextDisabled("VY node");
    ImGui::NextColumn();
    // show checkbox
    ImGui::NextColumn();
    ImGui::Separator();

    for (int i = 0; i < flexbody->getVertexCount(); i++)
    {
        ImGui::PushID(i);
        Locator_t& loc = flexbody->getVertexLocator(i);
        ImGui::TextDisabled("%d", i);
        ImGui::NextColumn();
        ImGui::Text("%d", (int)loc.ref);
        ImGui::NextColumn();
        ImGui::Text("%d", (int)loc.nx);
        ImGui::NextColumn();
        ImGui::Text("%d", (int)loc.ny);
        ImGui::NextColumn();
        bool show = this->show_locator[i];
        if (ImGui::Checkbox("Show", &show))
        {
            this->show_locator[i] = show;
        }
        ImGui::NextColumn();
        ImGui::PopID();

        locators_visible = locators_visible || this->show_locator[i];
    }

    // End table
    ImGui::Columns(1);
    ImGui::EndChild();
}

void FlexbodyDebug::DrawMemoryOrderGraph(FlexBody* flexbody)
{
    // Analysis
    NodeNum_t forset_max = std::numeric_limits<NodeNum_t>::min();
    NodeNum_t forset_min = std::numeric_limits<NodeNum_t>::max();
    for (NodeNum_t n : flexbody->getForsetNodes())
    {
        if (n > forset_max) { forset_max = n; }
        if (n < forset_min) { forset_min = n; }
    }

    // Tools!
    const float SLIDER_WIDTH = 150;
    DrawGCheckbox(App::flexbody_defrag_enabled, "Enable defrag");
    ImGui::SameLine();
    if (ImGui::Button("Reload vehicle"))
    {
        ActorModifyRequest* rq = new ActorModifyRequest;
        rq->amr_type = ActorModifyRequest::Type::RELOAD;
        rq->amr_actor = App::GetGameContext()->GetPlayerActor()->ar_instance_id;
        App::GetGameContext()->PushMessage(Message(MSG_SIM_MODIFY_ACTOR_REQUESTED, (void*)rq));
    }

    if (App::flexbody_defrag_enabled->getBool())
    {
        if (ImGui::CollapsingHeader("Artistic effects (keep all enabled for correct visual)."))
        {
            DrawGCheckbox(App::flexbody_defrag_reorder_texcoords, "Reorder texcoords");
            ImGui::SameLine();
            DrawGCheckbox(App::flexbody_defrag_reorder_indices, "Reorder indices");
            ImGui::SameLine();
            DrawGCheckbox(App::flexbody_defrag_invert_lookup, "Invert index lookup");
        }
    }

    if (App::flexbody_defrag_enabled->getBool())
    {
        ImGui::TextDisabled("Sorting: insert-sort by lowest penalty, start: REF=VX=VY=%d", (int)forset_min);
        ImGui::TextDisabled("Penalty calc: nodes (each x each), smalest nodes, node means");
        ImGui::SetNextItemWidth(SLIDER_WIDTH);
        DrawGIntSlider(App::flexbody_defrag_const_penalty, "Const penalty for inequality", 0, 15);
        ImGui::SetNextItemWidth(SLIDER_WIDTH);
        DrawGIntSlider(App::flexbody_defrag_prog_up_penalty, "Progressive penalty for upward direction", 0, 15);
        ImGui::SetNextItemWidth(SLIDER_WIDTH);
        DrawGIntSlider(App::flexbody_defrag_prog_down_penalty, "Progressive penalty for downward direction", 0, 15);
    }

    // Legend
    ImGui::TextDisabled("For optimal CPU cache usage, all dots should be roughly in ascending order (left->right), gaps are OK");
    ImGui::TextDisabled("X axis (left->right) = verts (total %d)", flexbody->getVertexCount());
    ImGui::TextDisabled("Y axis (bottom->top) = nodes (lowest %d, higest %d) ", (int)forset_min, (int)forset_max);
    ImGui::SameLine();
    ImGui::TextColored(MEMGRAPH_NODEREF_COLOR_V4, "REF");
    ImGui::SameLine();
    ImGui::TextColored(MEMGRAPH_NODEX_COLOR_V4, " VX");
    ImGui::SameLine();
    ImGui::TextColored(MEMGRAPH_NODEY_COLOR_V4, " VY");
    ImGui::Separator();

    // The graph
    ImVec2 size(ImGui::GetWindowWidth() - 2 * ImGui::GetStyle().WindowPadding.x, 200);
    ImVec2 top_left_pos = ImGui::GetCursorScreenPos();
    ImGui::Dummy(size);

    ImDrawList* drawlist = ImGui::GetWindowDrawList();
    int num_verts = flexbody->getVertexCount();
    const float x_step = (size.x / (float)num_verts);
    const float y_step = (size.y / (float)(forset_max - forset_min));
    for (int i = 0; i < num_verts; i++)
    {
        const int NUM_SEGMENTS = 5;
        Locator_t& loc = flexbody->getVertexLocator(i);
        ImVec2 bottom_x_pos = top_left_pos + ImVec2(i * x_step, size.y);

        drawlist->AddCircleFilled(bottom_x_pos - ImVec2(0, (loc.ref - forset_min) * y_step), MEMGRAPH_NODE_RADIUS, ImColor(MEMGRAPH_NODEREF_COLOR_V4), NUM_SEGMENTS);
        drawlist->AddCircleFilled(bottom_x_pos - ImVec2(0, (loc.nx - forset_min) * y_step), MEMGRAPH_NODE_RADIUS, ImColor(MEMGRAPH_NODEX_COLOR_V4), NUM_SEGMENTS);
        drawlist->AddCircleFilled(bottom_x_pos - ImVec2(0, (loc.ny - forset_min) * y_step), MEMGRAPH_NODE_RADIUS, ImColor(MEMGRAPH_NODEY_COLOR_V4), NUM_SEGMENTS);
    }
    ImGui::Separator();
}

void FlexbodyDebug::DrawMeshInfo(FlexBody* flexbody)
{
    ImGui::Text("For developers only; modders cannot affect this.");
    ImGui::Separator();
    ImGui::Text("%s", flexbody->getOrigMeshInfo().c_str());
    ImGui::Separator();
    ImGui::Text("%s", flexbody->getLiveMeshInfo().c_str());
}

void FlexbodyDebug::DrawMeshInfo(Prop* prop)
{
    ImGui::Text("The prop mesh files as provided by modder.");
    if (prop->pp_mesh_obj && prop->pp_mesh_obj->getLoadedMesh())
    {
        ImGui::Separator();
        ImGui::Text("%s", RoR::PrintMeshInfo("Prop", prop->pp_mesh_obj->getLoadedMesh()).c_str());
    }
    if (prop->pp_wheel_mesh_obj && prop->pp_wheel_mesh_obj->getLoadedMesh())
    {
        ImGui::Separator();
        ImGui::Text("%s", RoR::PrintMeshInfo("Special: steering wheel", prop->pp_wheel_mesh_obj->getLoadedMesh()).c_str());
    }
    // NOTE: `prop->pp_beacon_scene_node` has only billboards attached, not meshes.
}

bool FlexbodyDebug::DrawFlexbodyOffsetRotationEdit(FlexBody* flexbody)
{
    bool changed = false;

    // Position and rotation arrays
    static float pos[3] = {0,0,0}; 
    static float rot[3] = {0,0,0}; 

    // Initialize values when starting or resetting
    if (!m_values_initialized)
    {
        if (!m_is_editing)
        {
            flexbody->BeginEditMode();
            m_is_editing = true;
        }

        // Position
        pos[0] = flexbody->GetInitialOffset().x;
        pos[1] = flexbody->GetInitialOffset().y;
        pos[2] = flexbody->GetInitialOffset().z;

        // Extract Euler angles from quaternion
        Ogre::Matrix3 rot_matrix;
        flexbody->GetInitialRotation().ToRotationMatrix(rot_matrix);
        Ogre::Radian pitch, yaw, roll;
        rot_matrix.ToEulerAnglesXYZ(pitch, yaw, roll);
        rot[0] = pitch.valueDegrees();
        rot[1] = yaw.valueDegrees();
        rot[2] = roll.valueDegrees();

        m_values_initialized = true;
    }

    // Position sliders
    ImGui::Text("Position offset from reference node:");
    bool pos_changed = false;
    pos_changed |= ImGui::SliderFloat("X offset", &pos[0], -10.0f, 10.0f, "%.3f");
    pos_changed |= ImGui::SliderFloat("Y offset", &pos[1], -10.0f, 10.0f, "%.3f");
    pos_changed |= ImGui::SliderFloat("Z offset", &pos[2], -10.0f, 10.0f, "%.3f");

    // Rotation sliders
    ImGui::Text("Rotation (degrees, applied in X->Y->Z order):");
    bool rot_changed = false;
    rot_changed |= ImGui::SliderFloat("Pitch (X)", &rot[0], -180.0f, 180.0f, "%.1f");
    rot_changed |= ImGui::SliderFloat("Yaw (Y)", &rot[1], -180.0f, 180.0f, "%.1f");  
    rot_changed |= ImGui::SliderFloat("Roll (Z)", &rot[2], -180.0f, 180.0f, "%.1f");

    if (pos_changed || rot_changed)
    {
        // Update position
        Ogre::Vector3 new_offset(pos[0], pos[1], pos[2]);
        flexbody->UpdateCurrentOffset(new_offset);

        // Update rotation using Euler angles in X->Y->Z order
        Ogre::Quaternion new_rot = 
            Ogre::Quaternion(Ogre::Degree(rot[0]), Ogre::Vector3::UNIT_X) * 
            Ogre::Quaternion(Ogre::Degree(rot[1]), Ogre::Vector3::UNIT_Y) *
            Ogre::Quaternion(Ogre::Degree(rot[2]), Ogre::Vector3::UNIT_Z);
        flexbody->UpdateCurrentRotation(new_rot);

        // Update flexbody
        flexbody->UpdateFlexbodyPosition();

        changed = true;
    }

    return changed;
}

bool FlexbodyDebug::DrawPropOffsetRotationEdit(Prop* prop)
{
    bool changed = false;

    // Position and rotation arrays
    static float pos[3] = {0,0,0};
    static float rot[3] = {0,0,0};

    // Initialize values when starting or resetting
    if (!m_values_initialized)
    {
        // Position
        pos[0] = prop->pp_offset.x;
        pos[1] = prop->pp_offset.y;
        pos[2] = prop->pp_offset.z;

        // For props: convert raw angles from truck file to degrees
        rot[0] = prop->pp_rota.x;  // Pitch angle from truck file
        rot[1] = prop->pp_rota.y;  // Yaw angle from truck file 
        rot[2] = prop->pp_rota.z;  // Roll angle from truck file

        m_values_initialized = true;
    }

    // Position sliders
    ImGui::Text("Position offset relative to reference nodes:");
    bool pos_changed = false;
    pos_changed |= ImGui::SliderFloat("X offset", &pos[0], -10.0f, 10.0f, "%.3f");
    pos_changed |= ImGui::SliderFloat("Y offset", &pos[1], -10.0f, 10.0f, "%.3f");
    pos_changed |= ImGui::SliderFloat("Z offset", &pos[2], -10.0f, 10.0f, "%.3f");

    // Rotation sliders - note Z->Y->X order to match ActorSpawner
    ImGui::Text("Rotation (degrees):");
    bool rot_changed = false;
    rot_changed |= ImGui::SliderFloat("Roll (Z)", &rot[2], -180.0f, 180.0f, "%.1f");    // First! 
    rot_changed |= ImGui::SliderFloat("Yaw (Y)", &rot[1], -180.0f, 180.0f, "%.1f");     // Second!
    rot_changed |= ImGui::SliderFloat("Pitch (X)", &rot[0], -180.0f, 180.0f, "%.1f");   // Last!

    if (pos_changed || rot_changed)
    {
        // Update position
        prop->pp_offset = Ogre::Vector3(pos[0], pos[1], pos[2]);

        // Store raw angles for truck file
        prop->pp_rota.x = rot[0];
        prop->pp_rota.y = rot[1]; 
        prop->pp_rota.z = rot[2];

        // Create quaternion in same order as ActorSpawner (Z->Y->X)
        prop->pp_rot = Ogre::Quaternion(Ogre::Degree(rot[2]), Ogre::Vector3::UNIT_Z) *  // Roll first!
                       Ogre::Quaternion(Ogre::Degree(rot[1]), Ogre::Vector3::UNIT_Y) *   // Then yaw
                       Ogre::Quaternion(Ogre::Degree(rot[0]), Ogre::Vector3::UNIT_X);    // Then pitch

        // Update visual state immediately
        if (prop->pp_scene_node)
        {
            prop->pp_scene_node->setPosition(prop->pp_offset);
            prop->pp_scene_node->setOrientation(prop->pp_rot);
        }
        if (prop->pp_wheel_scene_node)
        {
            prop->pp_wheel_scene_node->setPosition(prop->pp_offset);
            prop->pp_wheel_scene_node->setOrientation(prop->pp_rot);
        }

        changed = true;
        m_offset_rot_changed = true;
    }

    return changed;
}
