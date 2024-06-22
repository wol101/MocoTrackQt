cmake_minimum_required(VERSION 3.5)

project(MocoTrackQt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # generates detailed compile_commands.json file
#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO /MD /Zi /O2 /Ob1 /DNDEBUG)
#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO /MD /Zi /Od /Ob0 /DNDEBUG)

if (CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
    add_compile_options(/Od /Ob0) # turns off optimisation and function inlining
endif ()
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
add_compile_definitions(_USE_MATH_DEFINES)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        MainWindow.cpp
        MainWindow.h
        MainWindow.ui
        Tracker.cpp
        Tracker.h 
        resources.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MocoTrackQt
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET MocoTrackQt APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(MocoTrackQt SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(MocoTrackQt
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(MocoTrackQt PRIVATE
    "C:/Program Files/OpenSim 4.5/sdk/include"
    "C:/Program Files/OpenSim 4.5/sdk/include/OpenSim"
    "C:/Program Files/OpenSim 4.5/sdk/Simbody/include"
    "C:/Program Files/OpenSim 4.5/sdk/spdlog/include"
)

target_link_libraries(MocoTrackQt PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    "C:/Program Files/OpenSim 4.5/sdk/lib/osimActuators.lib"
    "C:/Program Files/OpenSim 4.5/sdk/lib/osimAnalyses.lib"
    "C:/Program Files/OpenSim 4.5/sdk/lib/osimCommon.lib"
    "C:/Program Files/OpenSim 4.5/sdk/lib/osimExampleComponents.lib"
    "C:/Program Files/OpenSim 4.5/sdk/lib/osimLepton.lib"
    "C:/Program Files/OpenSim 4.5/sdk/lib/osimMoco.lib"
    "C:/Program Files/OpenSim 4.5/sdk/lib/osimSimulation.lib"
    "C:/Program Files/OpenSim 4.5/sdk/lib/osimTools.lib"
    "C:/Program Files/OpenSim 4.5/sdk/lib/tropter.lib"
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.MocoTrackQt)
endif()
set_target_properties(MocoTrackQt PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS MocoTrackQt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(MocoTrackQt)
endif()

add_custom_command(TARGET MocoTrackQt POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/SimTKcommon.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/SimTKmath.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/SimTKsimbody.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/adolc.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_conic_nlpsol.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_conic_qrqp.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_importer_shell.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_integrator_collocation.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_integrator_cvodes.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_integrator_idas.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_integrator_rk.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_interpolant_bspline.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_interpolant_linear.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_linsol_csparse.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_linsol_csparsecholesky.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_linsol_ldl.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_linsol_lsqr.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_linsol_qr.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_linsol_symbolicqr.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_linsol_tridiag.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_nlpsol_ipopt.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_nlpsol_qrsqp.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_nlpsol_scpgen.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_nlpsol_sqpmethod.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_rootfinder_fast_newton.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_rootfinder_kinsol.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_rootfinder_newton.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_rootfinder_nlpsol.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_sundials_common.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/casadi_xmlfile_tinyxml.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/coinmumps-3.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/concrt140.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/ezc3d.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/ipopt-3.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/libgcc_s_sjlj-1.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/libgfortran-3.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/libifcoremd.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/libblas.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/libiomp5md.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/liblapack.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/libmmd.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/libquadmath-0.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/osimActuators.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/osimAnalyses.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/osimCommon.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/osimExampleComponents.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/osimLepton.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/osimMoco.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/osimSimulation.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/osimTools.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/svml_dispmd.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/dev/opensim-core-release/bin/tropter.dll" ${CMAKE_CURRENT_BINARY_DIR}
)

function(dump_cmake_variables)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        if (ARGV0)
            unset(MATCHED)
            string(REGEX MATCH ${ARGV0} MATCHED ${_variableName})
            if (NOT MATCHED)
                continue()
            endif()
        endif()
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endfunction()

# handy variable dump to General Messages window on QtCreator
dump_cmake_variables()


