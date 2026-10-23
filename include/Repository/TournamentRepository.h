#pragma once
#include <string>
class Tournament; class FileManager;
class TournamentRepository {
    FileManager& file_manager_;
public:
    explicit TournamentRepository(FileManager& fileManager);
    bool save(const Tournament& tournament, std::string& error);
    bool update(const Tournament& tournament, std::string& error);
};
