# Install script for directory: /home/wjl/work/cv5/sdks/DeviceLayer/plugins/OnvifDevice

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/home/wjl/bqvision/clearvision/linux_x86_64/release")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "release")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "0")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FOREACH(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/plugins/onvifdevice/libonvifdeviceplugin-d.so.0.9.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/plugins/onvifdevice/libonvifdeviceplugin-d.so.0.9"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/plugins/onvifdevice/libonvifdeviceplugin-d.so"
      )
    IF(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      FILE(RPATH_CHECK
           FILE "${file}"
           RPATH "/home/wjl/bqvision/clearvision/linux_x86_64/release/lib")
    ENDIF()
  ENDFOREACH()
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/plugins/onvifdevice" TYPE SHARED_LIBRARY FILES
    "/home/wjl/work/cv5/sdks/DeviceLayer/plugins/OnvifDevice/build/cmake/x86_64/release/libonvifdeviceplugin-d.so.0.9.0"
    "/home/wjl/work/cv5/sdks/DeviceLayer/plugins/OnvifDevice/build/cmake/x86_64/release/libonvifdeviceplugin-d.so.0.9"
    "/home/wjl/work/cv5/sdks/DeviceLayer/plugins/OnvifDevice/build/cmake/x86_64/release/libonvifdeviceplugin-d.so"
    )
  FOREACH(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/plugins/onvifdevice/libonvifdeviceplugin-d.so.0.9.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/plugins/onvifdevice/libonvifdeviceplugin-d.so.0.9"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/plugins/onvifdevice/libonvifdeviceplugin-d.so"
      )
    IF(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      FILE(RPATH_CHANGE
           FILE "${file}"
           OLD_RPATH "/home/wjl/bqvision/clearvision/linux_x86_64/release/lib:"
           NEW_RPATH "/home/wjl/bqvision/clearvision/linux_x86_64/release/lib")
      IF(CMAKE_INSTALL_DO_STRIP)
        EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "${file}")
      ENDIF(CMAKE_INSTALL_DO_STRIP)
    ENDIF()
  ENDFOREACH()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
ELSE(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
ENDIF(CMAKE_INSTALL_COMPONENT)

FILE(WRITE "/home/wjl/work/cv5/sdks/DeviceLayer/plugins/OnvifDevice/build/cmake/x86_64/release/${CMAKE_INSTALL_MANIFEST}" "")
FOREACH(file ${CMAKE_INSTALL_MANIFEST_FILES})
  FILE(APPEND "/home/wjl/work/cv5/sdks/DeviceLayer/plugins/OnvifDevice/build/cmake/x86_64/release/${CMAKE_INSTALL_MANIFEST}" "${file}\n")
ENDFOREACH(file)
