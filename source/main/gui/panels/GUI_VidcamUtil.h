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

#pragma once

#include "Actor.h"
#include "Application.h"
#include "ForwardDeclarations.h"
#include "GfxActor.h"

namespace RoR {
namespace GUI {

class VidcamUtil
{
public:
    bool IsVisible() const { return m_is_visible; }
    bool IsHovered() const { return m_is_hovered; }
    void SetVisible(bool value);
    void Draw();

private:
    void DrawVideoCamera(const VideoCamera* vcam);
    const char* GetVideoCamRoleStr(VideoCamRole role);
    void DrawDebugView(const VideoCamera* vcam);

    // Tracks camera definition order
    struct CameraOrderInfo
    {
        int index;          // Index in vcams array
        size_t load_order;  // Order in which camera was loaded (0 = first)
    };
    std::vector<CameraOrderInfo> m_camera_order;

    bool m_is_visible = false;
    bool m_is_hovered = false;
    bool m_show_base_nodes = false;
    ActorPtr m_actor;
    int m_selected_videocam = -1;
    float m_current_fov = 45.0f;

    // Store original values for reset
    struct VideoCamState 
    {
        VideoCamState():
            pos_offset(Ogre::Vector3::ZERO),
            rotation(Ogre::Quaternion::IDENTITY),
            node_center(0), 
            node_dir_y(0),  
            node_dir_z(0),
            node_alt_pos(NODENUM_INVALID),
            node_lookat(NODENUM_INVALID),
            field_of_view(45.0f)
        {}

        Ogre::Vector3    pos_offset;
        Ogre::Quaternion rotation;
        int node_center;
        int node_dir_y;
        int node_dir_z;
        int node_alt_pos;
        int node_lookat;
        float field_of_view;
    };
    std::map<int, VideoCamState> m_orig_state;
    
    float initial_fov = 45.0f; // Default initial FOV
};

} // namespace GUI
} // namespace RoR
