cmake_minimum_required(VERSION 3.1)
project(Snd)

if(WIN32)
	if(${SOUND} STREQUAL "AL")
		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			set(OPENAL_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/OpenAL/include")
			set(OPENAL_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/OpenAL/libs/Win64")
		else(CMAKE_SIZEOF_VOID_P EQUAL 8)
			set(OPENAL_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/OpenAL/include")
			set(OPENAL_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/OpenAL/libs/Win32")
		endif()
	
		set(LIBS OpenAL32)
	endif()
else(WIN32)
    
endif(WIN32)

set(SOURCE_FILES Context.h
                 Context.cpp
                 Log.h)

if(${SOUND} STREQUAL "AL")
	add_definitions(-DUSE_AL_SOUND)
    set(SOURCE_FILES ${SOURCE_FILES}
                     ContextAL.cpp)
endif()

list(APPEND ALL_SOURCE_FILES ${SOURCE_FILES})
set_source_files_properties(${SOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group("src" FILES ${SOURCE_FILES})

list(APPEND ALL_SOURCE_FILES _Snd.cpp)
source_group("src" FILES _Snd.cpp)

add_library(Snd STATIC ${ALL_SOURCE_FILES})
target_link_libraries(Snd ${LIBS})
target_include_directories(Snd PRIVATE ${OPENAL_INCLUDE_DIRS})
target_link_directories(Snd PUBLIC ${OPENAL_LIBRARIES})

add_subdirectory(tests)
