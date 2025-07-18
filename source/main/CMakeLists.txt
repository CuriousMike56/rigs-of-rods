####################################################################################################
#  SOURCE FILES
####################################################################################################

set(SOURCE_FILES
        main.cpp
        Application.{h,cpp}
        ForwardDeclarations.h
        AppContext.{h,cpp}
        GameContext.{h,cpp}
        audio/MumbleIntegration.{h,cpp}
        audio/Sound.{h,cpp}
        audio/SoundManager.{h,cpp}
        audio/SoundScriptManager.{h,cpp}
        gameplay/AutoPilot.{h,cpp}
        gameplay/Character.{h,cpp}
        gameplay/CharacterFactory.{h,cpp}
        gameplay/ChatSystem.{h,cpp}
        gameplay/CruiseControl.cpp
        gameplay/Engine.{h,cpp}
        gameplay/Landusemap.{h,cpp}
        gameplay/RaceSystem.{h,cpp}
        gameplay/RepairMode.{h,cpp}
        gameplay/Replay.{h,cpp}
        gameplay/SceneMouse.{h,cpp}
        gameplay/ScriptEvents.h
        gameplay/TorqueCurve.{h,cpp}
        gameplay/TyrePressure.{h,cpp}
        gameplay/VehicleAI.{h,cpp}
        gfx/AdvancedScreen.h
        gfx/ColoredTextAreaOverlayElement.{h,cpp}
        gfx/ColoredTextAreaOverlayElementFactory.h
        gfx/DustPool.{h,cpp}
        gfx/EnvironmentMap.{h,cpp}
        gfx/GfxActor.{h,cpp}
        gfx/GfxData.{h,cpp}
        gfx/GfxScene.{h,cpp}
        gfx/HydraxWater.{h,cpp}
        gfx/IWater.h
        gfx/MovableText.{h,cpp}
        gfx/Renderdash.{h,cpp}
        gfx/ShadowManager.{h,cpp}
        gfx/SimBuffers.{h,cpp}
        gfx/Skidmark.{h,cpp}
        gfx/SkyManager.{h,cpp}
        gfx/SkyXManager.{h,cpp}
        gfx/SurveyMapTextureCreator.{h,cpp}
        gfx/Water.{h,cpp}
        gfx/camera/CameraManager.{h,cpp}
        gfx/camera/PerVehicleCameraContext.h
        gfx/hydrax/CfgFileManager.{h,cpp}
        gfx/hydrax/DecalsManager.{h,cpp}
        gfx/hydrax/Enums.{h,cpp}
        gfx/hydrax/FFT.{h,cpp}
        gfx/hydrax/GodRaysManager.{h,cpp}
        gfx/hydrax/GPUNormalMapManager.{h,cpp}
        gfx/hydrax/Help.{h,cpp}
        gfx/hydrax/Hydrax.{h,cpp}
        gfx/hydrax/Image.{h,cpp}
        gfx/hydrax/MaterialManager.{h,cpp}
        gfx/hydrax/Mesh.{h,cpp}
        gfx/hydrax/Module.{h,cpp}
        gfx/hydrax/Noise.{h,cpp}
        gfx/hydrax/Perlin.{h,cpp}
        gfx/hydrax/Prerequisites.{h,cpp}
        gfx/hydrax/PressurePoint.{h,cpp}
        gfx/hydrax/ProjectedGrid.{h,cpp}
        gfx/hydrax/RadialGrid.{h,cpp}
        gfx/hydrax/Real.{h,cpp}
        gfx/hydrax/RttManager.{h,cpp}
        gfx/hydrax/SimpleGrid.{h,cpp}
        gfx/hydrax/TextureManager.{h,cpp}
        gfx/hydrax/Wave.{h,cpp}
        gfx/particle/ExtinguishableFireAffector.{h,cpp}
        gfx/particle/ExtinguishableFireAffectorFactory.h
        gfx/particle/FireExtinguisherAffector.{h,cpp}
        gfx/particle/FireExtinguisherAffectorFactory.h
        gfx/particle/OgreParticleCustomParam.h
        gfx/particle/OgreShaderParticleRenderer.{h,cpp}
        gfx/skyx/AtmosphereManager.{h,cpp}
        gfx/skyx/BasicController.{h,cpp}
        gfx/skyx/CloudsManager.{h,cpp}
        gfx/skyx/ColorGradient.{h,cpp}
        gfx/skyx/Controller.h
        gfx/skyx/GPUManager.{h,cpp}
        gfx/skyx/MeshManager.{h,cpp}
        gfx/skyx/MoonManager.{h,cpp}
        gfx/skyx/Prerequisites.{h,cpp}
        gfx/skyx/SCfgFileManager.{h,cpp}
        gfx/skyx/SkyX.{h,cpp}
        gfx/skyx/VCloudsManager.{h,cpp}
        gfx/skyx/VClouds/DataManager.{h,cpp}
        gfx/skyx/VClouds/Ellipsoid.{h,cpp}
        gfx/skyx/VClouds/FastFakeRandom.{h,cpp}
        gfx/skyx/VClouds/GeometryBlock.{h,cpp}
        gfx/skyx/VClouds/GeometryManager.{h,cpp}
        gfx/skyx/VClouds/Lightning.{h,cpp}
        gfx/skyx/VClouds/LightningManager.{h,cpp}
        gfx/skyx/VClouds/VClouds.{h,cpp}
        gui/DashBoardManager.{h,cpp}
        gui/GUIManager.{h,cpp}
        gui/GUIUtils.{h,cpp}
        gui/OverlayWrapper.{h,cpp}
        gui/RTTLayer.{h,cpp}
        gui/imgui/imgui.{h,cpp}
        gui/imgui/imgui_demo.cpp
        gui/imgui/imgui_draw.cpp
        gui/imgui/imgui_widgets.cpp
        gui/imgui/OgreImGuiOverlay.{h,cpp}
        gui/imgui/OgreImGui.{h,cpp}
        gui/imgui/imconfig.h
        gui/imgui/imgui_internal.h
        gui/imgui/imstb_rectpack.h
        gui/imgui/imstb_textedit.h
        gui/imgui/imstb_truetype.h
        gui/panels/GUI_AngelScriptExamples.{h,cpp}
        gui/panels/GUI_CollisionsDebug.{h,cpp}
        gui/panels/GUI_ConsoleView.{h,cpp}
        gui/panels/GUI_ConsoleWindow.{h,cpp}
        gui/panels/GUI_DirectionArrow.{h,cpp}
        gui/panels/GUI_LoadingWindow.{h,cpp}
        gui/panels/GUI_FlexbodyDebug.{h,cpp}
        gui/panels/GUI_FrictionSettings.{h,cpp}
        gui/panels/GUI_TopMenubar.{h,cpp}
        gui/panels/GUI_TextureToolWindow.{h,cpp}
        gui/panels/GUI_RepositorySelector.{h,cpp}
        gui/panels/GUI_GameControls.{h,cpp}
        gui/panels/GUI_GameAbout.{h,cpp}
        gui/panels/GUI_GameChatBox.{h,cpp}
        gui/panels/GUI_GameMainMenu.{h,cpp}
        gui/panels/GUI_GameSettings.{h,cpp}
        gui/panels/GUI_MainSelector.{h,cpp}
        gui/panels/GUI_MessageBox.{h,cpp}
        gui/panels/GUI_MultiplayerSelector.{h,cpp}
        gui/panels/GUI_MultiplayerClientList.{h,cpp}
        gui/panels/GUI_NodeBeamUtils.{h,cpp}
        gui/panels/GUI_VehicleInfoTPanel.{h,cpp}
        gui/panels/GUI_ScriptMonitor.{h,cpp}
        gui/panels/GUI_SimPerfStats.{h,cpp}
        gui/panels/GUI_SurveyMap.{h,cpp}
        network/CurlHelpers.{h,cpp}
        network/DiscordRpc.{h,cpp}
        network/Network.{h,cpp}
        network/OutGauge.{h,cpp}
        network/RoRnet.h
        physics/Actor.{h,cpp}
        physics/ApproxMath.h
        physics/ActorExport.cpp
        physics/ActorForcesEuler.cpp
        physics/ActorManager.{h,cpp}
        physics/ActorSlideNode.cpp
        physics/ActorSpawner.{h,cpp}
        physics/ActorSpawnerFlow.cpp
        physics/CmdKeyInertia.{h,cpp}
        physics/Differentials.{h,cpp}
        physics/Savegame.cpp
        physics/SimConstants.h
        physics/SimData.{h,cpp}
        physics/SlideNode.{h,cpp}
        physics/air/AeroEngine.h
        physics/air/AirBrake.{h,cpp}
        physics/air/Airfoil.{h,cpp}
        physics/air/TurboJet.{h,cpp}
        physics/air/TurboProp.{h,cpp}
        physics/collision/CartesianToTriangleTransform.h
        physics/collision/Collisions.{h,cpp}
        physics/collision/DynamicCollisions.{h,cpp}
        physics/collision/PointColDetector.{h,cpp}
        physics/collision/Triangle.h
        physics/flex/Flexable.h
        physics/flex/FlexAirfoil.{h,cpp}
        physics/flex/FlexBody.{h,cpp}
        physics/flex/FlexFactory.{h,cpp}
        physics/flex/FlexMesh.{h,cpp}
        physics/flex/FlexMeshWheel.{h,cpp}
        physics/flex/FlexObj.{h,cpp}
        physics/flex/Locator_t.h
        physics/water/Buoyance.{h,cpp}
        physics/water/ScrewProp.{h,cpp}
        resources/CacheSystem.{h,cpp}
        resources/ContentManager.{h,cpp}
        resources/addonpart_fileformat/AddonPartFileFormat.{h,cpp}
        resources/otc_fileformat/OTCFileFormat.{h,cpp}
        resources/odef_fileformat/ODefFileFormat.{h,cpp}
        resources/rig_def_fileformat/RigDef_File.{h,cpp}
        resources/rig_def_fileformat/RigDef_Node.{h,cpp}
        resources/rig_def_fileformat/RigDef_Parser.{h,cpp}
        resources/rig_def_fileformat/RigDef_Prerequisites.h
        resources/rig_def_fileformat/RigDef_Regexes.h
        resources/rig_def_fileformat/RigDef_SequentialImporter.{h,cpp}
        resources/rig_def_fileformat/RigDef_Serializer.{h,cpp}
        resources/rig_def_fileformat/RigDef_Validator.{h,cpp}
        resources/skin_fileformat/SkinFileFormat.{h,cpp}
        resources/terrn2_fileformat/Terrn2FileFormat.{h,cpp}
        resources/tobj_fileformat/TObjFileFormat.{h,cpp}
        resources/tuneup_fileformat/TuneupFileFormat.{h,cpp}
        system/AppCommandLine.cpp
        system/AppConfig.cpp
        system/Console.{h,cpp}
        system/ConsoleCmd.{h,cpp}
        system/CVar.{h,cpp}
        terrain/OgreTerrainPSSMMaterialGenerator.{h,cpp}
        terrain/ProceduralManager.{h,cpp}
        terrain/ProceduralRoad.{h,cpp}
        terrain/SurveyMapEntity.h
        terrain/TerrainEditor.{h,cpp}
        terrain/TerrainGeometryManager.{h,cpp}
        terrain/Terrain.{h,cpp}
        terrain/TerrainObjectManager.{h,cpp}
        threadpool/ThreadPool.h
        utils/ConfigFile.{h,cpp}
        utils/ErrorUtils.{h,cpp}
        utils/ForceFeedback.{h,cpp}
        utils/GenericFileFormat.{h,cpp}
        utils/ImprovedConfigFile.h
        utils/InputEngine.{h,cpp}
        utils/InterThreadStoreVector.h
        utils/Language.{h,cpp}
        utils/MeshObject.{h,cpp}
        utils/PlatformUtils.{h,cpp}
        utils/SHA1.{h,cpp}
        utils/Utils.{h,cpp}
        utils/WriteTextToTexture.{h,cpp}
        utils/memory/RefCountingObject.h
        utils/memory/RefCountingObjectPtr.h
        )

