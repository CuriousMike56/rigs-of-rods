/*
    This source file is part of Rigs of Rods (CM fork)
    Copyright 2005-2012 Pierre-Michel Ricordel
    Copyright 2007-2012 Thomas Fischer
    Copyright 2013-2020 Petr Ohlidal
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

#include "GUI_VidcamUtil.h"

#include "Actor.h"
#include "ActorManager.h"
#include "CameraManager.h"
#include "GameContext.h"
#include "GUIManager.h"
#include "GUIUtils.h"
#include "Language.h"
#include "OgreCamera.h" 
#include "OgreMatrix3.h"
#include "GfxActor.h" 
#include "Utils.h"


using namespace RoR;
using namespace GUI;

namespace {
    const ImU32 BASENODE_COLOR(0xff44a5ff); // ABGR format
    const ImU32 ALT_NODE_COLOR(0xff44ffaa);  // Light green
    const ImU32 LOOKAT_NODE_COLOR(0xffff44aa); // Pink
    const float BASENODE_RADIUS(3.f);
    const float BEAM_THICKNESS(1.2f);
    const float BLUE_BEAM_THICKNESS = BEAM_THICKNESS + 0.8f;
    const ImU32 NODE_TEXT_COLOR(0xffcccccf);
    const ImVec4 AXIS_Y_BEAM_COLOR_V4(0.15f, 0.15f, 1.f, 1.f);
    const ImVec4 AXIS_Z_BEAM_COLOR_V4(0.f, 1.f, 0.f, 1.f);
    const ImU32 AXIS_Y_BEAM_COLOR = ImColor(AXIS_Y_BEAM_COLOR_V4);
    const ImU32 AXIS_Z_BEAM_COLOR = ImColor(AXIS_Z_BEAM_COLOR_V4);
    const ImU32 ALT_BEAM_COLOR = ImColor(0.27f, 1.0f, 0.67f, 1.0f);    // Light green
    const ImU32 LOOKAT_BEAM_COLOR = ImColor(1.0f, 0.27f, 0.67f, 1.0f); // Pink
}

struct CameraOrderInfo
{
    int index;
    int load_order;
};

void VidcamUtil::SetVisible(bool v)
{
    m_is_visible = v;
    m_show_base_nodes = false; // Reset debug view when window is opened/closed
    m_camera_order.clear(); // Clear previous order
    m_orig_state.clear();
    m_selected_videocam = -1;

    if (v)
    {
        m_actor = App::GetGameContext()->GetPlayerActor();

        // Store initial states of all cameras
        if (m_actor)
        {
            const std::vector<VideoCamera>& vcams = m_actor->GetGfxActor()->getVideoCameras();
            if (!vcams.empty())
            {
                // Store camera indices in order they were defined (first = 0)
                for (size_t i = 0; i < vcams.size(); i++)
                {
                    m_camera_order.push_back({static_cast<int>(i), vcams.size() - 1 - i});
                }
                // Sort by load order
                std::sort(m_camera_order.begin(), m_camera_order.end(), 
                    [](const CameraOrderInfo& a, const CameraOrderInfo& b) {
                        return a.load_order < b.load_order;
                    });

                for (int i = 0; i < vcams.size(); i++)
                {
                    VideoCamState state;
                    state.pos_offset = vcams[i].vcam_pos_offset;
                    state.rotation = vcams[i].vcam_rotation;
                    state.node_center = vcams[i].vcam_node_center;
                    state.node_dir_y = vcams[i].vcam_node_dir_y;
                    state.node_dir_z = vcams[i].vcam_node_dir_z;
                    state.node_alt_pos = vcams[i].vcam_node_alt_pos;
                    state.node_lookat = vcams[i].vcam_node_lookat;
                    state.field_of_view = vcams[i].vcam_ogre_camera->getFOVy().valueDegrees();
                    
                    m_orig_state[i] = state;

                    // Log original rotation
                    Ogre::Matrix3 mat;
                    state.rotation.ToRotationMatrix(mat);
                    Ogre::Radian pitch, yaw, roll;
                    mat.ToEulerAnglesXYZ(pitch, yaw, roll);
                    
                    LOG(fmt::format("[Debug] Camera {}: Original rotation at spawn (Pitch, Yaw, Roll): ({:.2f}, {:.2f}, {:.2f})",
                        i, pitch.valueDegrees(), yaw.valueDegrees(), roll.valueDegrees()));
                }
            }
        }
    }
}

void VidcamUtil::Draw()
{
    // Get current vehicle and reset everything if it changed
    ActorPtr current_actor = App::GetGameContext()->GetPlayerActor();
    if (current_actor != m_actor)
    {
        // Re-initialize with new actor
        SetVisible(m_is_visible);
    }

    if (!m_is_visible)
        return;

    ImGui::SetNextWindowSize(ImVec2(500.0f, 400.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPosCenter(ImGuiCond_FirstUseEver);

    if (ImGui::Begin(_LC("VidcamUtil", "VideoCamera Utility"), &m_is_visible))
    {
        if (!m_actor)
        {
            ImGui::Text("%s", _LC("VidcamUtil", "You are on foot."));
            ImGui::End();
            return;
        }

        if (current_actor->GetGfxActor()->getVideoCameras().size() == 0)
        {
            ImGui::Text("%s", _LC("VidcamUtil", "This vehicle has no videocameras."));
            ImGui::End();
            return;
        }

        // Vehicle info
        ImGui::Text("%s", m_actor->ar_design_name.c_str());
        ImGui::Separator();

        const std::vector<VideoCamera>& vcams = m_actor->GetGfxActor()->getVideoCameras();

        // VideoCamera list
        ImGui::Text(_LC("VidcamUtil", "VideoCameras: %d"), vcams.size());

        ImGui::BeginChild("camera_list", ImVec2(200, 0), true);
        // Display cameras in truck file definition order
        for (size_t i = 0; i < m_camera_order.size(); i++)
        {
            const auto& cam_info = m_camera_order[i];
            char label[256];
            snprintf(label, 256, "[%d] %s\n%s", (int)i,
                GetVideoCamRoleStr(vcams[cam_info.index].vcam_role),
                vcams[cam_info.index].vcam_mat_name_orig.c_str());

            if (ImGui::Selectable(label, m_selected_videocam == cam_info.index))
            {
                m_selected_videocam = cam_info.index;
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // Camera properties
        if (m_selected_videocam >= 0 && m_selected_videocam < vcams.size())
        {
            DrawVideoCamera(&vcams[m_selected_videocam]);
        }
    }
    ImGui::End();

    if (m_show_base_nodes && m_selected_videocam >= 0)
    {
        const std::vector<VideoCamera>& vcams = m_actor->GetGfxActor()->getVideoCameras();
        DrawDebugView(&vcams[m_selected_videocam]);
    }
}

void VidcamUtil::DrawVideoCamera(const VideoCamera* vcam)
{
    ImGui::BeginGroup();

    // Check for classic mirror props first
    if (vcam->vcam_role == VCAM_ROLE_MIRROR_PROP_LEFT || vcam->vcam_role == VCAM_ROLE_MIRROR_PROP_RIGHT)
    {
        ImGui::Text("Classic mirror prop cameras cannot be modified.");
        ImGui::EndGroup();
        return;
    }

    // Then determine if this is any kind of tracking camera
    bool is_tracking = (vcam->vcam_role == VCAM_ROLE_TRACKING_VIDEOCAM ||
                       vcam->vcam_role == VCAM_ROLE_TRACKING_MIRROR ||
                       vcam->vcam_role == VCAM_ROLE_TRACKING_MIRROR_NOFLIP);

    // Is this a tracking mirror specifically?
    bool is_tracking_mirror = (vcam->vcam_role == VCAM_ROLE_TRACKING_MIRROR ||
                             vcam->vcam_role == VCAM_ROLE_TRACKING_MIRROR_NOFLIP);

    // Truck file format line for easy copy-paste
    {
        ImGui::TextWrapped("videocamera Truck file format line:");
        ImGui::TextWrapped("NOTE: This currently only includes the first 12 values, don't forget the rest!");
        ImGui::TextWrapped(";nref, nx, ny, ncam, nlookat, offx, offy, offz, rotx, roty, rotz, fov ...");
        
        // Get Euler angles
        float pitch = 0.f, yaw = 0.f, roll = 0.f;
        if (!is_tracking)
        {
            Ogre::Matrix3 mat;
            vcam->vcam_rotation.ToRotationMatrix(mat);
            Ogre::Radian rpitch, ryaw, rroll;
            mat.ToEulerAnglesXYZ(rpitch, ryaw, rroll);
            pitch = rpitch.valueDegrees();
            yaw = ryaw.valueDegrees();
            roll = rroll.valueDegrees();
        }

        // For tracking mirrors, alt_pos should be -1 in truck file even though internally it's set to center node
        // This works for the Thomas HDX, may not be OK for other vehicles
        int alt_pos_value = (is_tracking_mirror || vcam->vcam_node_alt_pos == NODENUM_INVALID) ? -1 : vcam->vcam_node_alt_pos;

        // Format truck file line
        std::string csv = fmt::format("{}, {}, {}, {}, {}, {:.3f}, {:.3f}, {:.3f}, {:.3f}, {:.3f}, {:.3f}, {:.1f},",
            vcam->vcam_node_center,
            vcam->vcam_node_dir_z,
            vcam->vcam_node_dir_y,
            alt_pos_value,
            vcam->vcam_node_lookat != NODENUM_INVALID ? vcam->vcam_node_lookat : -1,
            vcam->vcam_pos_offset.x,
            vcam->vcam_pos_offset.y,
            vcam->vcam_pos_offset.z,
            pitch, yaw, roll,
            m_current_fov);

        // Display in a selectable text box
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        ImGui::InputText("##truckline", const_cast<char*>(csv.c_str()), csv.length(), ImGuiInputTextFlags_ReadOnly);
        ImGui::PopStyleColor();
    }

    ImGui::Separator();

    // Node editors with spawn values
    {
        float w = ImGui::CalcTextSize("000").x + ImGui::GetStyle().FramePadding.x * 4;
        ImGui::PushItemWidth(w);
        int node_center = vcam->vcam_node_center;
        if (ImGui::InputInt(fmt::format("Center node (spawn: {})", m_orig_state[m_selected_videocam].node_center).c_str(),
            &node_center, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            node_center = std::max(0, std::min(node_center, (int)m_actor->ar_num_nodes - 1));
            std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
            vcams[m_selected_videocam].vcam_node_center = node_center;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Reset##center"))
        {
            std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
            vcams[m_selected_videocam].vcam_node_center = m_orig_state[m_selected_videocam].node_center;
        }
    }
    {
        float w = ImGui::CalcTextSize("000").x + ImGui::GetStyle().FramePadding.x * 4;
        ImGui::PushItemWidth(w);
        int node_dir_z = vcam->vcam_node_dir_z;
        if (ImGui::InputInt(fmt::format("Direction Z node (spawn: {})", m_orig_state[m_selected_videocam].node_dir_z).c_str(),
            &node_dir_z, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            node_dir_z = std::max(0, std::min(node_dir_z, (int)m_actor->ar_num_nodes - 1));
            std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
            vcams[m_selected_videocam].vcam_node_dir_z = node_dir_z;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Reset##dirz"))
        {
            std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
            vcams[m_selected_videocam].vcam_node_dir_z = m_orig_state[m_selected_videocam].node_dir_z;
        }
    }
    {
        float w = ImGui::CalcTextSize("000").x + ImGui::GetStyle().FramePadding.x * 4;
        ImGui::PushItemWidth(w);
        int node_dir_y = vcam->vcam_node_dir_y;
        if (ImGui::InputInt(fmt::format("Direction Y node (spawn: {})", m_orig_state[m_selected_videocam].node_dir_y).c_str(),
            &node_dir_y, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            node_dir_y = std::max(0, std::min(node_dir_y, (int)m_actor->ar_num_nodes - 1));
            std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
            vcams[m_selected_videocam].vcam_node_dir_y = node_dir_y;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Reset##diry"))
        {
            std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
            vcams[m_selected_videocam].vcam_node_dir_y = m_orig_state[m_selected_videocam].node_dir_y;
        }
    }
    
    // Show alt reference node editor only if this is not a tracking mirror
    if (vcam->vcam_node_alt_pos != NODENUM_INVALID && !is_tracking_mirror)
    {
        float w = ImGui::CalcTextSize("000").x + ImGui::GetStyle().FramePadding.x * 4;
        ImGui::PushItemWidth(w);
        int node_alt = vcam->vcam_node_alt_pos;
        if (ImGui::InputInt(fmt::format("Alt. reference node (spawn: {})", m_orig_state[m_selected_videocam].node_alt_pos).c_str(),
            &node_alt, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            node_alt = std::max(0, std::min(node_alt, (int)m_actor->ar_num_nodes - 1));
            std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
            vcams[m_selected_videocam].vcam_node_alt_pos = node_alt;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Reset##altpos"))
        {
            std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
            vcams[m_selected_videocam].vcam_node_alt_pos = m_orig_state[m_selected_videocam].node_alt_pos;
        }
    }
    if (vcam->vcam_node_lookat != NODENUM_INVALID)
    {
        float w = ImGui::CalcTextSize("000").x + ImGui::GetStyle().FramePadding.x * 4;
        ImGui::PushItemWidth(w);
        int node_lookat = vcam->vcam_node_lookat;
        if (ImGui::InputInt(fmt::format("Look-at node (spawn: {})", m_orig_state[m_selected_videocam].node_lookat).c_str(),
            &node_lookat, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            node_lookat = std::max(0, std::min(node_lookat, (int)m_actor->ar_num_nodes - 1));
            std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
            vcams[m_selected_videocam].vcam_node_lookat = node_lookat;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Reset##lookat"))
        {
            std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
            vcams[m_selected_videocam].vcam_node_lookat = m_orig_state[m_selected_videocam].node_lookat;
        }
    }

    ImGui::Checkbox("Show##base", &m_show_base_nodes);

    ImGui::Separator();

    ImGui::Text(_LC("VidcamUtil", "Role: %s"), GetVideoCamRoleStr(vcam->vcam_role));
    ImGui::Text(_LC("VidcamUtil", "Material: %s"), vcam->vcam_mat_name_orig.c_str());
    ImGui::Text(_LC("VidcamUtil", "Offline texture: %s"), vcam->vcam_off_tex_name.c_str());

    ImGui::Separator();

    // Position offset editor with fine adjustment buttons
    ImGui::Text(_LC("VidcamUtil", "Position offset:"));
    const float content_w = ImGui::GetContentRegionAvail().x * 0.6f;
    // Round stored values to 3 decimal places before displaying
    float offset[3] = { 
        std::round(vcam->vcam_pos_offset.x * 1000.0f) / 1000.0f,
        std::round(vcam->vcam_pos_offset.y * 1000.0f) / 1000.0f,
        std::round(vcam->vcam_pos_offset.z * 1000.0f) / 1000.0f
    };
    bool pos_changed = false;
    
    const char* axes[] = {"X", "Y", "Z"};
    for (int i = 0; i < 3; i++)
    {
        ImGui::PushID(axes[i]);
        ImGui::PushItemWidth(content_w);
        float prev_value = offset[i];
        if (ImGui::SliderFloat(axes[i], &offset[i], -10.0f, 10.0f, "%.3f"))
        {
            // Round to 3 decimal places
            offset[i] = std::round(offset[i] * 1000.0f) / 1000.0f;
            if (offset[i] != prev_value)
            {
                pos_changed = true;
            }
        }
        ImGui::PopItemWidth();
        
        float btn_width = 25.0f;
        ImGui::SameLine();
        if (ImGui::Button("-", ImVec2(btn_width,0))) 
        { 
            offset[i] = std::round((offset[i] - 0.001f) * 1000.0f) / 1000.0f;
            pos_changed = true; 
        }
        ImGui::SameLine();
        if(ImGui::Button("+", ImVec2(btn_width,0))) 
        { 
            offset[i] = std::round((offset[i] + 0.001f) * 1000.0f) / 1000.0f;
            pos_changed = true; 
        }
        ImGui::PopID();
    }

    if (pos_changed)
    {
        std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
        // Store the rounded values
        vcams[m_selected_videocam].vcam_pos_offset = Ogre::Vector3(offset[0], offset[1], offset[2]);
    }

    if (ImGui::Button(_LC("VidcamUtil", "Reset##offset")))
    {
        std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
        vcams[m_selected_videocam].vcam_pos_offset = m_orig_state[m_selected_videocam].pos_offset;
    }

    // Only show rotation controls for non-tracking cameras/mirrors
    if (!is_tracking)
    {
        // Rotation editor with fine adjustment buttons
        ImGui::Text(_LC("VidcamUtil", "Rotation (degrees):"));
        Ogre::Matrix3 mat;
        vcam->vcam_rotation.ToRotationMatrix(mat);
        Ogre::Radian pitch, yaw, roll;
        mat.ToEulerAnglesXYZ(pitch, yaw, roll);
        
        float rotation[3] = {
            std::fmod(pitch.valueDegrees() + 180.0f, 360.0f) - 180.0f,  // Normalize to -180..+180
            std::fmod(yaw.valueDegrees() + 180.0f, 360.0f) - 180.0f,
            std::fmod(roll.valueDegrees() + 180.0f, 360.0f) - 180.0f
        };
        bool rot_changed = false;

        const char* rot_axes[] = {"Pitch", "Yaw", "Roll"};
        for (int i = 0; i < 3; i++)
        {
            ImGui::PushID(rot_axes[i]);
            ImGui::PushItemWidth(content_w);

            float prev_value = rotation[i];
            float min_angle = -89.9f;
            float max_angle = 89.9f;

            if (ImGui::SliderFloat(rot_axes[i], &rotation[i], min_angle, max_angle, "%.3f"))
            {
                // Round to 3 decimal places
                rotation[i] = std::round(rotation[i] * 1000.0f) / 1000.0f;
                if (rotation[i] != prev_value)
                {
                    rot_changed = true;
                }
            }
            ImGui::PopItemWidth();
            
            float btn_width = 25.0f;
            ImGui::SameLine();
            if (ImGui::Button("-", ImVec2(btn_width,0))) 
            { 
                rotation[i] = std::max(min_angle, rotation[i] - 0.1f);
                rot_changed = true;
            }
            ImGui::SameLine();
            if(ImGui::Button("+", ImVec2(btn_width,0))) 
            { 
                rotation[i] = std::min(max_angle, rotation[i] + 0.1f);
                rot_changed = true;
            }
            ImGui::PopID();
        }

        if (rot_changed)
        {
            // Safety check - validate all values are in proper range
            bool valid = true;
            for (int i = 0; i < 3; i++)
            {
                if (std::isnan(rotation[i]) || rotation[i] < -180.f || rotation[i] > 180.f)
                {
                    valid = false;
                    break;
                }
            }

            if (valid)
            {
                Ogre::Matrix3 newMat;
                newMat.FromEulerAnglesXYZ(
                    Ogre::Radian(Ogre::Degree(rotation[0])),
                    Ogre::Radian(Ogre::Degree(rotation[1])),
                    Ogre::Radian(Ogre::Degree(rotation[2])));

                std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
                Ogre::Quaternion originalRotation = vcams[m_selected_videocam].vcam_rotation;
                vcams[m_selected_videocam].vcam_rotation.FromRotationMatrix(newMat);

                // Convert both rotations to Euler angles for logging
                Ogre::Matrix3 origMat, editedMat;
                originalRotation.ToRotationMatrix(origMat);
                vcams[m_selected_videocam].vcam_rotation.ToRotationMatrix(editedMat);

                Ogre::Radian origPitch, origYaw, origRoll;
                origMat.ToEulerAnglesXYZ(origPitch, origYaw, origRoll);

                Ogre::Radian editedPitch, editedYaw, editedRoll;
                editedMat.ToEulerAnglesXYZ(editedPitch, editedYaw, editedRoll);

                LOG(fmt::format("[Debug] Camera {}: Rotation changed\n"
                               "Original (Pitch, Yaw, Roll): ({:.2f}, {:.2f}, {:.2f})\n"
                               "Modified (Pitch, Yaw, Roll): ({:.2f}, {:.2f}, {:.2f})",
                               m_selected_videocam,
                               origPitch.valueDegrees(), origYaw.valueDegrees(), origRoll.valueDegrees(),
                               editedPitch.valueDegrees(), editedYaw.valueDegrees(), editedRoll.valueDegrees()));
            }
        }
        
        if (ImGui::Button(_LC("VidcamUtil", "Reset##rotation")))
        {
            std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
            vcams[m_selected_videocam].vcam_rotation = m_orig_state[m_selected_videocam].rotation;
        }
    }
    else
    {
        ImGui::Text("Rotation controls disabled for tracking cameras/mirrors");
        ImGui::Text("(rotation is auto-calculated based on look-at node)");
    }

    ImGui::Separator();

    // FOV editor 
    ImGui::Text(_LC("VidcamUtil", "Field of View (degrees):"));
    float w = ImGui::GetContentRegionAvail().x * 0.6f;
    ImGui::PushItemWidth(w);
    
    static float initial_fov = 45.0f; // Keep this static for initialization
    if (m_selected_videocam >= 0) 
    {
        // Initialize both current and initial FOV when camera is selected
        static int prev_selected_cam = -1;
        if (prev_selected_cam != m_selected_videocam)
        {
            m_current_fov = m_orig_state[m_selected_videocam].field_of_view;
            initial_fov = m_current_fov;
            prev_selected_cam = m_selected_videocam;
        }

        bool fov_changed = false;
        float prev_fov = m_current_fov;
        if (ImGui::SliderFloat("##fov", &m_current_fov, 10.0f, 120.0f, "%.1f"))
        {
            // Round to 1 decimal place
            m_current_fov = std::round(m_current_fov * 10.0f) / 10.0f;
            if (m_current_fov != prev_fov)
            {
                fov_changed = true;
            }
        }

        if (fov_changed)
        {
            std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
            vcams[m_selected_videocam].vcam_ogre_camera->setFOVy(Ogre::Degree(m_current_fov));
        }
    }

    if (ImGui::Button(_LC("VidcamUtil", "Reset##fov")))
    {
        m_current_fov = initial_fov;
        std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
        vcams[m_selected_videocam].vcam_ogre_camera->setFOVy(Ogre::Degree(m_current_fov));
    }

    ImGui::EndGroup();
}

void VidcamUtil::DrawDebugView(const VideoCamera* vcam)
{
    NodeSB* nodes = m_actor->GetGfxActor()->GetSimNodeBuffer();

    // Setup world to screen conversion
    ImVec2 screen_size = ImGui::GetIO().DisplaySize;
    World2ScreenConverter world2screen(
        App::GetCameraManager()->GetCamera()->getViewMatrix(true), App::GetCameraManager()->GetCamera()->getProjectionMatrix(), Ogre::Vector2(screen_size.x, screen_size.y));

    ImDrawList* drawlist = GetImDummyFullscreenWindow();

    const int LAYER_BEAMS = 0;
    const int LAYER_NODES = 1;
    const int LAYER_TEXT = 2;
    drawlist->ChannelsSplit(3);

    // Get node positions
    Ogre::Vector3 center_pos = world2screen.Convert(nodes[vcam->vcam_node_center].AbsPosition);
    Ogre::Vector3 dir_y_pos = world2screen.Convert(nodes[vcam->vcam_node_dir_y].AbsPosition);
    Ogre::Vector3 dir_z_pos = world2screen.Convert(nodes[vcam->vcam_node_dir_z].AbsPosition);

    // Get additional node positions
    Ogre::Vector3 alt_pos;
    Ogre::Vector3 lookat_pos;
    bool has_alt = vcam->vcam_node_alt_pos != NODENUM_INVALID;
    bool has_lookat = vcam->vcam_node_lookat != NODENUM_INVALID;
    
    if (has_alt)
        alt_pos = world2screen.Convert(nodes[vcam->vcam_node_alt_pos].AbsPosition);
    if (has_lookat)
        lookat_pos = world2screen.Convert(nodes[vcam->vcam_node_lookat].AbsPosition);

    // Draw nodes
    drawlist->ChannelsSetCurrent(LAYER_NODES);
    if (center_pos.z < 0.f) { drawlist->AddCircleFilled(ImVec2(center_pos.x, center_pos.y), BASENODE_RADIUS, BASENODE_COLOR); }
    if (dir_y_pos.z < 0.f) { drawlist->AddCircleFilled(ImVec2(dir_y_pos.x, dir_y_pos.y), BASENODE_RADIUS, BASENODE_COLOR); }
    if (dir_z_pos.z < 0.f) { drawlist->AddCircleFilled(ImVec2(dir_z_pos.x, dir_z_pos.y), BASENODE_RADIUS, BASENODE_COLOR); }
    if (has_alt && alt_pos.z < 0.f) { drawlist->AddCircleFilled(ImVec2(alt_pos.x, alt_pos.y), BASENODE_RADIUS, ALT_NODE_COLOR); }
    if (has_lookat && lookat_pos.z < 0.f) { drawlist->AddCircleFilled(ImVec2(lookat_pos.x, lookat_pos.y), BASENODE_RADIUS, LOOKAT_NODE_COLOR); }

    // Draw connection beams
    drawlist->ChannelsSetCurrent(LAYER_BEAMS);
    if (center_pos.z < 0.f)
    {
        if (dir_y_pos.z < 0.f) { drawlist->AddLine(ImVec2(center_pos.x, center_pos.y), ImVec2(dir_y_pos.x, dir_y_pos.y), AXIS_Y_BEAM_COLOR, BLUE_BEAM_THICKNESS); }
        if (dir_z_pos.z < 0.f) { drawlist->AddLine(ImVec2(center_pos.x, center_pos.y), ImVec2(dir_z_pos.x, dir_z_pos.y), AXIS_Z_BEAM_COLOR, BEAM_THICKNESS); }
        if (has_alt && alt_pos.z < 0.f) { drawlist->AddLine(ImVec2(center_pos.x, center_pos.y), ImVec2(alt_pos.x, alt_pos.y), ALT_BEAM_COLOR, BEAM_THICKNESS); }
        if (has_lookat && lookat_pos.z < 0.f) { drawlist->AddLine(ImVec2(center_pos.x, center_pos.y), ImVec2(lookat_pos.x, lookat_pos.y), LOOKAT_BEAM_COLOR, BEAM_THICKNESS); }
    }

    // Draw node numbers
    drawlist->ChannelsSetCurrent(LAYER_TEXT);
    drawlist->AddText(ImVec2(center_pos.x, center_pos.y), NODE_TEXT_COLOR, fmt::format("{}", vcam->vcam_node_center).c_str());
    drawlist->AddText(ImVec2(dir_y_pos.x, dir_y_pos.y), NODE_TEXT_COLOR, fmt::format("{}", vcam->vcam_node_dir_y).c_str());
    drawlist->AddText(ImVec2(dir_z_pos.x, dir_z_pos.y), NODE_TEXT_COLOR, fmt::format("{}", vcam->vcam_node_dir_z).c_str());
    if (has_alt) { drawlist->AddText(ImVec2(alt_pos.x, alt_pos.y), NODE_TEXT_COLOR, fmt::format("{}", vcam->vcam_node_alt_pos).c_str()); }
    if (has_lookat) { drawlist->AddText(ImVec2(lookat_pos.x, lookat_pos.y), NODE_TEXT_COLOR, fmt::format("{}", vcam->vcam_node_lookat).c_str()); }

    drawlist->ChannelsMerge();
}

const char* VidcamUtil::GetVideoCamRoleStr(VideoCamRole role)
{
    switch (role)
    {
    case VCAM_ROLE_VIDEOCAM:           return "Video";
    case VCAM_ROLE_TRACKING_VIDEOCAM:  return "Tracking";
    case VCAM_ROLE_MIRROR:             return "Mirror";
    case VCAM_ROLE_MIRROR_NOFLIP:      return "Mirror (no flip)";
    case VCAM_ROLE_MIRROR_PROP_LEFT:   return "Classic mirror prop (left)";
    case VCAM_ROLE_MIRROR_PROP_RIGHT:  return "Classic mirror prop (right)";
    case VCAM_ROLE_TRACKING_MIRROR:    return "Tracking mirror";
    case VCAM_ROLE_TRACKING_MIRROR_NOFLIP: return "Tracking mirror (no flip)";
    default:                           return "Unknown";
    }
}
