#include "SaveCommand.hpp"
#include "persistence/SnapshotWriter.hpp"

std::string SaveCommand::execute() {
    SnapshotWriter::save();
    return "+OK\r\n";
}
