# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "AUCTool_autogen"
  "CMakeFiles\\AUCTool_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\AUCTool_autogen.dir\\ParseCache.txt"
  )
endif()
