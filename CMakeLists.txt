cmake_minimum_required(VERSION 3.1)

project(RAPID-AF)

set(CMAKE_CXX_STANDARD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

set(OTHER_FILES
    .gitignore
    uncrustify.cfg
)

add_custom_target(project-related-files SOURCES ${OTHER_FILES})

add_library(rapid-af SHARED rapid-af.cpp)

install(TARGETS rapid-af
    DESTINATION lib
    EXPORT rapidAfTargets
)
install(FILES rapid-af.h
        DESTINATION inlcude
)
install(EXPORT rapidAfTargets DESTINATION lib/cmake)

target_include_directories(rapid-af
    PUBLIC
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
)

target_link_libraries(rapid-af
    ${OpenCV_LIBS}
    Threads::Threads
)


add_executable(rapid-af-bin main.cpp)
target_link_libraries(rapid-af-bin
    PUBLIC rapid-af
)

set_target_properties(rapid-af-bin
    PROPERTIES OUTPUT_NAME rapid-af)

target_include_directories(rapid-af-bin
    PUBLIC
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
)

install(TARGETS rapid-af-bin DESTINATION bin)
