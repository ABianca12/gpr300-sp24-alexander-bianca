file(
 GLOB_RECURSE ASSIGNMENT0_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE ASSIGNMENT0_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this WorkSession1's asset folder to bin when it is built
add_custom_target(copyAssetsWA3 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${ASSIGNMENT0_INC} DESTINATION include/WorkSession2)
add_executable(WorkSession2 ${ASSIGNMENT0_SRC} ${ASSIGNMENT0_INC})
target_link_libraries(WorkSession2 PUBLIC core IMGUI assimp)
target_include_directories(WorkSession2 PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when WorkSession1 is built
add_dependencies(WorkSession2 copyAssetsWA3)