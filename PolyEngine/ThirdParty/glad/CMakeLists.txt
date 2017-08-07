add_library(glad STATIC src/glad.cpp src/glad_glx.cpp)
target_include_directories(glad PUBLIC include)
target_link_libraries(glad PRIVATE ${CMAKE_DL_LIBS})
set_target_properties(glad PROPERTIES LINKER_LANGUAGE CXX POSITION_INDEPENDENT_CODE ON)
