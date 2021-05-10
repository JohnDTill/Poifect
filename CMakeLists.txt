cmake_minimum_required(VERSION 3.5)

project(HashSearch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DSRC="${CMAKE_SOURCE_DIR}")

add_executable(HashSearch
    main.cpp
    hashutil.h
    hashsearch.h
    hashsearch2.h
    hashbenchmark.h
    #poifect_adhocsymbols.h
    #poifect_adhocsymbols2.h
    #poifect_cppkeywords.h
    #poifect_cppkeywords2.h
    #poifect_greekletters.h
    #poifect_greekletters2.h
)
