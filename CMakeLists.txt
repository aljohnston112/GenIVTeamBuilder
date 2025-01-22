cmake_minimum_required(VERSION 3.30)

project(gen_iv_team_builder)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)
FetchContent_Declare(
        glaze
        GIT_REPOSITORY https://github.com/stephenberry/glaze.git
        GIT_TAG main
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glaze)

add_executable(
        ${PROJECT_NAME}
        src/main.cpp
        src/models.h
        src/pokemon_data_source.cpp
        src/pokemon_data_source.h
        src/type_chart_data_source.cpp
        src/type_chart_data_source.h
)
target_link_libraries(${PROJECT_NAME} PRIVATE glaze::glaze)
