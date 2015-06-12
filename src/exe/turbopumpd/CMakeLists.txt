cmake_minimum_required(VERSION 2.6)

project(turbopumpd)

set (SOURCES
	main.cpp

	app.h
)

add_executable (
	turbopumpd
	${SOURCES}
)

target_link_libraries(turbopumpd

	turbopump
	user_server

	http

	pthread
)

add_custom_command(
	TARGET turbopumpd POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:turbopumpd> turbopumpd.dbg
	COMMAND ${CMAKE_STRIP} -g $<TARGET_FILE:turbopumpd>
)

install(
	TARGETS turbopumpd
	DESTINATION ${turboproj_SOURCE_DIR}/dist/bin
)


