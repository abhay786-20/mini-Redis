cmake_minimum_required(VERSION 3.15)
project(mini-redis)

set(CMAKE_CXX_STANDARD 17)

add_executable(mini-redis
    src/main.cpp
    src/store/StoreEngine.cpp
    src/store/types/StringType.cpp
    src/store/types/ListType.cpp
    src/store/types/HashType.cpp
    src/store/types/SetType.cpp
    src/store/types/DataTypeFactory.cpp
    src/commands/CommandDispatcher.cpp
    src/commands/GetCommand.cpp
    src/commands/SetCommand.cpp
)

target_include_directories(mini-redis PRIVATE src)