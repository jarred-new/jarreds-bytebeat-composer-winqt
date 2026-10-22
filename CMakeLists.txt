cmake_minimum_required(VERSION 3.16)

project(JarredsBeatComposer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS  Widgets Multimedia)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

qt_add_executable(JarredsBeatComposer
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    bytebeatgenerator.h
    custoperators.h
    bytebeattype.h
)

target_compile_options(JarredsBeatComposer PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
)

# if(MSVC)
#     add_compile_options(/bigobj)
# endif()

target_link_libraries(JarredsBeatComposer PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
)

# Third-party Libraries put here...
# ExprTk
target_include_directories(JarredsBeatComposer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/exprtk/include
)


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(Qt6_VERSION VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.JarredsBeatComposer)
endif()
set_target_properties(JarredsBeatComposer PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS JarredsBeatComposer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(JarredsBeatComposer)
