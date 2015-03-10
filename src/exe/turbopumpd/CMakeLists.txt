cmake_minimum_required(VERSION 2.6)

project(turbopumpd)

set (SOURCES
	main.cpp
)

add_executable (
	turbopumpd
	${SOURCES}
)

target_link_libraries(turbopumpd

	main
	turbopump
	user_server

	http
)

