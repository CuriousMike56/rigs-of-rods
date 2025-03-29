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
    const float BASENODE_RADIUS(3.f);
    const float BEAM_THICKNESS(1.2f);
    const float BLUE_BEAM_THICKNESS = BEAM_THICKNESS + 0.8f;
    const ImU32 NODE_TEXT_COLOR(0xffcccccf);
    const ImVec4 AXIS_Y_BEAM_COLOR_V4(0.15f, 0.15f, 1.f, 1.f);
    const ImVec4 AXIS_Z_BEAM_COLOR_V4(0.f, 1.f, 0.f, 1.f);
    const ImU32 AXIS_Y_BEAM_COLOR = ImColor(AXIS_Y_BEAM_COLOR_V4);
    const ImU32 AXIS_Z_BEAM_COLOR = ImColor(AXIS_Z_BEAM_COLOR_V4);
}

void VidcamUtil::SetVisible(bool v)
{
    m_is_visible = v;
    m_show_base_nodes = false; // Reset debug view when window is opened/closed
    if (v)
    {
        m_actor = App::GetGameContext()->GetPlayerActor();
        m_selected_videocam = -1;
        m_orig_state.clear();

        // Store initial states of all cameras
        if (m_actor)
        {
            const std::vector<VideoCamera>& vcams = m_actor->GetGfxActor()->getVideoCameras();
            if (!vcams.empty())
            {
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
                    m_orig_state[i] = state;
                }
            }
        }
    }
}

