cmake_minimum_required(VERSION 3.16...3.27)
project(muontracker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ui_all vis_all pulls in the OpenGL viewer and the Qt/tcsh UI sessions.
# Without these two components the "vis" mode silently does nothing.
find_package(Geant4 REQUIRED ui_all vis_all)
include(${Geant4_USE_FILE})

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

add_executable(muontracker muontracker.cc ${sources} ${headers})
target_link_libraries(muontracker ${Geant4_LIBRARIES})

# Copy the macros next to the binary so /control/execute finds them
# whichever directory you launch from.
set(MACRO_FILES
    macros/vis.mac
    macros/vis_single.mac
    macros/vis_dense.mac
    macros/run.mac)

foreach(mac ${MACRO_FILES})
  configure_file(${PROJECT_SOURCE_DIR}/${mac}
                 ${PROJECT_BINARY_DIR}/${mac} COPYONLY)
endforeach()

install(TARGETS muontracker DESTINATION bin)
