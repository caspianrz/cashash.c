cmake_minimum_required(VERSION 3.16)

project(cashash.c
	VERSION 0.7.0
	DESCRIPTION "A small C hashing library."
	LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(CAS_TOP_LEVEL_PROJECT ON)
else()
    set(CAS_TOP_LEVEL_PROJECT OFF)
endif()

option(CAS_BUILD_TESTS   "Build cashash tests."    ${CAS_TOP_LEVEL_PROJECT})
option(CAS_BUILD_EXAMPLE "Build cashash examples." ${CAS_TOP_LEVEL_PROJECT})
option(CAS_BUILD_DOCS    "Build documentation."    ${CAS_TOP_LEVEL_PROJECT})

option(CAS_USE_XXHASH "Use xxHash algorithm library." OFF)
option(CAS_FETCH_XXHASH "Fetch xxHash if it is not installed." ON)

include(CheckIncludeFiles)
include(GNUInstallDirs)
include(FetchContent)


set(cashash_libraries "")
set(cashash_defines "")
file(GLOB cashash_sources "src/*.c")

if(CAS_USE_XXHASH)
find_package(xxHash CONFIG QUIET)

if(NOT xxHash_FOUND AND CAS_FETCH_XXHASH)
	FetchContent_Declare(
		xxhash
		GIT_REPOSITORY https://github.com/cyan4973/xxhash.git
		GIT_TAG v0.8.3
		SOURCE_SUBDIR cmake_unofficial
	)
	FetchContent_MakeAvailable(xxhash)
endif()
	if(TARGET xxHash::xxhash)
		list(APPEND cashash_libraries "xxHash::xxhash")
    elseif(TARGET xxhash)
		list(APPEND cashash_libraries "xxhash")
    else()
		message(FATAL_ERROR "CAS_USE_XXHASH=ON but xxHash was not found")
    endif()
	list(APPEND cashash_defines "CASHASH_USE_XXHASH=1")
endif()

add_library(cashash.c STATIC)
add_library(cashash::cashash ALIAS cashash.c)

target_sources(cashash.c PRIVATE ${cashash_sources})
target_include_directories(cashash.c
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(cashash.c PRIVATE ${cashash_libraries})
target_compile_definitions(cashash.c PUBLIC ${cashash_defines})

if(MSVC)
    target_compile_options(cashash.c
        PRIVATE
            /W4
            /permissive-
    )
else()
    target_compile_options(cashash.c
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
endif()

install(TARGETS cashash.c
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

if(CAS_BUILD_EXAMPLE)
    set(examples
        "sample1:examples/sample1.c"
		"hashtable_iteration:examples/hashtable_iteration.c"
		"hashtable_foreach:examples/hashtable_foreach.c"
		"simple_stats:examples/simple_stats.c"
    )
	if(CAS_USE_XXHASH)
		list(APPEND examples "sample2:examples/sample2.c")
	endif()

	set(example_output_dir "${CMAKE_BINARY_DIR}/example")

    foreach(example ${examples})
        string(REPLACE ":" ";" pair ${example})

        list(GET pair 0 name)
        list(GET pair 1 source)

        add_executable(${name} ${source})
        target_link_libraries(${name} PRIVATE cashash::cashash)

		set_target_properties(${name} PROPERTIES
        	RUNTIME_OUTPUT_DIRECTORY ${example_output_dir}
    	)

        target_include_directories(${name} PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        )
    endforeach()
endif()

if(CAS_BUILD_TESTS)
    enable_testing()

	set(tests
		"test_cashash_chain:tests/test_cashash_chain.c"
		"test_cashash_oa:tests/test_cashash_oa.c"
		"test_cashash_hash:tests/test_cashash_hash.c"
		"test_cashash_iter:tests/test_cashash_iter.c"
		"test_cashash_iter_oa:tests/test_cashash_iter_oa.c"
		"test_cashash_diagnostics:tests/test_cashash_diagnostics.c"
	)

	foreach(test ${tests})
		string(REPLACE ":" ";" pair ${test})

		list(GET pair 0 name)
		list(GET pair 1 source)

		add_executable(${name} ${source})

		target_link_libraries(${name}
			PRIVATE
				cashash::cashash
		)

		if(WIN32)
			find_package(check CONFIG REQUIRED)

			target_link_libraries(${name}
				PRIVATE
					$<IF:$<TARGET_EXISTS:Check::check>,Check::check,Check::checkShared>
			)
		else()
			find_package(PkgConfig REQUIRED)
			pkg_check_modules(CHECK REQUIRED IMPORTED_TARGET check)

			target_link_libraries(${name}
				PRIVATE
					PkgConfig::CHECK
			)
		endif()

		add_test(NAME ${name} COMMAND ${name})
	endforeach()
endif()

if(CAS_BUILD_DOCS)
	find_package(Doxygen)

	if(Doxygen_FOUND)
		FetchContent_Declare(
			doxygen-awesome-css
			GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css.git
			GIT_TAG        v2.4.2
		)
		FetchContent_MakeAvailable(doxygen-awesome-css)
		FetchContent_GetProperties(doxygen-awesome-css SOURCE_DIR AWESOME_CSS_DIR)

		configure_file(
			"${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in"
			"${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
			@ONLY
		)

		doxygen_add_docs(
			cashash_docs
			"${CMAKE_CURRENT_SOURCE_DIR}/include"
			"${CMAKE_CURRENT_SOURCE_DIR}/docs/mainpage.md"
			COMMENT "Generating cashash documentation"
			CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
		)
	else()
		message(STATUS "Doxygen not found; documentation target disabled")
	endif()
endif()
