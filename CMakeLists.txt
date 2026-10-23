cmake_minimum_required(VERSION 3.25)

project(LumaSpace VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Test)

add_library(lumaspace_viewport STATIC
    src/viewport/coordinate_space.cpp
    src/viewport/coordinate_space.h
)
target_include_directories(lumaspace_viewport PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_library(lumaspace_app_core STATIC
    src/app/app_mode.cpp
    src/app/app_mode.h
    src/app/mode_transition_coordinator.cpp
    src/app/mode_transition_coordinator.h
)
target_include_directories(lumaspace_app_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_library(lumaspace_actions STATIC
    src/actions/action_registry.cpp
    src/actions/action_registry.h
    src/actions/mode_actions.cpp
    src/actions/mode_actions.h
)
target_include_directories(lumaspace_actions PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(lumaspace_actions PUBLIC lumaspace_app_core)

add_library(lumaspace_windows_hotkeys STATIC
    src/platform/windows/dpi_awareness.cpp
    src/platform/windows/dpi_awareness.h
    src/platform/windows/hotkey_service.cpp
    src/platform/windows/hotkey_service.h
    src/platform/windows/hotkey_types.cpp
    src/platform/windows/hotkey_types.h
    src/platform/windows/overlay_surface.cpp
    src/platform/windows/overlay_surface.h
)
target_include_directories(lumaspace_windows_hotkeys PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(lumaspace_windows_hotkeys PUBLIC lumaspace_actions user32 gdi32)

add_library(lumaspace_app STATIC
    src/app/logging.cpp
    src/app/logging.h
    src/app/product_info.cpp
    src/app/product_info.h
    src/app/single_instance.cpp
    src/app/single_instance.h
)
target_link_libraries(lumaspace_app PUBLIC lumaspace_windows_hotkeys Qt6::Core)
target_include_directories(lumaspace_app PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_definitions(lumaspace_app PUBLIC QT_NO_KEYWORDS)

add_executable(LumaSpace WIN32 src/main.cpp)
target_link_libraries(LumaSpace PRIVATE lumaspace_app Qt6::Gui Qt6::Widgets)
set_target_properties(LumaSpace PROPERTIES
    OUTPUT_NAME LumaSpace
    WIN32_EXECUTABLE TRUE
)

if(MSVC)
    target_compile_options(lumaspace_viewport PRIVATE /W4 /permissive-)
    target_compile_options(lumaspace_app_core PRIVATE /W4 /permissive-)
    target_compile_options(lumaspace_actions PRIVATE /W4 /permissive-)
    target_compile_options(lumaspace_windows_hotkeys PRIVATE /W4 /permissive-)
    target_compile_options(lumaspace_app PRIVATE /W4 /permissive-)
    target_compile_options(LumaSpace PRIVATE /W4 /permissive-)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(lumaspace_smoke_tests tests/product_info_test.cpp)
    target_link_libraries(lumaspace_smoke_tests PRIVATE lumaspace_app Qt6::Test)
    if(MSVC)
        target_compile_options(lumaspace_smoke_tests PRIVATE /W4 /permissive-)
    endif()
    add_test(NAME product_info_smoke COMMAND lumaspace_smoke_tests)
    set_tests_properties(product_info_smoke PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
    )

    add_executable(lumaspace_app_mode_tests tests/app_mode_test.cpp)
    target_link_libraries(lumaspace_app_mode_tests PRIVATE lumaspace_app_core Qt6::Test)
    target_compile_definitions(lumaspace_app_mode_tests PRIVATE QT_NO_KEYWORDS)
    if(MSVC)
        target_compile_options(lumaspace_app_mode_tests PRIVATE /W4 /permissive-)
    endif()
    add_test(NAME app_mode_state_machine COMMAND lumaspace_app_mode_tests)
    set_tests_properties(app_mode_state_machine PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
    )

    add_executable(lumaspace_action_tests tests/action_registry_test.cpp)
    target_link_libraries(lumaspace_action_tests PRIVATE lumaspace_actions Qt6::Test)
    target_compile_definitions(lumaspace_action_tests PRIVATE QT_NO_KEYWORDS)
    if(MSVC)
        target_compile_options(lumaspace_action_tests PRIVATE /W4 /permissive-)
    endif()
    add_test(NAME action_registry COMMAND lumaspace_action_tests)
    set_tests_properties(action_registry PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
    )

    add_executable(lumaspace_hotkey_tests tests/hotkey_test.cpp)
    target_link_libraries(lumaspace_hotkey_tests PRIVATE lumaspace_windows_hotkeys Qt6::Test)
    target_compile_definitions(lumaspace_hotkey_tests PRIVATE QT_NO_KEYWORDS NOMINMAX WIN32_LEAN_AND_MEAN)
    if(MSVC)
        target_compile_options(lumaspace_hotkey_tests PRIVATE /W4 /permissive-)
    endif()
    add_test(NAME windows_hotkeys COMMAND lumaspace_hotkey_tests)
    set_tests_properties(windows_hotkeys PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
    )

    add_executable(lumaspace_coordinator_tests tests/mode_transition_coordinator_test.cpp)
    target_link_libraries(lumaspace_coordinator_tests PRIVATE lumaspace_app_core Qt6::Test)
    target_compile_definitions(lumaspace_coordinator_tests PRIVATE QT_NO_KEYWORDS)
    if(MSVC)
        target_compile_options(lumaspace_coordinator_tests PRIVATE /W4 /permissive-)
    endif()
    add_test(NAME mode_transition_coordinator COMMAND lumaspace_coordinator_tests)
    set_tests_properties(mode_transition_coordinator PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
    )

    add_executable(lumaspace_m1_integration_tests tests/m1_integration_test.cpp)
    target_link_libraries(lumaspace_m1_integration_tests PRIVATE lumaspace_actions Qt6::Test)
    target_compile_definitions(lumaspace_m1_integration_tests PRIVATE QT_NO_KEYWORDS)
    if(MSVC)
        target_compile_options(lumaspace_m1_integration_tests PRIVATE /W4 /permissive-)
    endif()
    add_test(NAME m1_action_transition_integration COMMAND lumaspace_m1_integration_tests)
    set_tests_properties(m1_action_transition_integration PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
    )

    add_executable(lumaspace_overlay_surface_tests tests/overlay_surface_test.cpp)
    target_link_libraries(lumaspace_overlay_surface_tests PRIVATE lumaspace_windows_hotkeys Qt6::Test)
    target_compile_definitions(lumaspace_overlay_surface_tests PRIVATE QT_NO_KEYWORDS)
    if(MSVC)
        target_compile_options(lumaspace_overlay_surface_tests PRIVATE /W4 /permissive-)
    endif()
    add_test(NAME win32_overlay_surface COMMAND lumaspace_overlay_surface_tests)
    set_tests_properties(win32_overlay_surface PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
    )

    add_executable(lumaspace_coordinate_tests tests/coordinate_space_test.cpp)
    target_link_libraries(lumaspace_coordinate_tests PRIVATE lumaspace_viewport Qt6::Test)
    target_compile_definitions(lumaspace_coordinate_tests PRIVATE QT_NO_KEYWORDS)
    if(MSVC)
        target_compile_options(lumaspace_coordinate_tests PRIVATE /W4 /permissive-)
    endif()
    add_test(NAME coordinate_spaces COMMAND lumaspace_coordinate_tests)
    set_tests_properties(coordinate_spaces PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
    )
endif()
