cmake_minimum_required(VERSION 3.16)
project(FarmSimulator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SFML은 Dockerfile에서 libsfml-dev로 이미 설치됨
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(farm_sim
    main.cpp
    src/game/Game.cpp
)

target_include_directories(farm_sim PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(farm_sim PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
)

# 빌드 후 assets 폴더를 빌드 디렉토리로 복사
add_custom_command(TARGET farm_sim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/assets
    COMMENT "Copying assets to build directory"
)
