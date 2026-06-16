cmake_minimum_required(VERSION 3.16)

project(cashash.c
	VERSION 0.4.0
	DESCRIPTION "A small C hashing library."
	LANGUAGES C
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(CAS_BUILD_TESTS "Build cashash tests." ON)
option(CAS_BUILD_EXAMPLE "Build cashash examples." ON)
option(CAS_BUILD_DOCS "Build documentation with Doxygen." ON)

option(CAS_USE_XXHASH "Use xxHash algorithm library." OFF)
option(CAS_FETCH_XXHASH "Fetch xxHash if it is not installed." ON)

include(CheckIncludeFiles)
include(GNUInstallDirs)
include(FetchContent)


set(cashash_libraries "")
set(cashash_defines "")
set(cashash_sources "src/cashash.c" "src/cashash_hash.c" "src/cashash_iter.c")

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
		"test_cashash:tests/test_cashash.c"
		"test_cashash_hash:tests/test_cashash_hash.c"
		"test_cashash_iter:tests/test_cashash_iter.c"
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

    if(DOXYGEN_FOUND)
        set(DOXYGEN_PROJECT_NAME "cashash.c")
        set(DOXYGEN_PROJECT_BRIEF "Small C string-key hash table library")
        set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/docs")
        set(DOXYGEN_OPTIMIZE_OUTPUT_FOR_C YES)
        set(DOXYGEN_EXTRACT_ALL NO)
        set(DOXYGEN_GENERATE_TREEVIEW YES)
        set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_CURRENT_SOURCE_DIR}/docs/mainpage.md")

        doxygen_add_docs(
            cashash_docs
            "${CMAKE_CURRENT_SOURCE_DIR}/include"
            "${CMAKE_CURRENT_SOURCE_DIR}/docs/mainpage.md"
            COMMENT "Generating cashash documentation"
        )
    else()
        message(STATUS "Doxygen not found; documentation target disabled")
    endif()
endif()
