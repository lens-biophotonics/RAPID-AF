cmake_minimum_required(VERSION 3.0)

project(RAPID-AF)

find_package(OpenCV REQUIRED)

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
)
