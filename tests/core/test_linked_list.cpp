#include <cassert>
#include <iostream>
#include <string>
#include <stdexcept>
#include <type_traits>
#include "Core/LinkedList.h"

// LÝ DO CHỌN: Tracker đối tượng để kiểm tra rò rỉ bộ nhớ (memory leak) và ngoại lệ (exception safety).
struct LeakTracker {
    static int liveCount;
    static int copyCount;
    static int throwAtCopy;

    int id;

    explicit LeakTracker(int v = 0) : id(v) {
        liveCount++;
    }

    LeakTracker(const LeakTracker& other) : id(other.id) {
        copyCount++;
        if (throwAtCopy > 0 && copyCount >= throwAtCopy) {
            throw std::runtime_error("Simulated copy failure in LeakTracker");
        }
        liveCount++;
    }

    LeakTracker(LeakTracker&& other) noexcept : id(other.id) {
        liveCount++;
    }

    LeakTracker& operator=(const LeakTracker& other) {
        if (this != &other) {
            copyCount++;
            if (throwAtCopy > 0 && copyCount >= throwAtCopy) {
                throw std::runtime_error("Simulated copy assignment failure in LeakTracker");
            }
            id = other.id;
        }
        return *this;
    }

    LeakTracker& operator=(LeakTracker&& other) noexcept {
        id = other.id;
        return *this;
    }

    ~LeakTracker() {
        liveCount--;
    }

    static void reset() {
        liveCount = 0;
        copyCount = 0;
        throwAtCopy = 0;
    }
};

int LeakTracker::liveCount = 0;
int LeakTracker::copyCount = 0;
int LeakTracker::throwAtCopy = 0;

void testEmptyList() {
    std::cout << "  [RUN] testEmptyList...\n";
    LinkedList<int> list;
    assert(list.isEmpty());
    assert(list.size() == 0);
    assert(list.get_head() == nullptr);
    assert(list.get_tail() == nullptr);
    assert(!list.pop_front());
    assert(!list.pop_back());
    assert(!list.remove(42));
    assert(!list.removeIf([](int x) { return x == 42; }));
    assert(!list.removeFirstIf([](int x) { return x == 42; }));
    bool frontThrew = false;
    try {
        (void)list.front();
    } catch (const std::underflow_error&) {
        frontThrew = true;
    }
    assert(frontThrew);
    bool backThrew = false;
    try {
        (void)list.back();
    } catch (const std::underflow_error&) {
        backThrew = true;
    }
    assert(backThrew);
    list.clear();
    assert(list.isEmpty());
    assert(list.size() == 0);
}

void testConstNodeAccess() {
    std::cout << "  [RUN] testConstNodeAccess...\n";
    using MutableHead = decltype(std::declval<LinkedList<int>&>().get_head());
    using MutableTail = decltype(std::declval<LinkedList<int>&>().get_tail());
    using ConstHead = decltype(std::declval<const LinkedList<int>&>().get_head());
    using ConstTail = decltype(std::declval<const LinkedList<int>&>().get_tail());
    using NextNode = decltype(std::declval<LinkedList<int>::Node>().next);
    static_assert(std::is_same<MutableHead, const LinkedList<int>::Node*>::value,
                  "List nodes must be read-only outside LinkedList");
    static_assert(std::is_same<MutableTail, const LinkedList<int>::Node*>::value,
                  "List nodes must be read-only outside LinkedList");
    static_assert(std::is_same<ConstHead, const LinkedList<int>::Node*>::value,
                  "Const lists must expose nodes as read-only");
    static_assert(std::is_same<ConstTail, const LinkedList<int>::Node*>::value,
                  "Const lists must expose nodes as read-only");
    static_assert(std::is_same<NextNode, const LinkedList<int>::Node*>::value,
                  "Node links must not expose mutable nodes");
    static_assert(!std::is_constructible<LinkedList<int>::Node, const int&>::value,
                  "Node construction must remain internal to LinkedList");
}

void testControlledValueMutation() {
    std::cout << "  [RUN] testControlledValueMutation...\n";
    LinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(2);

    int* firstMatch = list.findIf([](int value) { return value == 2; });
    assert(firstMatch != nullptr);
    *firstMatch = 20;
    assert(list.front() == 1);
    assert(list.findIf([](int value) { return value == 20; }) != nullptr);

    assert(list.replaceFirstIf(
        [](int value) { return value == 2; }, 30));
    assert(list.back() == 30);
    assert(list.replaceFirstIf(
        [](int value) { return value == 1; }, 10));
    assert(list.front() == 10);
    assert(!list.replaceFirstIf(
        [](int value) { return value == 99; }, 0));
}