void VidcamUtil::Draw()
{

    // Get current vehicle
    ActorPtr current_actor = App::GetGameContext()->GetPlayerActor();
    
    // Reset selection if vehicle changed
    if (current_actor != m_actor)
    {
        m_actor = current_actor;
        m_selected_videocam = -1;
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

        // Vehicle info
        ImGui::Text("%s", m_actor->ar_design_name.c_str());
        ImGui::Separator();

        const std::vector<VideoCamera>& vcams = m_actor->GetGfxActor()->getVideoCameras();

        // VideoCamera list
        ImGui::Text(_LC("VidcamUtil", "VideoCameras: %d"), vcams.size());

        ImGui::BeginChild("camera_list", ImVec2(200, 0), true);
        for (int i = 0; i < vcams.size(); i++)
        {
            char label[128];
            snprintf(label, 128, "[%d] %s", i, GetVideoCamRoleStr(vcams[i].vcam_role));

            if (ImGui::Selectable(label, m_selected_videocam == i))
            {
                m_selected_videocam = i;
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
    
    if (vcam->vcam_node_alt_pos != NODENUM_INVALID)
    {
        float w = ImGui::CalcTextSize("000").x + ImGui::GetStyle().FramePadding.x * 4;
        ImGui::PushItemWidth(w);
        int node_alt = vcam->vcam_node_alt_pos;
        if (ImGui::InputInt(fmt::format("Alt. pos node (spawn: {})", m_orig_state[m_selected_videocam].node_alt_pos).c_str(),
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
    float w = ImGui::GetContentRegionAvail().x * 0.6f;
    float offset[3] = { vcam->vcam_pos_offset.x, vcam->vcam_pos_offset.y, vcam->vcam_pos_offset.z };
    bool pos_changed = false;
    
    const char* axes[] = {"X", "Y", "Z"};
    for (int i = 0; i < 3; i++)
    {
        ImGui::PushID(axes[i]);
        ImGui::PushItemWidth(w);
        if (ImGui::SliderFloat(axes[i], &offset[i], -10.0f, 10.0f, "%.3f"))
        {
            pos_changed = true;
        }
        ImGui::PopItemWidth();
        
        float btn_width = 25.0f;
        ImGui::SameLine();
        if (ImGui::Button("-", ImVec2(btn_width,0))) 
        { 
            offset[i] -= 0.001f; 
            pos_changed = true; 
        }
        ImGui::SameLine();
        if(ImGui::Button("+", ImVec2(btn_width,0))) 
        { 
            offset[i] += 0.001f; 
            pos_changed = true; 
        }
        ImGui::PopID();
    }

    if (pos_changed)
    {
        std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
        vcams[m_selected_videocam].vcam_pos_offset = Ogre::Vector3(offset[0], offset[1], offset[2]);
    }

    if (ImGui::Button(_LC("VidcamUtil", "Reset##offset")))
    {
        std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
        vcams[m_selected_videocam].vcam_pos_offset = m_orig_state[m_selected_videocam].pos_offset;
    }

    // Rotation editor with fine adjustment buttons
    ImGui::Text(_LC("VidcamUtil", "Rotation (degrees):"));
    Ogre::Matrix3 mat;
    vcam->vcam_rotation.ToRotationMatrix(mat);
    Ogre::Radian pitch, yaw, roll;
    mat.ToEulerAnglesXYZ(pitch, yaw, roll);
    
    float rotation[3] = {
        pitch.valueDegrees(),
        yaw.valueDegrees(),
        roll.valueDegrees()
    };
    bool rot_changed = false;

    const char* rot_axes[] = {"Pitch", "Yaw", "Roll"};
    for (int i = 0; i < 3; i++)
    {
        ImGui::PushID(rot_axes[i]);
        ImGui::PushItemWidth(w);
        if (ImGui::SliderFloat(rot_axes[i], &rotation[i], -180.0f, 180.0f, "%.1f"))
        {
            rot_changed = true;
        }
        ImGui::PopItemWidth();
        
        float btn_width = 25.0f;
        ImGui::SameLine();
        if (ImGui::Button("-", ImVec2(btn_width,0))) 
        { 
            rotation[i] -= 0.1f; 
            rot_changed = true; 
        }
        ImGui::SameLine();
        if(ImGui::Button("+", ImVec2(btn_width,0))) 
        { 
            rotation[i] += 0.1f; 
            rot_changed = true; 
        }
        ImGui::PopID();
    }

    if (rot_changed)
    {
        Ogre::Matrix3 newMat;
        newMat.FromEulerAnglesXYZ(
            Ogre::Radian(Ogre::Degree(rotation[0])),
            Ogre::Radian(Ogre::Degree(rotation[1])),
            Ogre::Radian(Ogre::Degree(rotation[2])));

        std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
        vcams[m_selected_videocam].vcam_rotation.FromRotationMatrix(newMat);
    }
    
    if (ImGui::Button(_LC("VidcamUtil", "Reset##rotation")))
    {
        std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
        vcams[m_selected_videocam].vcam_rotation = m_orig_state[m_selected_videocam].rotation;
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

    // Draw nodes
    drawlist->ChannelsSetCurrent(LAYER_NODES);
    if (center_pos.z < 0.f) { drawlist->AddCircleFilled(ImVec2(center_pos.x, center_pos.y), BASENODE_RADIUS, BASENODE_COLOR); }
    if (dir_y_pos.z < 0.f) { drawlist->AddCircleFilled(ImVec2(dir_y_pos.x, dir_y_pos.y), BASENODE_RADIUS, BASENODE_COLOR); }
    if (dir_z_pos.z < 0.f) { drawlist->AddCircleFilled(ImVec2(dir_z_pos.x, dir_z_pos.y), BASENODE_RADIUS, BASENODE_COLOR); }

    // Draw connection beams
    drawlist->ChannelsSetCurrent(LAYER_BEAMS);
    if (center_pos.z < 0.f)
    {
        if (dir_y_pos.z < 0.f) { drawlist->AddLine(ImVec2(center_pos.x, center_pos.y), ImVec2(dir_y_pos.x, dir_y_pos.y), AXIS_Y_BEAM_COLOR, BLUE_BEAM_THICKNESS); }
        if (dir_z_pos.z < 0.f) { drawlist->AddLine(ImVec2(center_pos.x, center_pos.y), ImVec2(dir_z_pos.x, dir_z_pos.y), AXIS_Z_BEAM_COLOR, BEAM_THICKNESS); }
    }

    // Draw node numbers
    drawlist->ChannelsSetCurrent(LAYER_TEXT);
    drawlist->AddText(ImVec2(center_pos.x, center_pos.y), NODE_TEXT_COLOR, fmt::format("{}", vcam->vcam_node_center).c_str());
    drawlist->AddText(ImVec2(dir_y_pos.x, dir_y_pos.y), NODE_TEXT_COLOR, fmt::format("{}", vcam->vcam_node_dir_y).c_str());
    drawlist->AddText(ImVec2(dir_z_pos.x, dir_z_pos.y), NODE_TEXT_COLOR, fmt::format("{}", vcam->vcam_node_dir_z).c_str());

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
    default:                           return "Invalid";
    }
}
