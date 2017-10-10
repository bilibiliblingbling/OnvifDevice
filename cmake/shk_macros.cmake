#
# shk cmake macros
#

option(PACKAGER_CPACK "use cpack as packager" OFF)

# 通用设置, 需要尽早设置
macro(shk_setup_common)
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
  set(CMAKE_VERBOSE_MAKEFILE ON)

  if(NOT DEFINED INCLUDE_INSTALL_DIR)
    set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include)
  endif()
  include_directories(SYSTEM ${INCLUDE_INSTALL_DIR})

  if(NOT DEFINED LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX})
  endif()
  link_directories(${LIB_INSTALL_DIR})
endmacro()

# 安装头文件目录
macro(shk_install_include_directory dir)
  if(PACKAGER_CPACK)
    install(DIRECTORY ${dir} DESTINATION include/${ARGN})
  else()
    install(DIRECTORY ${dir} DESTINATION ${INCLUDE_INSTALL_DIR}/${ARGN})
  endif()
endmacro()

# 安装库
macro(shk_install_lib_targets)
  if(PACKAGER_CPACK)
    install(TARGETS ${ARGN} DESTINATION lib${LIB_SUFFIX})
  else()
    install(TARGETS ${ARGN} DESTINATION ${LIB_INSTALL_DIR})
  endif()
endmacro()

# TODO: 安装程序
macro(shk_install_execute)
  message("TODO: shk_install_execute")
endmacro()

# cpack
macro(shk_fire_cpack)
  if(PACKAGER_CPACK)
    if(NOT DEFINED CPACK_GENERATOR)
      set(CPACK_GENERATOR "TGZ")
    endif()

    # setup package version
    if(NOT DEFINED CPACK_PACKAGE_VERSION)
      set(CMAKE_PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})
    endif()
    if(NOT DEFINED CPACK_PACKAGE_VERSION_MAJOR)
      set(CMAKE_PACKAGE_VERSION ${CMAKE_PROJECT_VERSION_MAJOR})
    endif()
    if(NOT DEFINED CPACK_PACKAGE_VERSION_MINOR)
      set(CPACK_PACKAGE_VERSION_MINOR ${CMAKE_PROJECT_VERSION_MINOR})
    endif()
    if(NOT DEFINED CPACK_PACKAGE_VERSION_PATCH)
      set(CPACK_PACKAGE_VERSION_PATCH ${CMAKE_PROJECT_VERSION_PATCH})
    endif()

    if(NOT DEFINED CPACK_PACKAGE_VERDOR)
      set(CPACK_PACKAGE_VENDOR "BQVision")
    endif()

    if(NOT DEFINED CPACK_PACKAGE_DESCRIPTION_SUMMARY)
      set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${CMAKE_PROJECT_NAME}")
    endif()

    if(NOT DEFINED CPACK_PACKAGE_FILE_NAME)
      set(CPACK_PACKAGE_FILE_NAME ${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION})
    endif()

    include(CPack)
  endif()
endmacro()