void testSingleElementBoundary() {
    std::cout << "  [RUN] testSingleElementBoundary...\n";
    LinkedList<int> list;

    // Test push_back 1 element
    list.push_back(100);
    assert(!list.isEmpty());
    assert(list.size() == 1);
    assert(list.get_head() == list.get_tail());
    assert(list.front() == 100);
    assert(list.back() == 100);
    assert(list.get_head()->next == nullptr);
    assert(list.pop_front());
    assert(list.isEmpty());
    assert(list.get_head() == nullptr);
    assert(list.get_tail() == nullptr);

    // Test push_front 1 element
    list.push_front(200);
    assert(!list.isEmpty());
    assert(list.size() == 1);
    assert(list.front() == 200);
    assert(list.back() == 200);
    assert(list.pop_back());
    assert(list.isEmpty());
    assert(list.get_head() == nullptr);
    assert(list.get_tail() == nullptr);
}

void testMultipleElementsNormal() {
    std::cout << "  [RUN] testMultipleElementsNormal...\n";
    LinkedList<int> list;
    list.push_back(10);
    list.push_back(20);
    list.push_front(5);
    // Danh sách: [5, 10, 20]
    assert(list.size() == 3);
    assert(list.front() == 5);
    assert(list.back() == 20);
    assert(list.get_head()->data == 5);
    assert(list.get_tail()->data == 20);

    int sum = 0;
    list.forEach([&](int val) { sum += val; });
    assert(sum == 35);
}

void testDeleteHeadMidTail() {
    std::cout << "  [RUN] testDeleteHeadMidTail...\n";
    LinkedList<int> list;
    for (int i = 1; i <= 5; ++i) list.push_back(i); // [1, 2, 3, 4, 5]

    // Xóa giữa (3)
    assert(list.remove(3)); // [1, 2, 4, 5]
    assert(list.size() == 4);
    assert(list.get_head()->data == 1);
    assert(list.get_tail()->data == 5);

    // Xóa đầu (pop_front)
    assert(list.pop_front()); // [2, 4, 5]
    assert(list.size() == 3);
    assert(list.front() == 2);

    // Xóa cuối (pop_back)
    assert(list.pop_back()); // [2, 4]
    assert(list.size() == 2);
    assert(list.back() == 4);
    assert(list.get_tail()->data == 4);
    assert(list.get_tail()->next == nullptr);

    // Xóa nốt các phần tử
    assert(list.remove(2)); // [4]
    assert(list.size() == 1);
    assert(list.get_head() == list.get_tail());
    assert(list.remove(4)); // []
    assert(list.isEmpty());
    assert(list.get_head() == nullptr);
    assert(list.get_tail() == nullptr);
}

void testRemoveIfSemantics() {
    std::cout << "  [RUN] testRemoveIfSemantics...\n";
    LinkedList<int> list;
    // Chèn chuỗi có phần tử trùng ở đầu, giữa và cuối: [9, 9, 1, 9, 2, 9]
    list.push_back(9);
    list.push_back(9);
    list.push_back(1);
    list.push_back(9);
    list.push_back(2);
    list.push_back(9);

    assert(list.removeIf([](int x) { return x == 9; }));
    // Còn lại: [1, 2]
    assert(list.size() == 2);
    assert(list.front() == 1);
    assert(list.back() == 2);
    assert(list.get_head()->data == 1);
    assert(list.get_tail()->data == 2);
    assert(list.get_tail()->next == nullptr);

    // removeFirstIf: chỉ xóa đúng 1 phần tử đầu tiên thỏa mãn
    LinkedList<int> list2;
    list2.push_back(7);
    list2.push_back(7);
    assert(list2.removeFirstIf([](int x) { return x == 7; }));
    assert(list2.size() == 1);
    assert(list2.front() == 7);
}

void testCopySemantics() {
    std::cout << "  [RUN] testCopySemantics...\n";
    LinkedList<std::string> orig;
    orig.push_back("node1");
    orig.push_back("node2");
    orig.push_back("node3");

    // Copy constructor
    LinkedList<std::string> copyList(orig);
    assert(copyList.size() == 3);
    assert(copyList.front() == "node1");
    assert(copyList.back() == "node3");
    assert(copyList.get_head() != orig.get_head());

    // Sửa orig không ảnh hưởng copyList
    orig.pop_front();
    orig.push_back("node4");
    assert(orig.size() == 3);
    assert(orig.front() == "node2");
    assert(copyList.size() == 3);
    assert(copyList.front() == "node1");

    // Copy assignment
    LinkedList<std::string> assignedList;
    assignedList.push_back("old");
    assignedList = copyList;
    assert(assignedList.size() == 3);
    assert(assignedList.front() == "node1");
    assert(assignedList.get_head() != copyList.get_head());

    // Self-assignment
    auto& ref = assignedList;
    assignedList = ref;
    assert(assignedList.size() == 3);
    assert(assignedList.front() == "node1");
}

