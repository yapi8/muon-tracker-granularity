cmake_minimum_required(VERSION 3.16)
project(MuonTracker)

# ui_all vis_all pulls in the interactive session and the visualisation
# drivers (OGL, VRML, HepRep...). Without them G4VisExecutive links but
# has no viewer to open, and /vis/open fails at runtime.
find_package(Geant4 REQUIRED ui_all vis_all)
include(${Geant4_USE_FILE})

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)

add_executable(MuonTracker muontracker.cc ${sources})
target_link_libraries(MuonTracker ${Geant4_LIBRARIES})

# copy vis.mac next to the executable so /control/execute vis.mac works
configure_file(${PROJECT_SOURCE_DIR}/vis.mac
               ${PROJECT_BINARY_DIR}/vis.mac COPYONLY)
