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

#include "GUI_FlareUtil.h"

#include "Actor.h"
#include "ActorManager.h"
#include "GameContext.h"
#include "GUIManager.h"
#include "Language.h"

using namespace RoR;
using namespace GUI;

void FlareUtil::SetVisible(bool v)
{
    m_is_visible = v;
    if (v)
    {
        m_actor = App::GetGameContext()->GetPlayerActor();
    }
}

void FlareUtil::Draw()
{
    // Get current vehicle
    ActorPtr current_actor = App::GetGameContext()->GetPlayerActor();
    
    // Reset selection if vehicle changed
    if (current_actor != m_actor)
    {
        m_actor = current_actor;
        m_selected_flare = -1;
    }

    if (!m_is_visible)
        return;

    ImGui::SetNextWindowSize(ImVec2(500.0f, 400.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPosCenter(ImGuiCond_FirstUseEver);

    if (ImGui::Begin(_LC("FlareUtil", "Flare Utility"), &m_is_visible))
    {
        if (!m_actor)
        {
            ImGui::Text("%s", _LC("FlareUtil", "You are on foot."));
            ImGui::End();
            return;
        }

        // Vehicle info
        ImGui::Text("%s", m_actor->ar_design_name.c_str());
        ImGui::Separator();

        // Flare selection
        ImGui::Text(_LC("FlareUtil", "Flares: %d"), m_actor->ar_flares.size());
        
        ImGui::BeginChild("flare_list", ImVec2(200, 0), true);
        for (unsigned int i = 0; i < m_actor->ar_flares.size(); i++)
        {
            const flare_t& flare = m_actor->ar_flares[i];

            // Build flare description
            char label[128];
            if (flare.fl_type == FlareType::USER)
            {
                snprintf(label, 128, "[%u] User %d", i, flare.controlnumber + 1);
            }
            else if (flare.fl_type == FlareType::DASHBOARD)
            {
                snprintf(label, 128, "[%u] Dashboard", i);
            }
            else
            {
                snprintf(label, 128, "[%u] %c", i, (char)flare.fl_type);
            }

            if (ImGui::Selectable(label, m_selected_flare == i))
            {
                m_selected_flare = i;
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // Flare properties
        if (m_selected_flare < m_actor->ar_flares.size())
        {
            flare_t& flare = m_actor->ar_flares[m_selected_flare];
            ImGui::BeginGroup();

            // Display type info
            ImGui::Text(_LC("FlareUtil", "Type: %c"), (char)flare.fl_type);
            if (flare.fl_type == FlareType::USER)
            {
                ImGui::Text(_LC("FlareUtil", "Control number: %d"), flare.controlnumber + 1);
            }

            ImGui::Separator();

            // Position editor
            ImGui::Text(_LC("FlareUtil", "Position offset:"));
            float pos[3] = {flare.offsetx, flare.offsety, flare.offsetz};
            if (ImGui::DragFloat3("##pos", pos, 0.1f))
            {
                flare.offsetx = pos[0];
                flare.offsety = pos[1]; 
                flare.offsetz = pos[2];
            }

            // Reference nodes
            ImGui::Text(_LC("FlareUtil", "Reference node: %d"), flare.noderef);
            ImGui::Text(_LC("FlareUtil", "Node X: %d"), flare.nodex);
            ImGui::Text(_LC("FlareUtil", "Node Y: %d"), flare.nodey);

            ImGui::EndGroup();
        }
    }
    ImGui::End();
}
