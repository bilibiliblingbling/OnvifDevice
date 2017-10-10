# swift cmake defines

include(cmake/shk_macros.cmake)
include(cmake/swift_macros.cmake)

set(CMAKE_PROJECT_VERSION_MAJOR 0)
set(CMAKE_PROJECT_VERSION_MINOR 9)
set(CMAKE_PROJECT_VERSION_PATCH 0)
set(CMAKE_PROJECT_VERSION ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH})


option(ENABLE_TESTS "build tests(default off)" OFF)

shk_setup_common()

# 编译选项在编译脚本中统一指定
if(UNIX)
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -fvisibility=default -DWITH_OPENSSL")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default -DWITH_OPENSSL -D_GLIBCXX_USE_CXX11_ABI=0")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=default -DWITH_OPENSSL -std=c++0x")

  set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG}   -DSWIFT_DEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DSWIFT_DEBUG -O0")
endif()

#include_directories(BEFORE ${CMAKE_SOURCE_DIR}/include)
include_directories(BEFORE ${CMAKE_SOURCE_DIR}/include)
include_directories(../Utils/include
                    ../DLPShareRes/include)
# shared library
add_library(${CMAKE_PROJECT_NAME} SHARED ${HDR_LIST} ${SRC_LIST})

set_target_properties(${CMAKE_PROJECT_NAME}
  PROPERTIES OUTPUT_NAME "${CMAKE_PROJECT_NAME}-d"
  VERSION ${CMAKE_PROJECT_VERSION}
  SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR})

#TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME} ssl uuid crypto DeviceLayer Dlp_UUids Dlp_Utils Dlp_ShareRes swift-1.0.0-gcc tinyxpath pthread)
target_link_libraries(${CMAKE_PROJECT_NAME} ssl uuid crypto DeviceLayer Dlp_UUids Dlp_Utils Dlp_ShareRes swift-1.0.0-gcc tinyxpath pthread)

if(ENABLE_TESTS)
  swift_enable_tests()
endif()

install(TARGETS ${CMAKE_PROJECT_NAME} LIBRARY DESTINATION lib/plugins/onvifdevice)
#shk_install_include_directory(${CMAKE_SOURCE_DIR}/include/swift)
#shk_install_lib_targets(${CMAKE_PROJECT_NAME} ${CMAKE_PROJECT_NAME}-static)

shk_fire_cpack()
