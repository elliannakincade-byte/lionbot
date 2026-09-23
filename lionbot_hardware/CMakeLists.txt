cmake_minimum_required(VERSION 3.8)
project(lionbot_hardware)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)

find_library(LIBSERIAL_LIBRARY serial REQUIRED)

add_library(lionbot_arduino_comms SHARED
  src/arduino_comms.cpp
)

target_compile_features(lionbot_arduino_comms PUBLIC cxx_std_17)

target_include_directories(lionbot_arduino_comms PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(lionbot_arduino_comms
  ${LIBSERIAL_LIBRARY}
)

add_executable(arduino_link_test
  src/arduino_link_test.cpp
)

target_compile_features(arduino_link_test PUBLIC cxx_std_17)

target_link_libraries(arduino_link_test
  lionbot_arduino_comms
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  TARGETS
    lionbot_arduino_comms
    arduino_link_test
  EXPORT export_lionbot_hardware
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

ament_export_targets(export_lionbot_hardware HAS_LIBRARY_TARGET)
ament_export_include_directories(include)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
