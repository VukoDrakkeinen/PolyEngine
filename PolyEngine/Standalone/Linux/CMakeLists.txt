find_package(X11 REQUIRED)
find_package(epoxy REQUIRED)

set(STANDALONE_SRCS main.cpp)

add_executable(standalone ${STANDALONE_SRCS})
target_include_directories(standalone PRIVATE ${X11_INCLUDE_DIR})
target_link_libraries(standalone polycore polyengine polygldevice sgjgame  ${X11_LIBRARIES} ${epoxy_LIBRARY})
add_custom_command(TARGET standalone POST_BUILD COMMAND ${CMAKE_COMMAND} -E create_symlink "${PROJECT_SOURCE_DIR}/Engine/Res" $<TARGET_FILE_DIR:polyengine>/Res)

cotire(standalone)
