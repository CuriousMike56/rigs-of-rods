/// \title Engine tweak + Clutch diag script
/// \brief Shows config params and state of engine simulation
/// \author ohlidalp 2024-2025, see https://github.com/RigsOfRods/rigs-of-rods/pull/3177

// Window [X] button handler
#include "imgui_utils.as"
imgui_utils::CloseWindowPrompt closeBtnHandler;

// #region Plots

vector2 cfgPlotSize(0.f, 45.f); // 0=autoresize

// assume 60FPS = circa 3 sec
const int MAX_SAMPLES = 3*60;
array<float> clutchBuf(MAX_SAMPLES, 0.f);
array<float> rpmBuf(MAX_SAMPLES, 0.f);
array<float> clutchTorqueBuf(MAX_SAMPLES, 0.f);

void updateFloatBuf(array<float>@ buf, float f)
{
    buf.removeAt(0);
    buf.insertLast(f); 
}

void updateEnginePlotBuffers(EngineClass@ engine)
{
    // Engine state values
    updateFloatBuf(clutchBuf, engine.getClutch());
    updateFloatBuf(rpmBuf, engine.getRPM());
    updateFloatBuf(clutchTorqueBuf, engine.getTorque());
    
}

// #endregion

// #region frameStep
CVarClass@  g_mp_state = console.cVarFind("mp_state"); // 0=disabled, 1=connecting, 2=connected, see MpState in Application.h
void frameStep(float dt)
{
    
    if (ImGui::Begin("Engine Tool", closeBtnHandler.windowOpen, 0))
    {
        // Draw the "Terminate this script?" prompt on the top (if not disabled by config).
        closeBtnHandler.draw();
        
        if (g_mp_state.getInt() == 2)
        {
            ImGui::Text("* * * This tool does not work in multiplayer! * * *");
        }
        
        // force minimum width
        ImGui::Dummy(vector2(250, 1));
        
        BeamClass@ playerVehicle = game.getCurrentTruck();
        if (@playerVehicle == null)
        {
            ImGui::Text("You are on foot.");
        }
        else
        {
            EngineClass@ engine = playerVehicle.getEngine();
            if (@engine == null)
            {
                ImGui::Text("Your vehicle doesn't have an engine");
            }
            else
            {
                updateEnginePlotBuffers(engine);
                
                drawEngineDiagUI(playerVehicle, engine);
            }
        }
        
        ImGui::End();
    }
}
// #endregion

// #region UI drawing helpers
void drawTableRow(string key, float val)
{
    ImGui::TextDisabled(key); ImGui::NextColumn(); ImGui::Text(formatFloat(val, "", 0, 3)); ImGui::NextColumn();
}

void drawTableRow(string key, int val)
{
    ImGui::TextDisabled(key); ImGui::NextColumn(); ImGui::Text(''+val); ImGui::NextColumn();
}

void drawTableRow(string key, bool val)
{
    ImGui::TextDisabled(key); ImGui::NextColumn(); ImGui::Text(val ? 'true' : 'false'); ImGui::NextColumn();
}

void drawTableRowPlot(string key, array<float>@ buf, float rangeMin, float rangeMax)
{
    ImGui::TextDisabled(key);
    ImGui::NextColumn(); 
    plotFloatBuf(buf, rangeMin, rangeMax); 
    ImGui::NextColumn();  
}

void plotFloatBuf(array<float>@ buf, float rangeMin, float rangeMax)
{
    //DOC: "void PlotLines(const string&in label, array<float>&in values, int values_count, int values_offset = 0, const string&in overlay_text = string(), float scale_min = FLT_MAX, float scale_max = FLT_MAX, vector2 graph_size = vector2(0,0))",
    ImGui::PlotLines("", buf, MAX_SAMPLES, 0, "", rangeMin, rangeMax, cfgPlotSize);
    ImGui::SameLine();
    float val = buf[buf.length()-1];
    ImGui::Text(formatFloat(val, "", 0, 3));     
}

