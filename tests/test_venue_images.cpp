#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "Repository/VenueRepository.h"
#include "Storage/FileManager.h"

int main() {
    namespace fs = std::filesystem;
    const fs::path originalDirectory = fs::current_path();
    const auto nonce = std::chrono::steady_clock::now()
                           .time_since_epoch()
                           .count();
    const fs::path testDirectory =
        fs::temp_directory_path() /
        ("pbl2_venue_images_" + std::to_string(nonce));
    fs::create_directories(testDirectory);
    fs::current_path(testDirectory);

    FileManager fileManager;
    VenueRepository repository(fileManager);
    LinkedList<VenueFieldImageRecord> images;
    images.push_back({"S5-01", "assets/images/field one.jpg"});
    images.push_back({"S7-02", "assets/images/field_two.jpg"});

    std::string error;
    assert(repository.saveFieldImages(images, error));

    LinkedList<VenueFieldImageRecord> loaded;
    assert(repository.loadFieldImages(loaded, error));
    assert(loaded.size() == 2);
    assert(loaded.get_head()->data.fieldId == "S5-01");
    assert(loaded.get_head()->data.path == "assets/images/field one.jpg");
    assert(loaded.get_head()->next->data.fieldId == "S7-02");

    fs::create_directories("data");
    std::ofstream malformed("data/field_images.txt", std::ios::trunc);
    malformed << "missing-separator\n";
    malformed.close();
    assert(!repository.loadFieldImages(loaded, error));
    assert(!error.empty());

    fs::current_path(originalDirectory);
    fs::remove_all(testDirectory);
    std::cout << "[PASS] Field image paths persist through VenueRepository.\n";
    return 0;
}