if (ROR_USE_ANGELSCRIPT)
    list(APPEND SOURCE_FILES
            scripting/GameScript.{h,cpp}
            scripting/LocalStorage.{h,cpp}
            scripting/OgreScriptBuilder.{h,cpp}
            scripting/ScriptEngine.{h,cpp}
            scripting/ScriptUtils.h
            scripting/bindings/AngelScriptBindings.h
            scripting/bindings/ActorAngelscript.cpp
            scripting/bindings/CacheSystemAngelscript.cpp
            scripting/bindings/ConsoleAngelscript.cpp
            scripting/bindings/EngineAngelscript.cpp
            scripting/bindings/GameScriptAngelscript.cpp
            scripting/bindings/GenericFileFormatAngelscript.cpp
            scripting/bindings/ImGuiAngelscript.cpp
            scripting/bindings/InputEngineAngelscript.cpp
            scripting/bindings/LocalStorageAngelscript.cpp
            scripting/bindings/MsgQueueAngelscript.cpp
            scripting/bindings/OgreAngelscript.cpp
            scripting/bindings/ProceduralRoadAngelscript.cpp
            scripting/bindings/ScriptEventsAngelscript.cpp
            scripting/bindings/SoundScriptAngelscript.cpp
            scripting/bindings/TerrainAngelscript.cpp
            scripting/bindings/VehicleAiAngelscript.cpp
            )
