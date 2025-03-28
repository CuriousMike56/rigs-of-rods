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

        // Initialize spawn values for the default selection (flare 0)
        if (m_actor && m_actor->ar_flares.size() > 0)
        {
            const flare_t& flare = m_actor->ar_flares[0];
            m_spawn_values.offset_x = flare.offsetx;
            m_spawn_values.offset_y = flare.offsety; 
            m_spawn_values.offset_z = flare.offsetz;
            m_spawn_values.size = flare.size;
        }
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
                // Store original values when selecting new flare
                m_spawn_values.offset_x = flare.offsetx;
                m_spawn_values.offset_y = flare.offsety;
                m_spawn_values.offset_z = flare.offsetz;
                m_spawn_values.size = flare.size;
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // Flare properties
        if (m_selected_flare < m_actor->ar_flares.size())
        {
            flare_t& flare = m_actor->ar_flares[m_selected_flare];
            ImGui::BeginGroup();

            // Display type and reference nodes
            ImGui::Text(_LC("FlareUtil", "Type: %c (%s)"), (char)flare.fl_type, GetFlareTypeDesc(flare.fl_type));
            ImGui::Text(_LC("FlareUtil", "Ref node: %d"), flare.noderef);
            ImGui::Text(_LC("FlareUtil", "Node X: %d"), flare.nodex);
            ImGui::Text(_LC("FlareUtil", "Node Y: %d"), flare.nodey);
            if (flare.fl_type == FlareType::USER)
            {
                ImGui::Text(_LC("FlareUtil", "Control number: %d"), flare.controlnumber + 1);
            }

            ImGui::Separator();

            // Position editor with fine adjustment buttons
            ImGui::Text(_LC("FlareUtil", "Position offset:"));
            {
                // X axis
                ImGui::PushID("PosX");
                float pos_x = flare.offsetx;
                bool pos_changed = ImGui::SliderFloat("X", &pos_x, -10.0f, 10.0f, "%.3f");
                {
                    float btn_width = 25.0f;
                    float spacing = ImGui::GetStyle().ItemSpacing.x;
                    ImGui::SameLine();
                    if (ImGui::Button("-##x", ImVec2(btn_width,0))) { pos_x -= 0.001f; pos_changed = true; }
                    ImGui::SameLine();
                    if(ImGui::Button("+##x", ImVec2(btn_width,0))) { pos_x += 0.001f; pos_changed = true; }
                }
                if (pos_changed) { flare.offsetx = pos_x; }
                ImGui::PopID();
            }
            {
                // Y axis  
                ImGui::PushID("PosY");
                float pos_y = flare.offsety;
                bool pos_changed = ImGui::SliderFloat("Y", &pos_y, -10.0f, 10.0f, "%.3f");
                {
                    float btn_width = 25.0f;
                    float spacing = ImGui::GetStyle().ItemSpacing.x;
                    ImGui::SameLine();
                    if (ImGui::Button("-##y", ImVec2(btn_width,0))) { pos_y -= 0.001f; pos_changed = true; }
                    ImGui::SameLine();
                    if(ImGui::Button("+##y", ImVec2(btn_width,0))) { pos_y += 0.001f; pos_changed = true; }
                }
                if (pos_changed) { flare.offsety = pos_y; }
                ImGui::PopID();
            }
            {
                // Z axis
                ImGui::PushID("PosZ");
                float pos_z = flare.offsetz;
                bool pos_changed = ImGui::SliderFloat("Z", &pos_z, -10.0f, 10.0f, "%.3f");
                {
                    float btn_width = 25.0f;
                    float spacing = ImGui::GetStyle().ItemSpacing.x;
                    ImGui::SameLine();
                    if (ImGui::Button("-##z", ImVec2(btn_width,0))) { pos_z -= 0.001f; pos_changed = true; }
                    ImGui::SameLine();
                    if(ImGui::Button("+##z", ImVec2(btn_width,0))) { pos_z += 0.001f; pos_changed = true; }
                }
                if (pos_changed) { flare.offsetz = pos_z; }
                ImGui::PopID();
            }
            
            if (ImGui::Button(_LC("FlareUtil", "Reset##pos")))
            {
                flare.offsetx = m_spawn_values.offset_x;
                flare.offsety = m_spawn_values.offset_y;
                flare.offsetz = m_spawn_values.offset_z;
            }

            // Size editor with fine adjustment buttons
            ImGui::Text(_LC("FlareUtil", "Size:"));
            float size = flare.size;
            if (ImGui::SliderFloat("##size", &size, 0.1f, 10.0f, "%.3f"))
            {
                flare.size = size;
            }
            {
                float btn_width = 25.0f;
                float spacing = ImGui::GetStyle().ItemSpacing.x;
                ImGui::SameLine();
                if (ImGui::Button("-##size", ImVec2(btn_width,0))) { size -= 0.001f; flare.size = size; }
                ImGui::SameLine();
                if(ImGui::Button("+##size", ImVec2(btn_width,0))) { size += 0.001f; flare.size = size; }
            }
            if (ImGui::Button(_LC("FlareUtil", "Reset##size")))
            {
                flare.size = m_spawn_values.size;
            }

            ImGui::Separator();

            // Additional properties
            //ImGui::Text(_LC("FlareUtil", "Material name: %s"), flare2.material_name.c_str());
            ImGui::Text(_LC("FlareUtil", "Blink delay: %d ms"), flare.blinkdelay > 0 ? (int)(flare.blinkdelay * 1000) : 0);

            ImGui::EndGroup();
        }
    }
    ImGui::End();
}

const char* FlareUtil::GetFlareTypeDesc(FlareType type)
{
    switch (type)
    {
    case FlareType::HEADLIGHT:    return "low beam";
    case FlareType::HIGH_BEAM:     return "high beam";
    case FlareType::SIDELIGHT:    return "sidelight";
    case FlareType::FOG_LIGHT:     return "fog light";
    case FlareType::BRAKE_LIGHT:        return "brake light";
    case FlareType::TAIL_LIGHT:    return "tail light";
    case FlareType::BLINKER_LEFT: return "left blinker";
    case FlareType::BLINKER_RIGHT:return "right blinker";
    case FlareType::REVERSE_LIGHT:      return "reverse light";
    case FlareType::DASHBOARD:    return "dashboard";
    case FlareType::USER:         return "user controlled";
    default:                      return "unknown";
    }
}
