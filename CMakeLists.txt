cmake_minimum_required(VERSION 2.6)

project ( dynamo )
enable_testing()

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
	set(CMAKE_BUILD_TYPE "RelWithDebInfo")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O2")
	# -ftime-report
endif()
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++0x")
endif()

set (CORELIB ${dynamo_SOURCE_DIR}/../cmake)

include_directories(
	${dynamo_SOURCE_DIR}/src/lib
	${CORELIB}/src/lib
)

add_subdirectory(${CORELIB} ${CORELIB})

set (PROJECTS
   src/exe/turbopump
   src/lib/main
   test/integration/standup

   src/lib/actions
   test/lib/actions
   src/lib/cohesion
   test/lib/cohesion
   src/lib/common
   test/lib/common
   src/lib/consistent_hashing
   test/lib/consistent_hashing
   src/lib/data_store
   test/lib/data_store
   src/lib/membership
   test/lib/membership
   src/lib/programmable
   test/lib/programmable
   src/lib/user_control
   src/lib/wan_server
   test/lib/wan_server

   test/lib/mock
)

foreach(proj ${PROJECTS})
	add_subdirectory(${proj} build/${proj})
endforeach()