endif ()

include(SourceFileUtils)

# Expand file extensions (i.e. path/to/file.{h,cpp} becomes path/to/file.h;path/to/file.cpp)
expand_file_extensions(SOURCE_FILES ${SOURCE_FILES})

# Generate source groups for use in IDEs
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${SOURCE_FILES})

if (WIN32)
    # clang-cl doesn't support resource files
    if (NOT (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
        list(APPEND SOURCE_FILES "icon.rc")
    endif ()
endif ()


####################################################################################################
#  EXECUTABLE TARGET
####################################################################################################
set(BINNAME "RoR")
add_executable(${BINNAME} ${SOURCE_FILES})

if (WIN32)
    set_target_properties(${BINNAME} PROPERTIES WIN32_EXECUTABLE YES)
    # disable some annoying VS warnings:
    # warning C4244: 'initializing' : conversion from 'const float' to 'int', possible loss of data
    # warning C4305: 'initializing' : truncation from 'double' to 'const float'
    target_compile_options(${BINNAME} PRIVATE /wd4305 /wd4244)
    # Disable non-standard behavior
    target_compile_options(${BINNAME} PRIVATE /permissive-)
endif ()

if (MSVC)
    # set startup project for Visual Studio Builds
    set_property(
            DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
            PROPERTY
            VS_STARTUP_PROJECT ${BINNAME}
    )
    # set Visual Studio working directory to the Configuration directory
    set_target_properties(
            ${BINNAME}
            PROPERTIES
            VS_DEBUGGER_WORKING_DIRECTORY "${RUNTIME_OUTPUT_DIRECTORY}"
    )
endif ()

####################################################################################################
#  PREPROCESSOR DEFINITIONS
####################################################################################################

target_compile_definitions(${BINNAME} PRIVATE
        USE_MUMBLE  # build with support for Mumble positional audio, has no dependencies but requires linking against librt on UNIX
        #FEAT_DEBUG_MUTEX
        )

if (ROR_FEAT_TIMING)
    target_compile_definitions(${BINNAME} PRIVATE FEAT_TIMING)
endif ()

if (ROR_USE_OIS_G27)
    target_compile_definitions(${BINNAME} PRIVATE USE_OIS_G27)
endif ()

if (WIN32)
    target_compile_definitions(${BINNAME} PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif ()

####################################################################################################
#  INCLUDE DIRECTORIES
####################################################################################################

target_include_directories(${BINNAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/external/header_only
        .
        audio
        datatypes
        gameplay
        gfx
        gfx/camera
        gfx/hydrax
        gfx/particle
        gfx/skyx
        gui
        gui/imgui
        gui/mygui
        gui/panels
        network
        physics
        physics/air
        physics/collision
        physics/flex
        physics/utils
        physics/water
        resources
        resources/addonpart_fileformat
        resources/odef_fileformat/
        resources/otc_fileformat/
        resources/rig_def_fileformat
        resources/skin_fileformat
        resources/terrn2_fileformat
        resources/tobj_fileformat
        resources/tuneup_fileformat
        system
        scripting
        scripting/bindings
        terrain
        terrain/map
        threadpool
        utils
        utils/memory
        )


####################################################################################################
# EXTERNAL DEPENDENCIES
#
# In CMake searching for external dependencies is done with the help of FindModule scripts. These
# are run by calling the find_package() command.
# CMake already ships with many ready-made FindModules (e.g. for OpenAL). Additional FindModule
# scripts that can be used are placed in the CMakeModule directory `<ror_root>/cmake`. These
# scripts typically define a set of variables, such as
#
#   <dependency_name>_FOUND           # indicates if the dependecy was found
#   <dependency_name>_INCLUDE_DIRS    # directories with headers to include
#   <dependency_name>_LIBRARIES       # libraries to link against
#
# Instead of directly using these variables, in modern CMake it is common to use additionally
# provided 'imported targets' for the sought dependency. This greatly facilitates dealing with
# dependencies. Adding a dependency to  an executable target is now simply a matter of specifying
# the imported target of the dependency in the target_link_libraries() command.
#
####################################################################################################


#  REQUIRED DEPENDENCIES
# -----------------------

if (WIN32)
    set(OS_LIBS "Ws2_32")
else ()
    #  include_directories(${GTK_INCLUDE_DIRS})
    set(OS_LIBS "X11 -l${CMAKE_DL_LIBS} -lrt")
endif ()
target_link_libraries(${BINNAME} PRIVATE ${OS_LIBS} version_info)


target_link_libraries(${BINNAME} PRIVATE
        Threads::Threads
        ${OGRE_LIBRARIES}
        ois::ois
        fmt::fmt
        MyGUI::MyGUI
        )
target_include_directories(${BINNAME} PRIVATE ${OGRE_INCLUDE_DIRS} ${RapidJSON_INCLUDE_DIRS})
if (TARGET MyGUI::OgrePlatform)
    target_link_libraries(${BINNAME} PRIVATE MyGUI::OgrePlatform)
endif ()
target_compile_definitions(${BINNAME} PRIVATE $<$<PLATFORM_ID:WINDOWS>:MYGUI_STATIC>)

#  OPTIONAL DEPENDENCIES
# -----------------------

if (ROR_USE_OPENAL)
    target_link_libraries(${BINNAME} PRIVATE ${OPENAL_LIBRARY})
    target_include_directories(${BINNAME} PRIVATE ${OPENAL_INCLUDE_DIR})
    target_compile_definitions(${BINNAME} PRIVATE USE_OPENAL)
endif ()

if (ROR_USE_DISCORD_RPC)
    target_link_libraries(${BINNAME} PRIVATE discord-rpc::discord-rpc)
    target_compile_definitions(${BINNAME} PRIVATE USE_DISCORD_RPC)
endif ()

if (ROR_USE_SOCKETW)
    target_link_libraries(${BINNAME} PRIVATE SocketW::SocketW)
    target_compile_definitions(${BINNAME} PRIVATE USE_SOCKETW)
endif ()

if (ROR_USE_ANGELSCRIPT)
    target_link_libraries(${BINNAME} PRIVATE Angelscript::angelscript angelscript_addons)
    target_include_directories(${BINNAME} PRIVATE ${CMAKE_SOURCE_DIR}/external/angelscript_addons)
    target_compile_definitions(${BINNAME} PRIVATE USE_ANGELSCRIPT AS_USE_NAMESPACE)
endif ()

if (ROR_USE_CURL)
    target_link_libraries(${BINNAME} PRIVATE CURL::libcurl)
    target_compile_definitions(${BINNAME} PRIVATE USE_CURL)

    if (WIN32)
        target_compile_definitions(${BINNAME} PRIVATE WIN32_LEAN_AND_MEAN)
    endif ()
endif ()

if (ROR_USE_CAELUM)
    target_link_libraries(${BINNAME} PRIVATE Caelum::Caelum)
    target_compile_definitions(${BINNAME} PRIVATE USE_CAELUM)
endif ()

if (ROR_USE_PAGED)
    target_link_libraries(${BINNAME} PRIVATE PagedGeometry::PagedGeometry)
    target_compile_definitions(${BINNAME} PRIVATE USE_PAGED)
endif ()

####################################################################################################
#  POST-BUILD STEPS
####################################################################################################

if (ROR_USE_PCH)
    target_precompile_headers(${BINNAME} PRIVATE pch.h)
endif ()

extract_pot("${SOURCE_FILES}")

# Configure plugins.cfg
# -----------------------
if (NOT WIN32)
    set(CFG_COMMENT_RENDERSYSTEM_D3D9 "# ")
endif ()

if (WIN32)
    set(CFG_COMMENT_RENDERSYSTEM_GL "# ")
endif ()

set(CFG_COMMENT_RENDERSYSTEM_D3D11 "# ")
set(CFG_COMMENT_RENDERSYSTEM_GL3PLUS "# ")

if (DEFINED OGRE_PLUGIN_DIR AND NOT WIN32)
    set(PLUGINS_FOLDER "${OGRE_PLUGIN_DIR}")
else ()
    set(PLUGINS_FOLDER ".")
endif ()

if (WIN32)
    set(CFG_OGRE_PLUGIN_CAELUM "Plugin=Caelum")
    set(CFG_OGRE_PLUGIN_CAELUM_D "Plugin=Caelum_d")
else ()
    if (ROR_USE_CAELUM STREQUAL "SYSTEM")
        set(CFG_OGRE_PLUGIN_CAELUM "Plugin=../libCaelum.so")
        set(CFG_OGRE_PLUGIN_CAELUM_D "Plugin=../libCaelum_d.so")
    else ()
        set(CFG_OGRE_PLUGIN_CAELUM "Plugin=libCaelum.so")
        set(CFG_OGRE_PLUGIN_CAELUM_D "Plugin=libCaelum_d.so")
    endif ()
endif ()

configure_file(plugins.cfg.in ${RUNTIME_OUTPUT_DIRECTORY}/plugins.cfg)
configure_file(plugins_d.cfg.in ${RUNTIME_OUTPUT_DIRECTORY}/plugins_d.cfg)

# Copy resources to bin directory
# -----------------------
recursive_zip_folder("${CMAKE_SOURCE_DIR}/resources" "${RUNTIME_OUTPUT_DIRECTORY}/resources")
if (ROR_CREATE_CONTENT_FOLDER)
    recursive_zip_folder("${CMAKE_SOURCE_DIR}/content" "${RUNTIME_OUTPUT_DIRECTORY}/content")
endif ()

fast_copy("${CMAKE_SOURCE_DIR}/resources/managed_materials" "${RUNTIME_OUTPUT_DIRECTORY}/resources/managed_materials")
fast_copy("${CMAKE_SOURCE_DIR}/resources/fonts" "${RUNTIME_OUTPUT_DIRECTORY}/languages")
fast_copy("${CMAKE_SOURCE_DIR}/languages" "${RUNTIME_OUTPUT_DIRECTORY}/languages")

# TODO: Run fixup_bundle instead
if (IS_DIRECTORY "${CMAKE_BINARY_DIR}/conan/bin")
    file(COPY "${CMAKE_BINARY_DIR}/conan/bin/" DESTINATION "${CMAKE_BINARY_DIR}/bin/")
endif ()


#  Install targets
# -----------------------
install(TARGETS ${BINNAME} DESTINATION . COMPONENT "Base_Game")

install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/fonts/ DESTINATION languages COMPONENT "Base_Game")
install(DIRECTORY ${CMAKE_SOURCE_DIR}/languages/ DESTINATION languages COMPONENT "Base_Game" FILES_MATCHING PATTERN "*.mo" )
install(DIRECTORY ${RUNTIME_OUTPUT_DIRECTORY}/resources/ DESTINATION resources COMPONENT "Base_Game")
install(DIRECTORY ${RUNTIME_OUTPUT_DIRECTORY}/content/ DESTINATION content COMPONENT "Base_Content")

if (WIN32)
    install(FILES ${RUNTIME_OUTPUT_DIRECTORY}/plugins.cfg DESTINATION . COMPONENT "Base_Game")
    install(FILES ${CMAKE_SOURCE_DIR}/tools/windows/.itch.toml DESTINATION . COMPONENT "Base_Game")
    install(CODE "file(GLOB files ${RUNTIME_OUTPUT_DIRECTORY}/*.dll)
            file(INSTALL \${files} DESTINATION \${CMAKE_INSTALL_PREFIX})" COMPONENT "Base_Game")
    install(CODE "file(GLOB files ${RUNTIME_OUTPUT_DIRECTORY}/*.exe)
            file(INSTALL \${files} DESTINATION \${CMAKE_INSTALL_PREFIX})" COMPONENT "Base_Game")
else ()
    set(PLUGINS_FOLDER "lib")
    configure_file(plugins.cfg.in ${CMAKE_CURRENT_BINARY_DIR}/plugins-install.cfg)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plugins-install.cfg DESTINATION . RENAME plugins.cfg)
    install(PROGRAMS ${CMAKE_SOURCE_DIR}/tools/linux/RunRoR DESTINATION .)
    install(FILES ${CMAKE_SOURCE_DIR}/tools/linux/.itch.toml DESTINATION .)
    install(CODE "file(GLOB_RECURSE files ${RUNTIME_OUTPUT_DIRECTORY}/*.so)
            file(INSTALL \${files} DESTINATION \${CMAKE_INSTALL_PREFIX}/lib FOLLOW_SYMLINK_CHAIN)")
endif ()


#  CPack
# -----------------------
set(CPACK_PACKAGE_NAME "Rigs of Rods")
set(CPACK_PACKAGE_FILE_NAME "rigs-of-rods-${CMAKE_PROJECT_VERSION}-${ROR_BUILD_INSTALLER}")
set(CPACK_PACKAGE_DESCRIPTION "Rigs of Rods soft-body physics simulator")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Rigs of Rods is an open source vehicle simulator based on soft-body physics.")
set(CPACK_PACKAGE_CONTACT "Edgar@AnotherFoxGuy.com")
set(CPACK_PACKAGE_VENDOR "AnotherFoxGuy")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/tools/windows/license.rtf")
set(CPACK_PACKAGE_EXECUTABLES "RoR" "Rigs of Rods")


if (WIN32 AND NOT ROR_BUILD_INSTALLER STREQUAL "Off")
    set(CPACK_GENERATOR INNOSETUP)
    set(CPACK_ADD_REMOVE TRUE)
    set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/tools/windows/AfterInstall.txt")
    set(CPACK_CREATE_DESKTOP_LINKS "RoR" "Rigs of Rods")

    set(CPACK_INNOSETUP_ALLOW_CUSTOM_DIRECTORY ON)
    set(CPACK_INNOSETUP_USE_MODERN_WIZARD ON)
    set(CPACK_INNOSETUP_ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ror.ico")
    set(CPACK_INNOSETUP_SETUP_WizardImageFile "${CMAKE_SOURCE_DIR}/tools/windows/RoRSetupLarge.bmp")
    set(CPACK_INNOSETUP_SETUP_WizardSmallImageFile "${CMAKE_SOURCE_DIR}/tools/windows/ror-64.bmp")
    set(CPACK_INNOSETUP_SETUP_AppId "{{EF536888-919D-47C0-8675-2F7AFA52EE27}")

    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}")

    cpack_configure_downloads("http://prdownloads.sourceforge.net/rigs-of-rods/")
    
    if(ROR_BUILD_INSTALLER STREQUAL "Bundled")
        set(CPACK_INNOSETUP_SETUP_DiskSpanning True)
    endif()

    if(ROR_BUILD_INSTALLER STREQUAL "Online")
        cpack_add_component("Base_Game"     DISPLAY_NAME "Game" GROUP "Main" REQUIRED TRUE DOWNLOADED TRUE)
        cpack_add_component("Base_Content"  DISPLAY_NAME "Starter content" GROUP "Main" DOWNLOADED TRUE)
    else()
        cpack_add_component("Base_Game"     DISPLAY_NAME "Game" GROUP "Main" REQUIRED TRUE)
        cpack_add_component("Base_Content"  DISPLAY_NAME "Starter content" GROUP "Main")
    endif ()

    if(NOT ROR_BUILD_INSTALLER STREQUAL "Offline")
        dl_contentpack("https://github.com/CuriousMike56/ContentPacks/releases/download/2022.12/ContentPack_HeavyEquipment.zip" "HeavyEquipment")
        dl_contentpack("https://github.com/CuriousMike56/ContentPacks/releases/download/2022.12/ContentPack_LightVehicles.zip" "LightVehicles")
        dl_contentpack("https://github.com/CuriousMike56/ContentPacks/releases/download/2022.12/ContentPack_Loads.zip" "Loads")
        dl_contentpack("https://github.com/CuriousMike56/ContentPacks/releases/download/2022.12/ContentPack_AirSea.zip" "AirSea")
        dl_contentpack("https://github.com/CuriousMike56/ContentPacks/releases/download/2022.12/ContentPack_Trains.zip" "Trains")
        dl_contentpack("https://github.com/CuriousMike56/ContentPacks/releases/download/2022.12/ContentPack_Terrains.zip" "Terrains")
    endif ()
else ()
    set(CPACK_GENERATOR ZIP)
endif ()

include(CPack)