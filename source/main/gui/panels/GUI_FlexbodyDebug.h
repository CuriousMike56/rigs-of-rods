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

#pragma once

#include "Application.h"
#include "SimData.h"
#include <OgreVector3.h>

namespace RoR {
namespace GUI {

/// Flexbody and prop diagnostic
class FlexbodyDebug
{
public:
    bool IsVisible() const { return m_is_visible; }
    bool IsHovered() const { return m_is_hovered; }
    void SetVisible(bool value) { m_is_visible = value; m_is_hovered = false; }
    void Draw();
    bool IsHideOtherElementsModeActive() const { return hide_other_elements; }

    void AnalyzeFlexbodies(); //!< populates the combobox
    void DrawDebugView(FlexBody* flexbody, Prop* prop, NodeNum_t node_ref, NodeNum_t node_x, NodeNum_t node_y);

    void EditPropPosition(int prop_id, Ogre::Vector3 offset);
    void EditPropRotation(int prop_id, Ogre::Vector3 rotation);

private:

    void UpdateVisibility();
    void DrawMemoryOrderGraph(FlexBody* flexbody);
    bool DrawOffsetRotationEdit(Ogre::Vector3& offset, Ogre::Quaternion& rotation); // Changed param type
    
    void DrawLocatorsTable(FlexBody* flexbody, bool& locators_visible);
    void DrawMeshInfo(FlexBody* flexbody); 
    void DrawMeshInfo(Prop* prop);

    bool DrawFlexbodyOffsetRotationEdit(FlexBody* flexbody);
    bool DrawPropOffsetRotationEdit(Prop* prop);

    // For editing offset/rotation
    struct ElementTransform
    {
        Ogre::Vector3 offset = Ogre::Vector3::ZERO;
        Ogre::Quaternion rotation = Ogre::Quaternion::IDENTITY;
        bool initialized = false;
    };
    std::vector<ElementTransform> m_element_transforms;

    bool m_is_editing = false; // Track if we're currently editing
    Ogre::Vector3 m_initial_local_pos;     // Store initial local position 
    Ogre::Quaternion m_initial_local_rot;  // Store initial local rotation

    // Add these member variables back
    Ogre::Vector3 m_edit_offset = Ogre::Vector3::ZERO;
    Ogre::Quaternion m_edit_rotation = Ogre::Quaternion::IDENTITY;
    bool m_offset_rot_changed = false;
    bool m_values_initialized = false;

    // Display options
    bool draw_mesh_wireframe = false;
    bool show_base_nodes = false;
    bool show_forset_nodes = false;
    bool show_vertices = false;
    bool hide_other_elements = false;
    std::vector<bool> show_locator;

    // Selection variables
    int m_selected_tab = 0;        // 0 = Flexbodies, 1 = Props 
    int m_selected_flexbody = -1;  // Index in flexbodies vector
    int m_selected_prop = -1;      // Index in props vector

    // Window state
    bool m_is_visible = false;
    bool m_is_hovered = false;

    // For prop rotation editing, stores raw Euler angles read from truck file
    Ogre::Vector3 m_raw_angles = Ogre::Vector3::ZERO;
};

} // namespace GUI
} // namespace RoR
