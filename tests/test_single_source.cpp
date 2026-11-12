#include <cassert>
#include <iostream>
#include <string>

#include "Core/DateTime.h"
#include "Core/LinkedList.h"
#include "Customer/Blacklist.h"
#include "Customer/Customer.h"
#include "Customer/Review.h"
#include "Repository/AccountRepository.h"
#include "Repository/BookingRepository.h"
#include "Repository/CustomerRepository.h"
#include "Repository/TournamentRepository.h"
#include "Repository/VenueRepository.h"
#include "Service/AccountService.h"
#include "Service/BookingService.h"
#include "Service/CustomerService.h"
#include "Service/TournamentService.h"
#include "Service/VenueService.h"
#include "Storage/FileManager.h"
#include "Tournament/Tournament.h"

int main() {
    FileManager fm;
    CustomerRepository custRepo(fm);
    CustomerService custService(custRepo);

    // 1. Test Single Source of Truth for Blacklist:
    std::string err;
    const std::string testPhone = "0999111222";
    assert(custService.addToBlacklist(testPhone, "Vi pham quy che dat san", err));

    bool isBlocked = false;
    std::string reason;
    assert(custService.isBlacklisted(testPhone, isBlocked, reason));
    assert(isBlocked);

    // Fresh repository instance: verifies file persistence on disk
    CustomerRepository freshRepo(fm);
    CustomerService freshCustService(freshRepo);
    bool freshBlocked = false;
    std::string freshReason;
    assert(freshCustService.isBlacklisted(testPhone, freshBlocked, freshReason));
    assert(freshBlocked);
    assert(freshReason == "Vi pham quy che dat san");

    // 2. Test Single Source of Truth for Review:
    Review r("RV-SSOT", testPhone, "S5-01", "", 5, DateTime::today(), "San rat tot", "Nguoi Dung Test");
    assert(custService.addOrUpdateReview(r, err));

    LinkedList<Review> loadedReviews;
    assert(freshRepo.loadAllReviews(loadedReviews, err));
    bool foundReview = false;
    for (auto* n = loadedReviews.get_head(); n; n = n->next) {
        if (n->data.getReviewId() == "RV-SSOT") {
            foundReview = true;
            assert(n->data.getRating() == 5);
            assert(n->data.getComment() == "San rat tot");
            break;
        }
    }
    assert(foundReview);

    // 3. Test Single Source of Truth for Tournament:
    TournamentRepository tournRepo(fm);
    BookingRepository bookRepo(fm);
    TournamentService tournService(tournRepo, bookRepo);

    Tournament t(
        "GD-SSOT", testPhone, "Giai SSOT Cup", "S5-01",
        "20/12/2026", "22/12/2026", "Giai thu nghiem single source of truth",
        TournamentStatus::REQUESTED, "", "Nguoi Dung Test", "18:00 - 20:00"
    );
    std::string outId;
    assert(tournService.guiYeuCauTaoGiai(t, outId, err));

    // Fresh TournamentRepository reload
    TournamentRepository freshTournRepo(fm);
    Tournament loadedT;
    assert(freshTournRepo.findById(outId, loadedT, err));
    assert(loadedT.getName() == "Giai SSOT Cup");
    assert(loadedT.getStatus() == TournamentStatus::REQUESTED);

    // Approve tournament
    assert(tournService.duyetGiaiDau(outId, err));
    assert(freshTournRepo.findById(outId, loadedT, err));
    assert(loadedT.getStatus() == TournamentStatus::CONFIRMED);

    // Clean up test tournament
    assert(tournService.huyGiaiDau(outId, err));
    assert(!freshTournRepo.findById(outId, loadedT, err));

    // Clean up test blacklist & review records
    LinkedList<std::string> bLines;
    fm.readAllLines("data/blacklist.txt", bLines, err);
    bLines.removeIf([&](const std::string& line) {
        return line.rfind(testPhone + "|", 0) == 0;
    });
    fm.writeAllLines("data/blacklist.txt", bLines, err);

    LinkedList<std::string> lines;
    fm.readAllLines("data/reviews.txt", lines, err);
    lines.removeIf([](const std::string& line) {
        return line.rfind("RV-SSOT|", 0) == 0;
    });
    fm.writeAllLines("data/reviews.txt", lines, err);

    std::cout << "ALL SINGLE SOURCE OF TRUTH TESTS PASSED!" << std::endl;
    return 0;
}
