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
#include "CameraManager.h"
#include "GameContext.h"
#include "GUIManager.h"
#include "GUIUtils.h"
#include "Language.h"
#include "OgreCamera.h" 
#include "GfxActor.h" 
#include "Utils.h"
#include "DashBoardManager.h"

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


void FlareUtil::SetVisible(bool v)
{
    m_is_visible = v;
    m_show_base_nodes = false; // Reset debug view when window is opened/closed
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
            m_spawn_values.noderef = flare.noderef;
            m_spawn_values.nodex = flare.nodex;
            m_spawn_values.nodey = flare.nodey;
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

    ImGui::SetNextWindowSize(ImVec2(1000, 600), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPosCenter(ImGuiCond_FirstUseEver);

    if (ImGui::Begin(_LC("FlareUtil", "Flare Utility (CM)"), &m_is_visible))
    {
        if (!m_actor)
        {
            ImGui::Text("%s", _LC("FlareUtil", "You are on foot."));
            ImGui::End();
            return;
        }

        if (m_actor->ar_flares.size() == 0)
        {
            ImGui::Text("%s", _LC("FlareUtil", "This vehicle has no flares."));
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
                m_spawn_values.noderef = flare.noderef;
                m_spawn_values.nodex = flare.nodex;
                m_spawn_values.nodey = flare.nodey;
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // Flare properties
        if (m_selected_flare < m_actor->ar_flares.size())
        {
            flare_t& flare = m_actor->ar_flares[m_selected_flare];
            ImGui::BeginGroup();

            // Truck file format line for easy copy-paste
            ImGui::TextWrapped("Truck file format line:");

            // Build lines for all flares
            std::string truck_lines = "flares2\n";
            truck_lines += ";RefNode, X, Y, OffsetX, OffsetY, OffsetZ, Type, ControlNumber, BlinkDelay, size MaterialName\n";

            // Helper function to format flare line
            auto format_flare_line = [&](const flare_t& f) {
                std::string mat_name = f.material_name.empty() ? "default" : f.material_name;
                return fmt::format("{}, {}, {}, {:.3f}, {:.3f}, {:.3f}, {}, {}, {}, {:.3f} {}\n",
                    f.noderef, f.nodex, f.nodey, f.offsetx, f.offsety, f.offsetz,
                    (char)f.fl_type, 
                    (f.fl_type == FlareType::DASHBOARD) ? 
                        m_actor->ar_dashboard->getLinkNameForID((DashData)f.dashboard_link) :
                        std::to_string(f.controlnumber + 1),
                    (int)(f.blinkdelay * 1000), f.size, mat_name);
            };

            // Helper to add type section
            auto add_type_section = [&](const char* comment, FlareType type) {
                bool section_added = false;
                for (const flare_t& f : m_actor->ar_flares) {
                    if (f.fl_type == type) {
                        if (!section_added) {
                            truck_lines += fmt::format(";{}\n", comment);
                            section_added = true;
                        }
                        truck_lines += format_flare_line(f);
                    }
                }
            };

            // Add sections in order
            add_type_section("low beams", FlareType::HEADLIGHT);
            add_type_section("high beams", FlareType::HIGH_BEAM);
            add_type_section("turn signals left", FlareType::BLINKER_LEFT);
            add_type_section("turn signals right", FlareType::BLINKER_RIGHT);
            add_type_section("fog lights", FlareType::FOG_LIGHT);
            add_type_section("brake lights", FlareType::BRAKE_LIGHT);
            add_type_section("tail lights", FlareType::TAIL_LIGHT);
            add_type_section("reverse lights", FlareType::REVERSE_LIGHT);
            add_type_section("dashboard links", FlareType::DASHBOARD);
            
            // User flares last
            bool user_section_added = false;
            for (const flare_t& f : m_actor->ar_flares) {
                if (f.fl_type == FlareType::USER) {
                    if (!user_section_added) {
                        truck_lines += ";user controlled\n";
                        user_section_added = true;
                    }
                    truck_lines += format_flare_line(f);
                }
            }

            // Create child window for scrolling
            static float text_box_height = ImGui::GetTextLineHeight() * 8;
            ImGui::BeginChild("truck_lines", ImVec2(-1, text_box_height), true);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

            // Split lines and make them selectable
            std::istringstream lines(truck_lines);
            std::string line;
            int line_number = 0;
            bool header_done = false;
            
            while (std::getline(lines, line))
            {
                // Skip header lines
                if (!header_done) 
                {
                    if (line.find("flares2") != std::string::npos || 
                        line.find(";RefNode") != std::string::npos)
                    {
                        ImGui::TextUnformatted(line.c_str());
                        continue;
                    }
                    header_done = true;
                }

                // Skip comment lines
                if (line.empty() || line[0] == ';')
                {
                    ImGui::TextUnformatted(line.c_str());
                    continue;
                }

                // Make actual flare lines selectable
                if (ImGui::Selectable(line.c_str(), false))
                {
                    // Try to match line with a flare
                    for (size_t i = 0; i < m_actor->ar_flares.size(); i++)
                    {
                        const flare_t& f = m_actor->ar_flares[i];
                        std::string mat_name = f.material_name.empty() ? "default" : f.material_name;
                        std::string flare_line = fmt::format("{}, {}, {}, {:.3f}, {:.3f}, {:.3f}, {}, {}, {}, {:.3f} {}",
                            f.noderef, f.nodex, f.nodey, f.offsetx, f.offsety, f.offsetz,
                            (char)f.fl_type,
                            (f.fl_type == FlareType::DASHBOARD) ?
                                m_actor->ar_dashboard->getLinkNameForID((DashData)f.dashboard_link) :
                                std::to_string(f.controlnumber + 1),
                            (int)(f.blinkdelay * 1000), f.size, mat_name);

                        if (line == flare_line)
                        {
                            m_selected_flare = i;
                            // Store original values
                            m_spawn_values.offset_x = f.offsetx;
                            m_spawn_values.offset_y = f.offsety;
                            m_spawn_values.offset_z = f.offsetz;
                            m_spawn_values.size = f.size;
                            m_spawn_values.noderef = f.noderef;
                            m_spawn_values.nodex = f.nodex;
                            m_spawn_values.nodey = f.nodey;
                            break;
                        }
                    }
                }
            }

            ImGui::PopStyleColor();
            ImGui::EndChild();

            // Add resize handle
            static const float min_height = ImGui::GetTextLineHeight() * 6;
            static const float max_height = ImGui::GetTextLineHeight() * 30;
            ImGui::InvisibleButton("resize_handle", ImVec2(-1, 3));
            if (ImGui::IsItemActive())
            {
                text_box_height += ImGui::GetIO().MouseDelta.y;
                text_box_height = std::max(min_height, std::min(text_box_height, max_height));
            }
            if (ImGui::IsItemHovered())
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);

            if (ImGui::Button(_LC("FlareUtil", "Copy to clipboard")))
            {
                ImGui::SetClipboardText(truck_lines.c_str());
            }

            ImGui::Separator();

            // Display active flare line
            ImGui::Text(_LC("FlareUtil", "Active flare controls:\n"));
            ImGui::Text(_LC("FlareUtil", "Type: %c (%s)"), (char)flare.fl_type, GetFlareTypeDesc(flare.fl_type));

            // Show truck file line for active flare in a selectable text box
            std::string mat_name = flare.material_name.empty() ? "default" : flare.material_name;
            std::string active_line = fmt::format("{}, {}, {}, {:.3f}, {:.3f}, {:.3f}, {}, {}, {}, {:.3f} {}",
                flare.noderef,
                flare.nodex,
                flare.nodey,
                flare.offsetx,
                flare.offsety,
                flare.offsetz,
                (char)flare.fl_type,
                (flare.fl_type == FlareType::DASHBOARD) ? 
                    m_actor->ar_dashboard->getLinkNameForID((DashData)m_actor->ar_flares[m_selected_flare].dashboard_link) :
                    std::to_string(flare.controlnumber + 1),
                (int)(flare.blinkdelay * 1000),
                flare.size,
                mat_name);

            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
            ImGui::InputText("##activeline", (char*)active_line.c_str(), 
                active_line.length(), ImGuiInputTextFlags_ReadOnly);
            ImGui::PopStyleColor();

            if (ImGui::Button(_LC("FlareUtil", "Copy active")))
            {
                ImGui::SetClipboardText(active_line.c_str());
            }
            
            // Node editors
            {
                float w = ImGui::CalcTextSize("000").x + ImGui::GetStyle().FramePadding.x * 4;
                ImGui::PushItemWidth(w);
                int noderef = flare.noderef;
                if (ImGui::InputInt(fmt::format("Ref node (spawn: {})", m_spawn_values.noderef).c_str(), 
                    &noderef, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    noderef = std::max(0, std::min(noderef, (int)m_actor->ar_num_nodes - 1));
                    flare.noderef = noderef;
                }
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button("Reset##ref")) { flare.noderef = m_spawn_values.noderef; }
            }
            {
                float w = ImGui::CalcTextSize("000").x + ImGui::GetStyle().FramePadding.x * 4;
                ImGui::PushItemWidth(w);
                int nodex = flare.nodex;
                if (ImGui::InputInt(fmt::format("Node X (spawn: {})", m_spawn_values.nodex).c_str(),
                    &nodex, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    nodex = std::max(0, std::min(nodex, (int)m_actor->ar_num_nodes - 1));
                    flare.nodex = nodex;
                }
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button("Reset##nodex")) { flare.nodex = m_spawn_values.nodex; }
            }
            {
                float w = ImGui::CalcTextSize("000").x + ImGui::GetStyle().FramePadding.x * 4;
                ImGui::PushItemWidth(w);
                int nodey = flare.nodey;
                if (ImGui::InputInt(fmt::format("Node Y (spawn: {})", m_spawn_values.nodey).c_str(),
                    &nodey, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    nodey = std::max(0, std::min(nodey, (int)m_actor->ar_num_nodes - 1));
                    flare.nodey = nodey;
                }
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::Button("Reset##nodey")) { flare.nodey = m_spawn_values.nodey; }
            }

            ImGui::Checkbox("Show##base", &m_show_base_nodes);

            ImGui::Separator();

            // Position editor with fine adjustment buttons
            ImGui::Text(_LC("FlareUtil", "Position offset:"));
            float w = ImGui::GetContentRegionAvail().x * 0.6f;
            {
                // X axis
                ImGui::PushID("PosX");
                ImGui::PushItemWidth(w);
                float pos_x = flare.offsetx;
                bool pos_changed = ImGui::SliderFloat("X", &pos_x, -10.0f, 10.0f, "%.3f");
                ImGui::PopItemWidth();
                if (ImGui::IsItemHovered() && !ImGui::IsItemActive() && !ImGui::IsItemClicked())
                {
                    ImGui::SetTooltip("CTRL + Click for manual input");
                }
                {
                    float btn_width = 25.0f;
                    float spacing = ImGui::GetStyle().ItemSpacing.x;
                    ImGui::SameLine();
                    ImGui::PushButtonRepeat(true);
                    if (ImGui::Button("-##x", ImVec2(btn_width,0))) 
                    { 
                        float step = 0.001f;
                        if (ImGui::GetIO().KeyCtrl)
                            step = 0.010f;
                        else if (ImGui::GetIO().KeyShift)
                            step = 0.100f;
                        pos_x -= step;
                        pos_changed = true;
                    }
                    ImGui::PopButtonRepeat();
                    if (ImGui::IsItemHovered() && !ImGui::IsItemActive() && !ImGui::IsItemClicked())
                    {
                        ImGui::BeginTooltip();
                        ImGui::Text("Click/Hold: 0.001 adjustment");
                        ImGui::Text("CTRL + Click/Hold: 0.010 adjustment");
                        ImGui::Text("SHIFT + Click/Hold: 0.100 adjustment");
                        ImGui::EndTooltip();
                    }
                    ImGui::SameLine();
                    ImGui::PushButtonRepeat(true);
                    if(ImGui::Button("+##x", ImVec2(btn_width,0)))
                    { 
                        float step = 0.001f;
                        if (ImGui::GetIO().KeyCtrl)
                            step = 0.010f;
                        else if (ImGui::GetIO().KeyShift)
                            step = 0.100f;
                        pos_x += step;
                        pos_changed = true;
                    }
                    ImGui::PopButtonRepeat();
                    if (ImGui::IsItemHovered() && !ImGui::IsItemActive() && !ImGui::IsItemClicked())
                    {
                        ImGui::BeginTooltip();
                        ImGui::Text("Click/Hold: 0.001 adjustment");
                        ImGui::Text("CTRL + Click/Hold: 0.010 adjustment");
                        ImGui::Text("SHIFT + Click/Hold: 0.100 adjustment");
                        ImGui::EndTooltip();
                    }
                }
                if (pos_changed) { flare.offsetx = pos_x; }
                ImGui::PopID();
            }
            {
                // Y axis  
                ImGui::PushID("PosY");
                ImGui::PushItemWidth(w);
                float pos_y = flare.offsety;
                bool pos_changed = ImGui::SliderFloat("Y", &pos_y, -10.0f, 10.0f, "%.3f");
                ImGui::PopItemWidth();
                if (ImGui::IsItemHovered() && !ImGui::IsItemActive() && !ImGui::IsItemClicked())
                {
                    ImGui::SetTooltip("CTRL + Click for manual input");
                }
                {
                    float btn_width = 25.0f;
                    float spacing = ImGui::GetStyle().ItemSpacing.x;
                    ImGui::SameLine();
                    ImGui::PushButtonRepeat(true);
                    if (ImGui::Button("-##y", ImVec2(btn_width,0)))
                    { 
                        float step = 0.001f;
                        if (ImGui::GetIO().KeyCtrl)
                            step = 0.010f;
                        else if (ImGui::GetIO().KeyShift)
                            step = 0.100f;
                        pos_y -= step;
                        pos_changed = true;
                    }
                    ImGui::PopButtonRepeat();
                    if (ImGui::IsItemHovered() && !ImGui::IsItemActive() && !ImGui::IsItemClicked())
                    {
                        ImGui::BeginTooltip();
                        ImGui::Text("Click/Hold: 0.001 adjustment");
                        ImGui::Text("CTRL + Click/Hold: 0.010 adjustment");
                        ImGui::Text("SHIFT + Click/Hold: 0.100 adjustment");
                        ImGui::EndTooltip();
                    }
                    ImGui::SameLine();
                    ImGui::PushButtonRepeat(true);
                    if(ImGui::Button("+##y", ImVec2(btn_width,0)))
                    { 
                        float step = 0.001f;
                        if (ImGui::GetIO().KeyCtrl)
                            step = 0.010f;
                        else if (ImGui::GetIO().KeyShift)
                            step = 0.100f;
                        pos_y += step;
                        pos_changed = true;
                    }
                    ImGui::PopButtonRepeat();
                    if (ImGui::IsItemHovered() && !ImGui::IsItemActive() && !ImGui::IsItemClicked())
                    {
                        ImGui::BeginTooltip();
                        ImGui::Text("Click/Hold: 0.001 adjustment");
                        ImGui::Text("CTRL + Click/Hold: 0.010 adjustment");
                        ImGui::Text("SHIFT + Click/Hold: 0.100 adjustment");
                        ImGui::EndTooltip();
                    }
                }
                if (pos_changed) { flare.offsety = pos_y; }
                ImGui::PopID();
            }
            {
                // Z axis
                ImGui::PushID("PosZ");
                ImGui::PushItemWidth(w);
                float pos_z = flare.offsetz;
                bool pos_changed = ImGui::SliderFloat("Z", &pos_z, -10.0f, 10.0f, "%.3f");
                ImGui::PopItemWidth();
                if (ImGui::IsItemHovered() && !ImGui::IsItemActive() && !ImGui::IsItemClicked())
                {
                    ImGui::SetTooltip("CTRL + Click for manual input");
                }
                {
                    float btn_width = 25.0f;
                    float spacing = ImGui::GetStyle().ItemSpacing.x;
                    ImGui::SameLine();
                    ImGui::PushButtonRepeat(true);
                    if (ImGui::Button("-##z", ImVec2(btn_width,0)))
                    { 
                        float step = 0.001f;
                        if (ImGui::GetIO().KeyCtrl)
                            step = 0.010f;
                        else if (ImGui::GetIO().KeyShift)
                            step = 0.100f;
                        pos_z -= step;
                        pos_changed = true;
                    }
                    ImGui::PopButtonRepeat();
                    if (ImGui::IsItemHovered() && !ImGui::IsItemActive() && !ImGui::IsItemClicked())
                    {
                        ImGui::BeginTooltip();
                        ImGui::Text("Click/Hold: 0.001 adjustment");
                        ImGui::Text("CTRL + Click/Hold: 0.010 adjustment");
                        ImGui::Text("SHIFT + Click/Hold: 0.100 adjustment");
                        ImGui::EndTooltip();
                    }
                    ImGui::SameLine();
                    ImGui::PushButtonRepeat(true);
                    if(ImGui::Button("+##z", ImVec2(btn_width,0)))
                    { 
                        float step = 0.001f;
                        if (ImGui::GetIO().KeyCtrl)
                            step = 0.010f;
                        else if (ImGui::GetIO().KeyShift)
                            step = 0.100f;
                        pos_z += step;
                        pos_changed = true;
                    }
                    ImGui::PopButtonRepeat();
                    if (ImGui::IsItemHovered() && !ImGui::IsItemActive() && !ImGui::IsItemClicked())
                    {
                        ImGui::BeginTooltip();
                        ImGui::Text("Click/Hold: 0.001 adjustment");
                        ImGui::Text("CTRL + Click/Hold: 0.010 adjustment");
                        ImGui::Text("SHIFT + Click/Hold: 0.100 adjustment");
                        ImGui::EndTooltip();
                    }
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
            ImGui::PushItemWidth(w);
            if (ImGui::SliderFloat("##size", &size, 0.1f, 10.0f, "%.3f"))
            {
                flare.size = size;
            }
            ImGui::PopItemWidth();
            if (ImGui::IsItemHovered() && !ImGui::IsItemActive() && !ImGui::IsItemClicked())
            {
                ImGui::SetTooltip("CTRL + Click for manual input");
            }
            {
                float btn_width = 25.0f;
                float spacing = ImGui::GetStyle().ItemSpacing.x;
                ImGui::SameLine();
                ImGui::PushButtonRepeat(true);
                if (ImGui::Button("-##size", ImVec2(btn_width,0))) 
                { 
                    float step = 0.001f;
                    if (ImGui::GetIO().KeyCtrl)
                        step = 0.010f;
                    else if (ImGui::GetIO().KeyShift)
                        step = 0.100f;
                    size -= step;
                    flare.size = size;
                }
                ImGui::PopButtonRepeat();
                ImGui::SameLine();
                ImGui::PushButtonRepeat(true);
                if(ImGui::Button("+##size", ImVec2(btn_width,0)))
                { 
                    float step = 0.001f;
                    if (ImGui::GetIO().KeyCtrl)
                        step = 0.010f;
                    else if (ImGui::GetIO().KeyShift)
                        step = 0.100f;
                    size += step;
                    flare.size = size;
                }
                ImGui::PopButtonRepeat();
            }
            if (ImGui::IsItemHovered() && !ImGui::IsItemActive() && !ImGui::IsItemClicked())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Click/Hold: 0.001 adjustment");
                ImGui::Text("CTRL + Click/Hold: 0.010 adjustment");
                ImGui::Text("SHIFT + Click/Hold: 0.100 adjustment");
                ImGui::EndTooltip();
            }
            if (ImGui::Button(_LC("FlareUtil", "Reset##size")))
            {
                flare.size = m_spawn_values.size;
            }

            ImGui::Separator();

            // Additional properties

            if (flare.fl_type == FlareType::USER)
            {
                ImGui::Text(_LC("FlareUtil", "Control number: %d"), flare.controlnumber + 1);
            }
            if (flare.fl_type == FlareType::DASHBOARD)
            {
                ImGui::Text(_LC("FlareUtil", "Dashboard link: %s"), m_actor->ar_dashboard->getLinkNameForID((DashData)m_actor->ar_flares[m_selected_flare].dashboard_link));
            }

            ImGui::Text(_LC("FlareUtil", "Blink delay: %d ms"), flare.blinkdelay > 0 ? (int)(flare.blinkdelay * 1000) : 0);
            
            if (flare.material_name.empty())
            {
                flare.material_name = "default";
            }
            ImGui::Text(_LC("FlareUtil", "Material name: %s"), flare.material_name.c_str());

            ImGui::EndGroup();
        }
    }
    ImGui::End();

    if (m_show_base_nodes && m_selected_flare < m_actor->ar_flares.size())
    {
        DrawDebugView(&m_actor->ar_flares[m_selected_flare]);
    }
}

void FlareUtil::DrawDebugView(const flare_t* flare)
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
    Ogre::Vector3 ref_pos = world2screen.Convert(nodes[flare->noderef].AbsPosition);
    Ogre::Vector3 x_pos = world2screen.Convert(nodes[flare->nodex].AbsPosition);
    Ogre::Vector3 y_pos = world2screen.Convert(nodes[flare->nodey].AbsPosition);

    // Draw nodes
    drawlist->ChannelsSetCurrent(LAYER_NODES);
    if (ref_pos.z < 0.f) { drawlist->AddCircleFilled(ImVec2(ref_pos.x, ref_pos.y), BASENODE_RADIUS, BASENODE_COLOR); }
    if (x_pos.z < 0.f) { drawlist->AddCircleFilled(ImVec2(x_pos.x, x_pos.y), BASENODE_RADIUS, BASENODE_COLOR); }
    if (y_pos.z < 0.f) { drawlist->AddCircleFilled(ImVec2(y_pos.x, y_pos.y), BASENODE_RADIUS, BASENODE_COLOR); }

    // Draw connection beams
    drawlist->ChannelsSetCurrent(LAYER_BEAMS);
    if (ref_pos.z < 0.f)
    {
        if (x_pos.z < 0.f) { drawlist->AddLine(ImVec2(ref_pos.x, ref_pos.y), ImVec2(x_pos.x, x_pos.y), AXIS_Y_BEAM_COLOR, BLUE_BEAM_THICKNESS); }
        if (y_pos.z < 0.f) { drawlist->AddLine(ImVec2(ref_pos.x, ref_pos.y), ImVec2(y_pos.x, y_pos.y), AXIS_Z_BEAM_COLOR, BEAM_THICKNESS); }
    }

    // Draw node numbers
    drawlist->ChannelsSetCurrent(LAYER_TEXT);
    drawlist->AddText(ImVec2(ref_pos.x, ref_pos.y), NODE_TEXT_COLOR, fmt::format("{}", flare->noderef).c_str());
    drawlist->AddText(ImVec2(x_pos.x, x_pos.y), NODE_TEXT_COLOR, fmt::format("{}", flare->nodex).c_str());
    drawlist->AddText(ImVec2(y_pos.x, y_pos.y), NODE_TEXT_COLOR, fmt::format("{}", flare->nodey).c_str());

    drawlist->ChannelsMerge();
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
