cmake_minimum_required(VERSION 3.5)

project(mocotrack VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # generates detailed compile_commands.json file
#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO /MD /Zi /O2 /Ob1 /DNDEBUG)
#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO /MD /Zi /Od /Ob0 /DNDEBUG)

if (CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
    add_compile_options(/Od /Ob0) # turns off optimisation and function inlining
endif ()
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
add_compile_definitions(_USE_MATH_DEFINES)
add_compile_definitions(_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS)
add_compile_definitions(_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

set(PROJECT_SOURCES
    ArgParse.cpp
    ArgParse.h
    Tracker.cpp
    Tracker.h
    XMLWriter.cpp
    XMLWriter.h
    main.cpp
    ../pystring/pystring.cpp
    ../pystring/pystring.h
)

add_executable(mocotrack
    ${PROJECT_SOURCES}
)

target_include_directories(mocotrack PRIVATE
    ".."
    "C:/Program Files/OpenSim 4.5/sdk/include"
    "C:/Program Files/OpenSim 4.5/sdk/include/OpenSim"
    "C:/Program Files/OpenSim 4.5/sdk/Simbody/include"
    "C:/Program Files/OpenSim 4.5/sdk/spdlog/include"
)

target_link_libraries(mocotrack PRIVATE
    "C:/Program Files/OpenSim 4.5/sdk/lib/osimActuators.lib"
    "C:/Program Files/OpenSim 4.5/sdk/lib/osimAnalyses.lib"
    "C:/Program Files/OpenSim 4.5/sdk/lib/osimCommon.lib"
    "C:/Program Files/OpenSim 4.5/sdk/lib/osimExampleComponents.lib"
    "C:/Program Files/OpenSim 4.5/sdk/lib/osimLepton.lib"
    "C:/Program Files/OpenSim 4.5/sdk/lib/osimMoco.lib"
    "C:/Program Files/OpenSim 4.5/sdk/lib/osimSimulation.lib"
    "C:/Program Files/OpenSim 4.5/sdk/lib/osimTools.lib"
    "C:/Program Files/OpenSim 4.5/sdk/lib/tropter.lib"
    "C:/Program Files/OpenSim 4.5/sdk/Simbody/lib/SimTKcommon.lib"
    "C:/Program Files/OpenSim 4.5/sdk/Simbody/lib/SimTKmath.lib"
    "C:/Program Files/OpenSim 4.5/sdk/Simbody/lib/SimTKsimbody.lib"
)


include(GNUInstallDirs)
install(TARGETS mocotrack
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


add_custom_command(TARGET mocotrack POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/SimTKcommon.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/SimTKmath.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/SimTKsimbody.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/adolc.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_conic_nlpsol.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_conic_qrqp.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_importer_shell.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_integrator_collocation.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_integrator_cvodes.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_integrator_idas.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_integrator_rk.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_interpolant_bspline.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_interpolant_linear.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_linsol_csparse.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_linsol_csparsecholesky.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_linsol_ldl.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_linsol_lsqr.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_linsol_qr.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_linsol_symbolicqr.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_linsol_tridiag.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_nlpsol_ipopt.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_nlpsol_qrsqp.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_nlpsol_scpgen.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_nlpsol_sqpmethod.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_rootfinder_fast_newton.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_rootfinder_kinsol.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_rootfinder_newton.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_rootfinder_nlpsol.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_sundials_common.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/casadi_xmlfile_tinyxml.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/concrt140.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/ezc3d.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/flang.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/flangrti.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/libgcc_s_sjlj-1.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/libgfortran-3.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/libblas.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/liblapack.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/libomp.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/libquadmath-0.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/openblas.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/osimActuators.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/osimAnalyses.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/osimCommon.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/osimExampleComponents.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/osimLepton.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/osimMoco.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/osimSimulation.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/osimTools.dll" ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy "C:/Program Files/OpenSim 4.5/bin/tropter.dll" ${CMAKE_CURRENT_BINARY_DIR}
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


