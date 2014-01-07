cmake_minimum_required(VERSION 2.6)

project ( TURBOPUMP )
enable_testing()

# gdwarf-3 to help older gdbs
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
	set(CMAKE_BUILD_TYPE "RelWithDebInfo")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O2 -fPIC -gdwarf-3")
	# -ftime-report
endif()
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++0x")
endif()

set (TURBOLIB ${TURBOPUMP_SOURCE_DIR}/../turbolib)

include_directories(
	${TURBOPUMP_SOURCE_DIR}/src/lib
	${TURBOLIB}/src/lib
)

add_subdirectory(${TURBOLIB} ${TURBOLIB})

set (PROJECTS
   src/exe/turbopump
   src/lib/main

   src/lib/actions
   test/lib/actions
   src/lib/actions_req
   test/lib/actions_req
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
   test/lib/user_control
   src/lib/wan_server
   test/lib/wan_server

   test/integration/standup
   test/integration/load
   test/integration/partition

   test/lib/integration
   test/lib/mock
)

foreach(proj ${PROJECTS})
	add_subdirectory(${proj} build/${proj})
endforeach()