void testMoveSemantics() {
    std::cout << "  [RUN] testMoveSemantics...\n";
    LinkedList<int> orig;
    orig.push_back(10);
    orig.push_back(20);
    auto* origHead = orig.get_head();
    auto* origTail = orig.get_tail();

    // Move constructor
    LinkedList<int> movedList(std::move(orig));
    assert(orig.isEmpty());
    assert(orig.size() == 0);
    assert(orig.get_head() == nullptr);
    assert(orig.get_tail() == nullptr);

    assert(movedList.size() == 2);
    assert(movedList.get_head() == origHead);
    assert(movedList.get_tail() == origTail);
    assert(movedList.front() == 10);
    assert(movedList.back() == 20);

    // Thao tác tiếp trên orig sau khi bị move
    orig.push_back(999);
    assert(orig.size() == 1);
    assert(orig.front() == 999);

    // Move assignment
    LinkedList<int> dest;
    dest.push_back(1);
    dest = std::move(movedList);
    assert(movedList.isEmpty());
    assert(dest.size() == 2);
    assert(dest.front() == 10);

    // Self-move assignment
    auto& ref = dest;
    dest = std::move(ref);
    assert(dest.size() == 2);
    assert(dest.front() == 10);
}

void testInterleavedOperations() {
    std::cout << "  [RUN] testInterleavedOperations...\n";
    LinkedList<int> list;
    for (int i = 0; i < 500; ++i) {
        list.push_back(i);
        list.push_front(-i);
        if (i % 2 == 0) {
            list.pop_front();
        }
    }
    assert(!list.isEmpty());
    list.clear();
    assert(list.isEmpty());
    assert(list.size() == 0);
    assert(list.get_head() == nullptr);
    assert(list.get_tail() == nullptr);
}

void testLargeDataset() {
    std::cout << "  [RUN] testLargeDataset (20,000 items)...\n";
    LinkedList<int> list;
    const int N = 20000;
    for (int i = 0; i < N; ++i) {
        list.push_back(i);
    }
    assert(list.size() == N);
    assert(list.front() == 0);
    assert(list.back() == N - 1);

    // Pop một nửa từ đầu
    for (int i = 0; i < N / 2; ++i) {
        assert(list.pop_front());
    }
    assert(list.size() == N / 2);
    assert(list.front() == N / 2);

    list.clear();
    assert(list.isEmpty());
}

void testMemoryLeaksAndExceptionSafety() {
    std::cout << "  [RUN] testMemoryLeaksAndExceptionSafety...\n";
    LeakTracker::reset();

    // 1. Kiểm tra không leak khi thao tác thông thường
    {
        LinkedList<LeakTracker> list;
        for (int i = 0; i < 100; ++i) {
            list.push_back(LeakTracker(i));
        }
        assert(LeakTracker::liveCount == 100);
        LinkedList<LeakTracker> copyList = list;
        assert(LeakTracker::liveCount == 200);
    }
    // Ra khỏi scope: toàn bộ 200 đối tượng phải được giải phóng sạch
    assert(LeakTracker::liveCount == 0);

    // 2. Kiểm tra Exception Safety khi cấp phát thất bại giữa chừng
    {
        LinkedList<LeakTracker> list;
        list.push_back(LeakTracker(1));
        list.push_back(LeakTracker(2));
        list.push_back(LeakTracker(3));

        LeakTracker::copyCount = 0;
        LeakTracker::throwAtCopy = 2; // Ném ngoại lệ khi sao chép Node thứ 2

        int liveBefore = LeakTracker::liveCount;
        bool caught = false;
        try {
            LinkedList<LeakTracker> copyList(list);
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught);
        // Node dở dang phải được dọn dẹp sạch, không rò rỉ
        assert(LeakTracker::liveCount == liveBefore);

        // Strong Exception Safety trên copy assignment
        LinkedList<LeakTracker> dest;
        dest.push_back(LeakTracker(999));
        int destLiveBefore = LeakTracker::liveCount;

        LeakTracker::copyCount = 0;
        LeakTracker::throwAtCopy = 2;
        caught = false;
        try {
            dest = list;
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught);
        // dest không bị phá vỡ cấu trúc và dữ liệu cũ
        assert(dest.size() == 1);
        assert(dest.front().id == 999);
        assert(LeakTracker::liveCount == destLiveBefore);
    }
    assert(LeakTracker::liveCount == 0);
}

int main() {
    std::cout << "=== CORE UNIT TEST: LinkedList ===\n";
    testEmptyList();
    testConstNodeAccess();
    testControlledValueMutation();
    testSingleElementBoundary();
    testMultipleElementsNormal();
    testDeleteHeadMidTail();
    testRemoveIfSemantics();
    testCopySemantics();
    testMoveSemantics();
    testInterleavedOperations();
    testLargeDataset();
    testMemoryLeaksAndExceptionSafety();
    std::cout << ">>> LINKEDLIST: ALL 10 TEST SUITES PASSED! <<<\n\n";
    return 0;
}