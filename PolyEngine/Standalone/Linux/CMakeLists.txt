find_package(X11 REQUIRED)

set(STANDALONE_SRCS main.cpp)

add_executable(standalone ${STANDALONE_SRCS})
target_include_directories(standalone PRIVATE ${X11_INCLUDE_DIR} ${GLAD_INCLUDE_DIRS})
target_link_libraries(standalone polycore polyengine polygldevice sgjgame glad ${X11_LIBRARIES})

cotire(standalone)
