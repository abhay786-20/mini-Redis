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
    src/commands/DelCommand.cpp
    src/commands/LPushCommand.cpp
    src/commands/HSetCommand.cpp
    src/protocol/RespParser.cpp
    src/server/TcpServer.cpp
    src/eviction/LRUPolicy.cpp
    src/store/DataEntry.cpp
    src/eviction/LFUPolicy.cpp
    src/pubsub/PubSubManager.cpp
    src/commands/SubscribeCommand.cpp
    src/commands/UnsubscribeCommand.cpp
    src/commands/PublishCommand.cpp
    src/persistence/AofWriter.cpp
    src/persistence/SnapshotWriter.cpp
    src/commands/SaveCommand.cpp
)


target_include_directories(mini-redis PRIVATE src)