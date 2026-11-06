#pragma once

#include <string>
#include "Core/LinkedList.h"
#include "Tournament/Tournament.h"

class FileManager;

// LÝ DO CHỌN: TournamentRepository quản lý việc lưu trữ và truy xuất các giải đấu
// từ tệp data/tournaments.txt bằng LinkedList tuần tự.
class TournamentRepository {
private:
    FileManager& file_manager_;

public:
    explicit TournamentRepository(FileManager& fileManager);

    bool loadAll(LinkedList<Tournament>& tournaments, std::string& error) const;
    bool saveAll(const LinkedList<Tournament>& tournaments, std::string& error) const;
    bool save(const Tournament& tournament, std::string& error);
    bool update(const Tournament& tournament, std::string& error);
    bool remove(const std::string& tournamentId, std::string& error);
    bool findById(const std::string& tournamentId, Tournament& out, std::string& error) const;
};
