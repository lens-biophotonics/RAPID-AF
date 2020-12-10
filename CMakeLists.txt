cmake_minimum_required(VERSION 3.1)

project(RAPID-AF)

set(CMAKE_CXX_STANDARD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

set(OTHER_FILES
    .gitignore
    uncrustify.cfg
)

set(SOURCES
    main.cpp
    rapid-af.cpp
)

add_custom_target(project-related-files SOURCES ${OTHER_FILES})

add_executable(rapid-af ${SOURCES})
target_link_libraries(rapid-af
    ${OpenCV_LIBS}
    Threads::Threads
)
