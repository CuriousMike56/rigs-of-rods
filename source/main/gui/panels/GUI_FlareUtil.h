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

class FlareUtil
{
public:
    bool IsVisible() const { return m_is_visible; }
    bool IsHovered() const { return m_is_hovered; }
    void SetVisible(bool value);
    void Draw();

private:
    const char* GetFlareTypeDesc(FlareType type);
    void DrawDebugView(const flare_t* flare);
    
    bool m_is_visible = false;
    bool m_is_hovered = false;
    bool m_show_base_nodes = false;
    ActorPtr m_actor;
    unsigned int m_selected_flare = 0;

    // Store original values
    struct OriginalValues 
    {
        float offset_x = 0.f;
        float offset_y = 0.f;
        float offset_z = 0.f;
        float size = 1.f;
        int noderef = 0;
        int nodex = 0;
        int nodey = 0;
    };
    OriginalValues m_spawn_values;
};

} // namespace GUI
} // namespace RoR
