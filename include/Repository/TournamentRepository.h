#pragma once

#include <string>
#include "Core/LinkedList.h"
#include "Tournament/Tournament.h"

class FileManager;

// L? DO CH?N: TournamentRepository qu?n l? vi?c l?u tr? v? truy xu?t c?c gi?i ??u
// t? t?p data/tournaments.txt b?ng LinkedList tu?n t?.
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
