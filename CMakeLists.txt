cmake_minimum_required(VERSION 2.6)

project ( turboproj )
enable_testing()

set(CMAKE_CXX_FLAGS "-DTURBOPUMP_PROJECT_ROOT=\\\"${turboproj_SOURCE_DIR}\\\"")
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
	set(CMAKE_BUILD_TYPE "RelWithDebInfo")
	set(CMAKE_C_FLAGS "${CFLAGS} -Wall -std=c99 -fsanitize=undefined -fsanitize=address")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -O2 -fpic -fno-rtti -std=c++11 -fsanitize=undefined -fsanitize=address")
	# -ftime-report -stdlib=libc++
endif()

set (TURBOLIB ${turboproj_SOURCE_DIR}/../turbolib)

include_directories(
	${turboproj_SOURCE_DIR}/src/lib
	${TURBOLIB}/src/lib
	${TURBOLIB}/src/third_party_lib
)

add_subdirectory(${TURBOLIB} ${TURBOLIB})

set (PROJECTS
   src/exe/turbopumpd

   src/lib/api
   test/lib/api
   src/lib/callbacks
   test/lib/callbacks
   src/lib/common
   test/lib/common
   src/lib/deskew
   test/lib/deskew
   src/lib/hashing
   test/lib/hashing
   src/lib/logging
   src/lib/membership
   test/lib/membership
   src/lib/peer_client
   test/lib/peer_client
   src/lib/peer_server
   test/lib/peer_server
   src/lib/storage
   test/lib/storage
   src/lib/turbopump
   src/lib/user_server
   test/lib/user_server

   test/integration/standup
   test/integration/versioning
   test/integration/deletes
   test/integration/load
   test/integration/persistence
   test/integration/dynamic_membership
   test/integration/partition

   test/lib/integration
   test/lib/mock
)

foreach(proj ${PROJECTS})
	add_subdirectory(${proj} build/${proj})
endforeach()