// Attributes can be edited realtime (on every keystroke) or using the [Focus] button.
// Focused editing means all other inputs are disabled and [Apply/Reset] buttons must be used.
ActorSimAttr gFocusedEditingAttr = ACTORSIMATTR_NONE;
float gFocusedEditingValue = 0.f;
float cfgAttrInputboxWidth = 125.f;
color cfgAttrUnfocusedBgColor = color(0.14, 0.14, 0.14, 1.0);
void drawAttrInputRow(BeamClass@ actor, ActorSimAttr attr, string label)
{
    ImGui::PushID(label);
    ImGui::TextDisabled(label);
    ImGui::NextColumn();
    if (gFocusedEditingAttr == ACTORSIMATTR_NONE)
    {
        // Focused editing inactive - draw [Focus] button
        float val = actor.getSimAttribute(attr);
        ImGui::SetNextItemWidth(cfgAttrInputboxWidth);
        if (ImGui::InputFloat("", val))
        {
            actor.setSimAttribute(attr, val);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Focus"))
        {
            gFocusedEditingAttr = attr;
            gFocusedEditingValue = val;
        }
    }
    else if (gFocusedEditingAttr == attr)
    {
        // This attr is focused - draw [Apply/Reset] buttons
        ImGui::SetNextItemWidth(cfgAttrInputboxWidth);
        ImGui::InputFloat("##"+label, gFocusedEditingValue);
        ImGui::SameLine();
        if (ImGui::Button("Apply"))
        {
            actor.setSimAttribute(attr, gFocusedEditingValue);
            gFocusedEditingAttr = ACTORSIMATTR_NONE;
            gFocusedEditingValue = 0.f;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Reset"))
        {
            gFocusedEditingAttr = ACTORSIMATTR_NONE;
            gFocusedEditingValue = 0.f;
        }        
    }
    else
    {
        // Some other attr is focused - just draw a label padded to size of inputbox.
        string valStr = "" + actor.getSimAttribute(attr);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, cfgAttrUnfocusedBgColor);
        ImGui::BeginChildFrame(uint(attr), vector2(cfgAttrInputboxWidth, ImGui::GetTextLineHeight()) + 3 * 2);
        ImGui::Text(valStr);
        ImGui::EndChildFrame(); // Must be called either way - inconsistent with other End*** funcs.
        ImGui::PopStyleColor(); // FrameBg
    }
    ImGui::NextColumn();
    ImGui::PopID(); // label
}

void drawAttributesCommonHelp()
{
    if (ImGui::CollapsingHeader("How to read and edit attributes:"))
    {
        ImGui::TextDisabled("Each value is displayed twice (for debugging) from different sources:");
        ImGui::TextDisabled("1. the 'get***' value is what EngineClass object reports via `get***()` functions.");
        ImGui::TextDisabled("2. the UPPERCASE value is what ActorClass object reports via `getSimAttribute()` function.");
        ImGui::TextDisabled("There are exceptions, a few values have multiple EngineClass getters or lack Attribute handle");
        ImGui::Dummy(vector2(10,10));
        ImGui::TextDisabled("Attributes can be edited realtime (on every keystroke) or using the [Focus] button.");
        ImGui::TextDisabled("Focused editing means all other inputs are disabled and [Apply/Reset] buttons must be used.");
        ImGui::Separator();
        ImGui::Separator();
    }
}

//#endregion

// #region Main window - 'engine' args tab
void drawEngineAttributesTab(BeamClass@ actor, EngineClass@ engine)
{
    drawAttributesCommonHelp();
    
    ImGui::Columns(2);
    
    drawTableRow("getShiftDownRPM", engine.getShiftDownRPM());
    drawAttrInputRow(actor, ACTORSIMATTR_ENGINE_SHIFTDOWN_RPM, "SHIFTDOWN_RPM");
    ImGui::Separator();
    drawTableRow("getShiftUpRPM", engine.getShiftUpRPM());
    drawAttrInputRow(actor, ACTORSIMATTR_ENGINE_SHIFTUP_RPM, "SHIFTUP_RPM");
    ImGui::Separator();
    drawTableRow("getEngineTorque", engine.getEngineTorque());
    drawAttrInputRow(actor, ACTORSIMATTR_ENGINE_TORQUE, "TORQUE");
    ImGui::Separator();
    drawTableRow("getDiffRatio", engine.getDiffRatio());
    drawAttrInputRow(actor, ACTORSIMATTR_ENGINE_DIFF_RATIO, "DIFF_RATIO");
    ImGui::Separator();    
    
    //float getGearRatio(int) const"
    //int getNumGears() const
    //int getNumGearsRanges() const
    ImGui::TextDisabled("gears (rev, neutral, "+engine.getNumGears()+" forward)");
    ImGui::NextColumn();
    for (int i = -1; i <= engine.getNumGears(); i++)
    {
        ImGui::NextColumn();
        ImGui::TextDisabled(">");
        ImGui::NextColumn();
        ImGui::Text(formatFloat(engine.getGearRatio(i), "", 0, 3));
    }
    ImGui::Columns(1);
}
//#endregion

//  #region Main window - 'engoption' args tab
void drawEngoptionAttributesTab(BeamClass@ actor, EngineClass@ engine)
{
    drawAttributesCommonHelp();
    
    ImGui::Columns(2);
    
    drawTableRow("getEngineInertia", engine.getEngineInertia());
    drawAttrInputRow(actor, ACTORSIMATTR_ENGOPTION_ENGINE_INERTIA, "ENGINE_INERTIA");
    ImGui::Separator();
    drawTableRow("getEngineType", engine.getEngineType()); //uint8
    drawTableRow("isElectric", engine.isElectric()); // bool
    drawTableRow("hasAir", engine.hasAir()); // bool
    drawTableRow("hasTurbo", engine.hasTurbo()); // bool
    drawAttrInputRow(actor, ACTORSIMATTR_ENGOPTION_ENGINE_TYPE, "ENGINE_TYPE");
    ImGui::Separator();
    drawTableRow("getClutchForce", engine.getClutchForce());
    drawAttrInputRow(actor,  ACTORSIMATTR_ENGOPTION_CLUTCH_FORCE, "CLUTCH_FORCE");
    ImGui::Separator();
    drawTableRow("getShiftTime", engine.getShiftTime());
    drawAttrInputRow(actor,  ACTORSIMATTR_ENGOPTION_SHIFT_TIME, "SHIFT_TIME");
    ImGui::Separator();
    drawTableRow("getClutchTime", engine.getClutchTime());
    drawAttrInputRow(actor,  ACTORSIMATTR_ENGOPTION_CLUTCH_TIME, "CLUTCH_TIME");
    ImGui::Separator();
    drawTableRow("getPostShiftTime", engine.getPostShiftTime());
    drawAttrInputRow(actor,  ACTORSIMATTR_ENGOPTION_POST_SHIFT_TIME, "POST_SHIFT_TIME");
    ImGui::Separator();
    
    drawTableRow("getStallRPM", engine.getStallRPM());
    drawAttrInputRow(actor,  ACTORSIMATTR_ENGOPTION_STALL_RPM, "STALL_RPM");
    ImGui::Separator();
    drawTableRow("getIdleRPM", engine.getIdleRPM());
    drawAttrInputRow(actor,  ACTORSIMATTR_ENGOPTION_IDLE_RPM, "IDLE_RPM");
    ImGui::Separator();
    drawTableRow("getMaxIdleMixture", engine.getMaxIdleMixture());
    drawAttrInputRow(actor,  ACTORSIMATTR_ENGOPTION_MAX_IDLE_MIXTURE, "MAX_IDLE_MIXTURE");
    ImGui::Separator();
    drawTableRow("getMinIdleMixture", engine.getMinIdleMixture());
    drawAttrInputRow(actor,  ACTORSIMATTR_ENGOPTION_MIN_IDLE_MIXTURE, "MIN_IDLE_MIXTURE");
    ImGui::Separator();
    drawTableRow("getBrakingTorque", engine.getBrakingTorque());
    drawAttrInputRow(actor,  ACTORSIMATTR_ENGOPTION_BRAKING_TORQUE, "BRAKING_TORQUE");
    ImGui::Separator();
    
    ImGui::Columns(1);
}
// #endregion

// #region Main window - simulation state tab
void drawSimulationStateTab(EngineClass@ engine)
{
    ImGui::Columns(2);
    
    drawTableRow("getAcc", engine.getAcc());
    drawTableRowPlot("getClutch (0.0 - 1.0)", clutchBuf, 0.f, 1.f);
    drawTableRow("getCrankFactor", engine.getCrankFactor());
    drawTableRowPlot("getRPM (0 - 10000)", rpmBuf, 0.f, 10000.f);
    drawTableRow("getSmoke", engine.getSmoke());
    float clutchTorquePlotMax = engine.getEngineTorque() * engine.getGearRatio(1) * 2.5f; // magic
    drawTableRowPlot("getTorque (0 - "+clutchTorquePlotMax+")", clutchTorqueBuf, 0.f, clutchTorquePlotMax);
    drawTableRow("getTurboPSI", engine.getTurboPSI());
    drawTableRow("getAutoMode", engine.getAutoMode());//SimGearboxMode 
    drawTableRow("getGear", engine.getGear());
    drawTableRow("getGearRange", engine.getGearRange());
    drawTableRow("isRunning", engine.isRunning()); // bool
    drawTableRow("hasContact", engine.hasContact()); //bool
    drawTableRow("getAutoShift", engine.getAutoShift());   //autoswitch   
    drawTableRow("getCurEngineTorque", engine.getCurEngineTorque());
    drawTableRow("getInputShaftRPM", engine.getInputShaftRPM());
    drawTableRow("getDriveRatio", engine.getDriveRatio());
    drawTableRow("getEnginePower", engine.getEnginePower());
    drawTableRow("getTurboPower", engine.getTurboPower());
    drawTableRow("getIdleMixture", engine.getIdleMixture());
    drawTableRow("getPrimeMixture", engine.getPrimeMixture());
    drawTableRow("getAccToHoldRPM", engine.getAccToHoldRPM());
    
    ImGui::Columns(1);   
}
// #endregion

// #region Main window
void drawEngineDiagUI(BeamClass@ actor, EngineClass@ engine)
{
    if (ImGui::BeginTabBar("engineDiagTabs"))
    {
        if (ImGui::BeginTabItem('engine args'))
        {
            drawEngineAttributesTab(actor, engine);
                        ImGui::EndTabItem();
        }
        
        
        if (ImGui::BeginTabItem("engoption args"))
        {
            drawEngoptionAttributesTab(actor, engine);
                        ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem('state'))
        {            drawSimulationStateTab(engine);
            
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
}

// #endregion

float fmax(float a, float b)
{
    return (a > b) ? a : b;
}

float fabs(float a)
{
    return a > 0.f ? a : -a;
}

float fclamp(float val, float minv, float maxv)
{
    return val < minv ? minv : (val > maxv) ? maxv : val;
}

float fexp(float val)
{
    const float eConstant = 2.71828183;
    return pow(eConstant, val);
}

