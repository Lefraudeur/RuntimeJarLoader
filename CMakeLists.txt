# CMakeList.txt : CMake project for MetaJNI2, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("JarLoader")

file(GLOB APP_SOURCES "src/*.cpp" "src/*.c" "src/*.hpp" "src/*.h" "src/*.hpp" "src/*.h")

# Add source to this project's executable.
add_library(JarLoader SHARED ${APP_SOURCES})


target_include_directories(JarLoader PUBLIC "include" "include/jni")

target_link_directories(JarLoader PUBLIC "lib")
target_link_libraries(JarLoader PUBLIC "jvm" "ixwebsocket" "z" "ssl" "crypto")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET JarLoader PROPERTY CXX_STANDARD 20)
endif()

if(WIN32)
	target_include_directories(JarLoader PUBLIC "include/jni/win32")
	target_link_libraries(JarLoader PUBLIC "shlwapi" "WS2_32" "CRYPT32")
elseif(UNIX)
	target_include_directories(JarLoader PUBLIC "include/jni/linux")
	target_link_libraries(JarLoader PUBLIC "ixwebsocket" "z" "ssl" "crypto" "X11") # apt install libx11-dev
endif()

# TODO: Add tests and install targets if needed.
