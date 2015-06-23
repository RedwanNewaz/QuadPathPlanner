cmake_minimum_required(VERSION 2.8.3)
project(jaist_quad_gui)

## Find catkin macros and libraries

find_package(catkin REQUIRED COMPONENTS
  ardrone_autonomy
  cv_bridge
  dynamic_reconfigure
  geometry_msgs
  message_generation
  roscpp
  rospy
  sensor_msgs
  std_msgs
  std_srvs
  image_transport
)

## System dependencies are found with CMake's conventions
find_package(Boost REQUIRED COMPONENTS system)
find_package(PCL 1.2 REQUIRED)

catkin_package(
 INCLUDE_DIRS include
 LIBRARIES jaist_quad_gui
 CATKIN_DEPENDS image_transport sensor_msgs cv_bridge
 # CATKIN_DEPENDS ardrone_autonomy cv_bridge dynamic_reconfigure geometry_msgs message_generation roscpp rospy sensor_msgs std_msgs std_srvs
#  DEPENDS system_lib
)


include_directories(
  ${catkin_INCLUDE_DIRS}
  ${PCL_INCLUDE_DIRS}
)

# ---------------------------- GUI --------------------------------------------------
# set header ans source files

set(PATH_PLANNER_HEADERS
    src/pathPlanner/Dstar.h
    src/pathPlanner/spline.h
    src/pathPlanner/path.h
    src/pathPlanner/pathsmoother.h
)

set(PATH_PLANNER_SOURCES
    src/pathPlanner/Dstar.cpp
    src/pathPlanner/path.cpp
    src/pathPlanner/pathsmoother.cpp
)




set(GUI_SOURCE_FILES         
  src/main.cpp
  src/mainwindow.cpp
  src/ros_thread.cpp
  src/ros_launch.cpp
  src/qcustomplot.cpp
  ${PATH_PLANNER_SOURCES}


)

set(GUI_HEADER_FILES
  ${PATH_PLANNER_HEADERS}
  src/mainwindow.h      
  src/ros_thread.h
  src/ros_launch.h
  src/qcustomplot.h
  src/stack.h
)

# *.ui
set(GUI_UI_FILES        
  src/mainwindow.ui 
)

# *.qrc
set(GUI_RESOURCE_FILES        
)

# do QT stuff
ADD_DEFINITIONS( -Wall )
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
QT4_ADD_RESOURCES(GUI_RESOURCE_FILES_CPP ${GUI_RESOURCE_FILES})
QT4_WRAP_UI(GUI_UI_FILES_HPP ${GUI_UI_FILES})
QT4_WRAP_CPP(GUI_HEADER_FILES_HPP ${GUI_HEADER_FILES})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# build!
add_executable(quad_gui ${GUI_SOURCE_FILES} ${GUI_RESOURCE_FILES_CPP} ${GUI_UI_FILES_HPP} ${GUI_HEADER_FILES_HPP})
target_link_libraries(quad_gui ${QT_LIBRARIES} ${catkin_LIBRARIES} ${PCL_LIBRARIES})
# add_dependencies(quad_gui thirdparty ${PROJECT_NAME}_gencpp ${PROJECT_NAME}_gencfg)


