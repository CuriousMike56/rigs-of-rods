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

    bool m_is_visible = false;
    bool m_is_hovered = false;
    ActorPtr m_actor;
    int m_selected_videocam = -1;

    // Store original values for reset
    struct VideoCamState 
    {
        VideoCamState():
            pos_offset(Ogre::Vector3::ZERO),
            rotation(Ogre::Quaternion::IDENTITY)
        {}

        Ogre::Vector3    pos_offset;
        Ogre::Quaternion rotation;
    };
    std::map<int, VideoCamState> m_orig_state;
};

} // namespace GUI
} // namespace RoR
