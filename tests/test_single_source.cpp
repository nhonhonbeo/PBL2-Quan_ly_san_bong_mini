#include <cassert>
#include <iostream>
#include <string>

#include "Core/DateTime.h"
#include "Core/LinkedList.h"
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

    std::string err;
    const std::string testPhone = "0999111222";
    CustomerRepository freshRepo(fm);

    // Kiểm tra Review được lưu và đọc lại từ cùng một nguồn dữ liệu.
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

    // Kiểm tra yêu cầu Tournament được lưu và đọc lại từ cùng một nguồn dữ liệu.
    TournamentRepository tournRepo(fm);
    VenueRepository venueRepo(fm);
    BookingRepository bookRepo(venueRepo);
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

    // Dọn bản ghi Review được tạo bởi kiểm thử.
    LinkedList<std::string> lines;
    fm.readAllLines("data/reviews.txt", lines, err);
    lines.removeIf([](const std::string& line) {
        return line.rfind("RV-SSOT|", 0) == 0;
    });
    fm.writeAllLines("data/reviews.txt", lines, err);

    std::cout << "ALL SINGLE SOURCE OF TRUTH TESTS PASSED!" << std::endl;
    return 0;
}
