IF(COMMAND cmake_policy)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

PROJECT(render)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

#--------------------------------------------------------------------------

# Just debug / release 
set (CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# Setup compiler specific flags for different configuration modes
IF (MSVC)
  ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  ADD_DEFINITIONS(-DNOMINMAX)
ELSE (MSVC)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
ENDIF(MSVC)

# out-comment to support unit-testing
#ENABLE_TESTING()   

# Add our own cmake modules
SET(
  CMAKE_MODULE_PATH 
  ${PROJECT_SOURCE_DIR}/CMake 
  ${PROJECT_SOURCE_DIR}/cmake 
  ${CMAKE_MODULE_PATH}
  )

# Create a header file that can be used to find data paths at runtime
CONFIGURE_FILE(
  "${PROJECT_SOURCE_DIR}/configuration.h.in"
  "${PROJECT_SOURCE_DIR}/configuration.h"
  @ONLY
)

#--------------------------------------------------------------------------

FIND_PACKAGE(OpenGL)
IF(NOT OPENGL_FOUND)
  MESSAGE("Could not find OpenGL library")
ENDIF()

# FIND_PACKAGE(GLUT)  # sucks on windows so we have to do our own stuff
FIND_PACKAGE(MYGLUT)
IF(NOT GLUT_FOUND)
  MESSAGE("Could not find GLUT library")
ENDIF()

#---------------------------------------------------------------------

FILE(GLOB SOIL_PROPS_SRCS ${PROJECT_SOURCE_DIR}/SOIL/*.c)
FILE(GLOB SOIL_PROPS_HDRS ${PROJECT_SOURCE_DIR}/SOIL/*.h )

ADD_LIBRARY(SOIL
  ${SOIL_PROPS_SRCS}
  ${SOIL_PROPS_HDRS} 
  )

SET_TARGET_PROPERTIES(SOIL PROPERTIES DEBUG_POSTFIX "d" )

#---------------------------------------------------------------------

INCLUDE_DIRECTORIES(  
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/SOIL
  ${PROJECT_SOURCE_DIR}/3rdparty/include
  ${OPENGL_INCLUDE_DIR}
  ${GLUT_INCLUDE_DIR}
)

FILE(GLOB RAY_TRACE_SRCS ${PROJECT_SOURCE_DIR}/raytrace/*.cpp)
FILE(GLOB RAY_TRACE_HDRS ${PROJECT_SOURCE_DIR}/raytrace/*.h )

ADD_EXECUTABLE(raytrace
  ${RAY_TRACE_SRCS}
  ${RAY_TRACE_HDRS} 
  )

TARGET_LINK_LIBRARIES(raytrace SOIL ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

SET_TARGET_PROPERTIES(raytrace PROPERTIES DEBUG_POSTFIX "d" )
SET_TARGET_PROPERTIES(raytrace PROPERTIES RUNTIME_OUTPUT_DIRECTORY                "${PROJECT_SOURCE_DIR}/bin/")
SET_TARGET_PROPERTIES(raytrace PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG          "${PROJECT_SOURCE_DIR}/bin/")
SET_TARGET_PROPERTIES(raytrace PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE        "${PROJECT_SOURCE_DIR}/bin/")
SET_TARGET_PROPERTIES(raytrace PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     "${PROJECT_SOURCE_DIR}/bin/")
SET_TARGET_PROPERTIES(raytrace PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PROJECT_SOURCE_DIR}/bin/")

#---------------------------------------------------------------------
