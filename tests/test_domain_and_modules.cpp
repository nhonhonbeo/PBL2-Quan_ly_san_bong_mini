#include "Account/Owner.h"
#include "Account/Customer.h"
#include "Booking/Booking.h"
#include "Booking/BookingDetail.h"
#include "Core/Core.h"
#include "Field/Field.h"
#include "Tournament/Tournament.h"
#include "Tournament/TournamentMatch.h"

#include <cassert>
#include <iostream>
#include <string>

int main() {
    std::cout << "[TEST] Kiểm tra Core...\n";
    assert(Core::runSelfCheck());
    assert(std::string(Core::getVersion()) == "1.0.0-pbl2");

    std::cout << "[TEST] Kiểm tra Owner và Customer...\n";
    Owner owner("US01", "owner01", "123456", "Nguyen Van Chu", "0901234567",
                "Cum san Sao Vang", "123 Nguyen Hue", "Quy dinh", "06:00",
                "23:00", "Nguyen Van Chu");
    assert(owner.getFacilityConfig().facility_name == "Cum san Sao Vang");
    assert(owner.allows60MinuteSlots());

    Customer customer("KH-01", "kh01", "123456", "Nguyen Van A", "0901112223");
    customer.setStatistics(6, 4);
    assert(customer.getReliabilityRate() == 60);
    std::string error;

    std::cout << "[TEST] Kiểm tra Field...\n";
    Field field("S5-01", "US01", "San 5 so 01", FieldSize::FIVE, 200000,
                "Quy dinh", "06:00", "23:00", FieldStatus::ACTIVE);
    assert(field.isAvailable());
    assert(field.calculatePrice(90) == 300000);
    field.toggleMaintenance();
    assert(field.isUnderMaintenance());
    assert(!field.isAvailable());

    std::cout << "[TEST] Kiểm tra Booking...\n";
    Booking booking("BK-TEST", "KH-01", "US01", "S5-01",
                    BookingStatus::CONFIRMED, "2026-11-10");
    booking.addDetail(BookingDetail("BK-TEST", "S5-01", "18:00", 90, 300000));
    assert(booking.calculateTotalVnd() == 300000);
    assert(!booking.canBeCancelledBy("KH-02", error));
    assert(booking.cancel("KH-01", error));
    assert(booking.isCancelled());
    assert(!booking.canBeCompletedBy("US01", error));

    std::cout << "[TEST] Kiểm tra Tournament và Match...\n";
    Tournament tournament("GD-TEST", "0901112223", "Giai Test", "S5-01",
                          "2026-11-10", "2026-11-15", "",
                          TournamentStatus::REQUESTED);
    assert(tournament.isPending());
    assert(tournament.conflictsWithDate("2026-11-12"));
    assert(!tournament.conflictsWithDate("2026-11-20"));
    tournament.confirm("BK-CONFIRMED");
    assert(tournament.isApproved());

    TournamentMatch match("MTH-01", "GD-TEST", "TM-01", "TM-02",
                          1, "2026-11-10", "18:00");
    assert(!match.isPlayed());
    match.setResult(3, 1);
    assert(match.isPlayed());
    assert(match.getWinnerTeamId() == "TM-01");
    assert(!match.isDraw());

    std::cout << "ALL DOMAIN TESTS PASSED!\n";
    return 0;
}
