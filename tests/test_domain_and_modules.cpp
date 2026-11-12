#include <cassert>
#include <iostream>
#include <string>

#include "Core/Core.h"
#include "Core/DateTime.h"
#include "Account/Customer.h"
#include "Field/Field.h"
#include "Booking/Booking.h"
#include "Booking/BookingDetail.h"
#include "Tournament/Tournament.h"
#include "Tournament/Team.h"
#include "Tournament/TournamentMatch.h"
#include "Tournament/TournamentResult.h"
#include "Payment/Invoice.h"
#include "Payment/Payment.h"
#include "Storage/FileManager.h"
#include "Repository/PaymentRepository.h"
#include "Repository/TournamentRepository.h"
#include "Repository/CustomerRepository.h"
#include "Service/PaymentService.h"
#include "Service/TournamentService.h"

int main() {
    std::cout << "[TEST] 1. Core runtime self-check...\n";
    assert(Core::runSelfCheck());
    assert(std::string(Core::getVersion()) == "1.0.0-pbl2");

    std::cout << "[TEST] 2. Customer rich domain rules...\n";
    Customer cust("KH-01", "kh01", "123456", "Nguyen Van A", "0901112223");
    cust.setStatistics(6, 4);
    assert(cust.isBlacklistCandidate(3) == true);
    assert(cust.qualifiesForDiscount(5) == true);
    assert(cust.getReliabilityRate() == 60); // 6 / 10 = 60%
    std::string err;
    assert(cust.canBook(true, err) == false);
    assert(cust.canBook(false, err) == true);

    std::cout << "[TEST] 3. Field rich domain rules...\n";
    Field field("S5-01", "US01", "San 5 so 01", FieldSize::FIVE, 200000, "Quy dinh", "06:00", "23:00", FieldStatus::ACTIVE);
    assert(field.isAvailable() == true);
    assert(field.calculatePrice(90) == 300000);
    field.toggleMaintenance();
    assert(field.isUnderMaintenance() == true);
    assert(field.isAvailable() == false);
    field.toggleMaintenance();
    assert(field.isAvailable() == true);

    std::cout << "[TEST] 4. Booking rich domain rules...\n";
    Booking booking("BK-TEST", "KH-01", "US01", "S5-01", BookingStatus::CONFIRMED, "2026-11-10");
    booking.addDetail(BookingDetail("BK-TEST", "S5-01", "18:00", 90, 300000));
    assert(booking.calculateTotalVnd() == 300000);
    assert(booking.canBeCancelledBy("KH-02", err) == false);
    assert(booking.canBeCancelledBy("KH-01", err) == true);
    assert(booking.cancel("KH-01", err) == true);
    assert(booking.isCancelled() == true);
    assert(booking.canBeCompletedBy("US01", err) == false); // Khong the hoan tat don da huy

    std::cout << "[TEST] 5. Tournament, Team, Match domain rules...\n";
    Tournament tour("GD-TEST", "0901112223", "Giai Test", "S5-01", "2026-11-10", "2026-11-15", "", TournamentStatus::REQUESTED);
    assert(tour.isPending() == true);
    assert(tour.conflictsWithDate("2026-11-12") == true);
    assert(tour.conflictsWithDate("2026-11-20") == false);
    tour.confirm("BK-CONFIRMED");
    assert(tour.isApproved() == true);

    Team team1("TM-01", "GD-TEST", "Doi Bong A", "0901112223", 7);
    assert(team1.isValid() == true);

    TournamentMatch match("MTH-01", "GD-TEST", "TM-01", "TM-02", 1, "2026-11-10", "18:00");
    assert(match.isPlayed() == false);
    match.setResult(3, 1);
    assert(match.isPlayed() == true);
    assert(match.getWinnerTeamId() == "TM-01");
    assert(match.isDraw() == false);

    TournamentResult res("MTH-01", 3, 1);
    assert(res.getGoalDifference() == 2);
    assert(res.isHomeWin() == true);
    assert(res.isAwayWin() == false);

    std::cout << "[TEST] 6. Invoice and Payment domain rules...\n";
    Invoice inv("INV-BK-TEST", "BK-TEST", 300000, "2026-11-07", InvoiceStatus::UNPAID);
    assert(inv.canAcceptPayment() == true);
    Payment pay("PAY-01", "INV-BK-TEST", 300000, PaymentMethod::TRANSFER, "2026-11-07");
    assert(pay.isValid() == true);
    assert(inv.matchesPayment(pay) == true);
    assert(pay.matchesInvoice(inv) == true);
    inv.markPaid();
    assert(inv.isPaid() == true);
    assert(inv.canAcceptPayment() == false);

    std::cout << "[TEST] 7. Repository and Service integration for ghost modules...\n";
    FileManager fm;
    PaymentRepository payRepo(fm);
    PaymentService payService(&payRepo);

    std::string newInvId, newPayId;
    assert(payService.createInvoice("BK-UNITTEST", 150000, newInvId, err));
    assert(newInvId == "INV-BK-UNITTEST");
    assert(payService.processPayment(newInvId, 150000, PaymentMethod::CASH, newPayId, err));

    Invoice loadedInv;
    assert(payService.getInvoiceByBookingId("BK-UNITTEST", loadedInv, err));
    assert(loadedInv.isPaid() == true);

    // Dọn dẹp bản ghi kiểm thử
    LinkedList<Invoice> allInvs;
    if (payRepo.loadAllInvoices(allInvs, err)) {
        LinkedList<Invoice> cleanInvs;
        for (auto* n = allInvs.get_head(); n != nullptr; n = n->next) {
            if (n->data.getBookingId() != "BK-UNITTEST") cleanInvs.push_back(n->data);
        }
        payRepo.saveAllInvoices(cleanInvs, err);
    }

    std::cout << ">>> ALL DOMAIN AND MODULE INTEGRATION CHECKS PASSED SUCCESSFULLY! <<<\n";
    return 0;
}