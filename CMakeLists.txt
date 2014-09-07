cmake_minimum_required(VERSION 2.6)

project ( turboproj )
enable_testing()

set(CMAKE_CXX_FLAGS "-DTURBOPUMP_PROJECT_ROOT=\\\"${turboproj_SOURCE_DIR}\\\"")
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
	set(CMAKE_BUILD_TYPE "RelWithDebInfo")
	set(CMAKE_C_FLAGS "${CFLAGS} -Wall -std=c99")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O2 -fPIC")
	# -ftime-report
endif()
if(CMAKE_COMPILER_IS_GNUCXX)
    # gdwarf-3 to help older gdbs
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -flto -gdwarf-3")
endif()

set (TURBOLIB ${turboproj_SOURCE_DIR}/../turbolib)

include_directories(
	${turboproj_SOURCE_DIR}/src/lib
	${TURBOLIB}/src/lib
	${TURBOLIB}/src/third_party_lib
)

add_subdirectory(${TURBOLIB} ${TURBOLIB})

set (PROJECTS
   src/exe/turbopump
   src/lib/main

   src/lib/actions
   test/lib/actions
   src/lib/actions_req
   test/lib/actions_req
   src/lib/common
   test/lib/common
   src/lib/consistent_hashing
   test/lib/consistent_hashing
   src/lib/data_store
   test/lib/data_store
   src/lib/deskew
   test/lib/deskew
   src/lib/logging
   src/lib/membership
   test/lib/membership
   src/lib/programmable
   test/lib/programmable
   src/lib/user_server
   test/lib/user_server
   src/lib/wan_server
   test/lib/wan_server

   test/integration/standup
   test/integration/versioning
   test/integration/deletes
   test/integration/dynamic_membership
   test/integration/load
   test/integration/partition

   test/lib/integration
   test/lib/mock
)

foreach(proj ${PROJECTS})
	add_subdirectory(${proj} build/${proj})
endforeach()


