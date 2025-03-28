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
#include "GameContext.h"
#include "GUIManager.h"
#include "Language.h"
#include "OgreMatrix3.h"

using namespace RoR;
using namespace GUI;

void VidcamUtil::SetVisible(bool v)
{
    m_is_visible = v;
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

    if (ImGui::Begin(_LC("VidcamUtil", "Videocamera Utility"), &m_is_visible))
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

        // Camera list
        ImGui::Text(_LC("VidcamUtil", "Cameras: %d"), vcams.size());

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
}

void VidcamUtil::DrawVideoCamera(const VideoCamera* vcam)
{
    ImGui::BeginGroup();

    // Display basic info
    ImGui::Text(_LC("VidcamUtil", "Role: %s"), GetVideoCamRoleStr(vcam->vcam_role));
    ImGui::Text(_LC("VidcamUtil", "Material: %s"), vcam->vcam_mat_name_orig.c_str());
    ImGui::Text(_LC("VidcamUtil", "Offline texture: %s"), vcam->vcam_off_tex_name.c_str());

    ImGui::Separator();

    // Position offset editor - we can't modify const object directly
    ImGui::Text(_LC("VidcamUtil", "Position offset:"));
    {
        float offset[3] = { vcam->vcam_pos_offset.x, vcam->vcam_pos_offset.y, vcam->vcam_pos_offset.z };
        if (ImGui::DragFloat3("##offset", offset, 0.01f))
        {
            // Need to modify through the original vector
            std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
            vcams[m_selected_videocam].vcam_pos_offset = Ogre::Vector3(offset[0], offset[1], offset[2]);
        }

        ImGui::SameLine();
        if (ImGui::Button(_LC("VidcamUtil", "Reset##offset")))
        {
            std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
            vcams[m_selected_videocam].vcam_pos_offset = m_orig_state[m_selected_videocam].pos_offset;
        }
    }

    // Rotation editor (as Euler angles)
    ImGui::Text(_LC("VidcamUtil", "Rotation (degrees):"));
    {
        // Create and get rotation matrix
        Ogre::Matrix3 mat;
        vcam->vcam_rotation.ToRotationMatrix(mat);

        // Get euler angles
        Ogre::Radian pitch, yaw, roll;
        mat.ToEulerAnglesXYZ(pitch, yaw, roll);
                             
        float rotation[3] = {
            pitch.valueDegrees(),
            yaw.valueDegrees(),
            roll.valueDegrees()
        };
                             
        if (ImGui::DragFloat3("##rotation", rotation, 0.1f))
        {
            // Convert back to quaternion
            Ogre::Matrix3 newMat;
            newMat.FromEulerAnglesXYZ(
                Ogre::Radian(Ogre::Degree(rotation[0])),
                Ogre::Radian(Ogre::Degree(rotation[1])),
                Ogre::Radian(Ogre::Degree(rotation[2])));

            // Need to modify through the original vector
            std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
            vcams[m_selected_videocam].vcam_rotation.FromRotationMatrix(newMat);
        }

        ImGui::SameLine();
        if (ImGui::Button(_LC("VidcamUtil", "Reset##rotation")))
        {
            std::vector<VideoCamera>& vcams = const_cast<std::vector<VideoCamera>&>(m_actor->GetGfxActor()->getVideoCameras());
            vcams[m_selected_videocam].vcam_rotation = m_orig_state[m_selected_videocam].rotation;
        }
    }

    ImGui::EndGroup();
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
