# swift cmake macros
macro(swift_add_test test)
  add_executable(${test} ${test}.cc ${ARGN})
  target_link_libraries(${test} swift)
  add_test(${test} ${test})
endmacro()

macro(swift_enable_tests)
  enable_testing()
  add_subdirectory(tests)
  include(CTest)
endmacro()
