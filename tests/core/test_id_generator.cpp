#include <cassert>
#include "Core/IdGenerator.h"
int main() {
    IdGenerator ids("BK-", 6);
    ids.observe("BK-000042");
    ids.observe("BK-000004");
    ids.observe("BK-bad");
    assert(ids.next() == "BK-000043");
    assert(ids.next() == "BK-000044");
}
