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
#Copies this WorkSession3's asset folder to bin when it is built
add_custom_target(copyAssetsWA4 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${ASSIGNMENT0_INC} DESTINATION include/WorkSession3)
add_executable(WorkSession3 ${ASSIGNMENT0_SRC} ${ASSIGNMENT0_INC})
target_link_libraries(WorkSession3 PUBLIC core IMGUI assimp)
target_include_directories(WorkSession3 PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when WorkSession3 is built
add_dependencies(WorkSession3 copyAssetsWA4)