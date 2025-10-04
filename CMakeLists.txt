cmake_minimum_required(VERSION 3.20)

# ---- Project ----
project(koverlay VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# ---- Deps ----
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets WaylandClient DBus Qml Quick)
find_package(LayerShellQt REQUIRED)
find_package(Wayland REQUIRED COMPONENTS Client)

# ---- Target ----
add_executable(koverlay
  src/main.cpp
  src/overlay_config.h
  src/overlay_view.h
  src/overlay_view.cpp
  src/overlay_adaptor.h
  src/overlay_adaptor.cpp
  src/resources.qrc
)

target_link_libraries(koverlay PRIVATE
  Qt6::Core Qt6::Gui Qt6::Widgets Qt6::WaylandClient Qt6::DBus
  Qt6::Qml Qt6::Quick Qt6::GuiPrivate
  LayerShellQt::Interface
  Wayland::Client
)

# ---- Dev convenience ----
add_custom_target(run
  COMMAND QT_QPA_PLATFORM=wayland $<TARGET_FILE:koverlay> --show
  DEPENDS koverlay
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

# ---- Install ----
include(GNUInstallDirs)

install(TARGETS koverlay
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Optional desktop file (install only if present)
if(EXISTS "${CMAKE_SOURCE_DIR}/packaging/koverlay.desktop")
  install(FILES packaging/koverlay.desktop
          DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
endif()

# ---- CPack (RPM) ----
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME "koverlay")
set(CPACK_PACKAGE_VENDOR "koverlay")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Click-through keybindings overlay for KDE Wayland")
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Utilities")

# Let RPM auto-detect exact .so deps
set(CPACK_RPM_PACKAGE_AUTOREQPROV ON)

# Also add human-friendly meta packages users should have
set(CPACK_RPM_PACKAGE_REQUIRES
    "layer-shell-qt, qt6-qtbase, qt6-qtdeclarative, qt6-qtwayland")

include(CPack)
