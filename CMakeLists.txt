cmake_minimum_required(VERSION 3.8)
project(transition_judge_interface)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

#==========================================================
# 依存関係
#==========================================================
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(transition_recipe_test REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)

#==========================================================
# ライブラリターゲット
#==========================================================
add_library(${PROJECT_NAME}_library
  src/transition_judge_interface_node.cpp
  src/transition_judge_interface_component.cpp
)

ament_target_dependencies(${PROJECT_NAME}_library
  rclcpp
  std_msgs
  geometry_msgs
  nav_msgs
  sensor_msgs
  visualization_msgs
  tf2
  tf2_geometry_msgs
)


# transition_recipe_test exports executables alongside libs, so
# ament_target_dependencies(... transition_recipe_test) would try to link
# imported executable targets and fail. Link only the C++ typesupport target.
target_link_libraries(${PROJECT_NAME}_library
  transition_recipe_test::transition_recipe_test__rosidl_typesupport_cpp
  transition_recipe_test::transition_recipe_test__rosidl_generator_cpp
)
target_include_directories(${PROJECT_NAME}_library PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

#==========================================================
# 実行ファイル
#==========================================================
add_executable(${PROJECT_NAME} src/transition_judge_interface.cpp)
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_library)

# pure_pursuit_planner のみの構成用（ALL_UNCONFIGURED -> ALL_CONFIGURED -> pure_pursuit_planner だけ）
add_executable(transition_judge_pp_only src/transition_judge_pp_only.cpp)
ament_target_dependencies(transition_judge_pp_only rclcpp std_msgs)
target_link_libraries(transition_judge_pp_only
  transition_recipe_test::transition_recipe_test__rosidl_typesupport_cpp
  transition_recipe_test::transition_recipe_test__rosidl_generator_cpp
)

#==========================================================
# インストール
#==========================================================
install(TARGETS
  ${PROJECT_NAME}
  ${PROJECT_NAME}_library
  transition_judge_pp_only
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include/
)

install(DIRECTORY config
  DESTINATION share/${PROJECT_NAME}
)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

#==========================================================
# Lint / Test
#==========================================================
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
