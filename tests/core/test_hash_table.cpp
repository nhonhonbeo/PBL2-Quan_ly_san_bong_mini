#include <cassert>
#include <iostream>
#include <string>
#include <stdexcept>
#include "Core/HashTable.h"

// Hasher luôn trả về 0 để ép toàn bộ key rơi vào cùng một bucket (kiểm thử va chạm và liên kết bucket)
struct ForceCollisionHasher {
    std::size_t operator()(const std::string&) const {
        return 0;
    }
    std::size_t operator()(int) const {
        return 0;
    }
};

// Tracker đối tượng để kiểm tra rò rỉ bộ nhớ và an toàn ngoại lệ
struct HashLeakTracker {
    static int liveCount;
    static int copyCount;
    static int throwAtCopy;

    int val;

    explicit HashLeakTracker(int v = 0) : val(v) {
        liveCount++;
    }

    HashLeakTracker(const HashLeakTracker& other) : val(other.val) {
        copyCount++;
        if (throwAtCopy > 0 && copyCount >= throwAtCopy) {
            throw std::runtime_error("Simulated copy failure in HashLeakTracker");
        }
        liveCount++;
    }

    HashLeakTracker(HashLeakTracker&& other) noexcept : val(other.val) {
        liveCount++;
    }

    HashLeakTracker& operator=(const HashLeakTracker& other) {
        if (this != &other) {
            copyCount++;
            if (throwAtCopy > 0 && copyCount >= throwAtCopy) {
                throw std::runtime_error("Simulated copy assignment failure in HashLeakTracker");
            }
            val = other.val;
        }
        return *this;
    }

    HashLeakTracker& operator=(HashLeakTracker&& other) noexcept {
        val = other.val;
        return *this;
    }

    ~HashLeakTracker() {
        liveCount--;
    }

    static void reset() {
        liveCount = 0;
        copyCount = 0;
        throwAtCopy = 0;
    }
};

int HashLeakTracker::liveCount = 0;
int HashLeakTracker::copyCount = 0;
int HashLeakTracker::throwAtCopy = 0;

void testEmptyTable() {
    std::cout << "  [RUN] testEmptyTable...\n";
    HashTable<std::string, int> table;
    assert(table.isEmpty());
    assert(table.size() == 0);
    assert(table.loadFactor() == 0.0f);
    assert(table.capacity() >= 4);

    int val = 0;
    assert(!table.search("missing", val));
    assert(table.find("missing") == nullptr);
    assert(!table.contains("missing"));
    assert(!table.remove("missing"));
    assert(table.get_all().isEmpty());

    table.clear();
    assert(table.isEmpty());
}

void testDuplicateKeyAndInPlaceUpdate() {
    std::cout << "  [RUN] testDuplicateKeyAndInPlaceUpdate...\n";
    HashTable<std::string, int> table;
    // Chèn mới trả về true
    assert(table.insert("user1", 100));
    assert(table.size() == 1);

    // Chèn trùng key cập nhật in-place, không tăng size, trả về false
    assert(!table.insert("user1", 200));
    assert(table.size() == 1);
    int val = 0;
    assert(table.search("user1", val) && val == 200);

    // Phương thức update() tường minh
    assert(table.update("user1", 300));
    assert(table.search("user1", val) && val == 300);
    assert(!table.update("non_existent", 0));
}

void testCollisionHandlingAndSameBucket() {
    std::cout << "  [RUN] testCollisionHandlingAndSameBucket...\n";
    // Ép mọi phần tử vào cùng 1 bucket qua ForceCollisionHasher
    HashTable<std::string, int, ForceCollisionHasher> table(8, 2.0f);
    assert(table.insert("k1", 10));
    assert(table.insert("k2", 20));
    assert(table.insert("k3", 30));
    assert(table.insert("k4", 40));
    assert(table.size() == 4);

    // Tìm kiếm chính xác từng key trong danh sách va chạm
    int val = 0;
    assert(table.search("k1", val) && val == 10);
    assert(table.search("k2", val) && val == 20);
    assert(table.search("k3", val) && val == 30);
    assert(table.search("k4", val) && val == 40);
    assert(!table.search("k5", val));
}

void testDeleteHeadMidTailInCollisionBucket() {
    std::cout << "  [RUN] testDeleteHeadMidTailInCollisionBucket...\n";
    // [head: k1, mid: k2, tail: k3]
    HashTable<std::string, int, ForceCollisionHasher> table(8, 2.0f);
    table.insert("k1", 1);
    table.insert("k2", 2);
    table.insert("k3", 3);

    // Xóa giữa (k2)
    assert(table.remove("k2"));
    assert(table.size() == 2);
    int val = 0;
    assert(!table.search("k2", val));
    assert(table.search("k1", val) && val == 1);
    assert(table.search("k3", val) && val == 3);

    // Xóa đầu (k1)
    assert(table.remove("k1"));
    assert(table.size() == 1);
    assert(!table.search("k1", val));
    assert(table.search("k3", val) && val == 3);

    // Xóa cuối (k3)
    assert(table.remove("k3"));
    assert(table.isEmpty());
    assert(table.size() == 0);
    assert(!table.search("k3", val));
}

void testRehashAndDynamicResizing() {
    std::cout << "  [RUN] testRehashAndDynamicResizing...\n";
    // Khởi tạo bảng capacity = 4, maxLoadFactor = 0.75f
    HashTable<int, std::string> table(4, 0.75f);
    assert(table.capacity() == 4);

    table.insert(1, "val1");
    table.insert(2, "val2");
    table.insert(3, "val3"); // 3 / 4 = 0.75 <= 0.75 -> chưa rehash
    assert(table.capacity() == 4);

    table.insert(4, "val4"); // 4 / 4 = 1.0 > 0.75 -> tự động rehash lên 8
    assert(table.capacity() == 8);
    assert(table.size() == 4);

    // Kiểm tra dữ liệu không bị thất thoát sau rehash
    std::string s;
    for (int i = 1; i <= 4; ++i) {
        assert(table.search(i, s) && s == "val" + std::to_string(i));
    }

    // Tiếp tục chèn để rehash lên 16
    for (int i = 5; i <= 10; ++i) {
        table.insert(i, "val" + std::to_string(i));
    }
    assert(table.capacity() >= 16);
    assert(table.size() == 10);
    for (int i = 1; i <= 10; ++i) {
        assert(table.search(i, s) && s == "val" + std::to_string(i));
    }
}

void testGenericKeyTypes() {
    std::cout << "  [RUN] testGenericKeyTypes...\n";
    // 1. Key int
    HashTable<int, int> intTable;
    intTable.insert(100, 200);
    int outVal = 0;
    assert(intTable.search(100, outVal) && outVal == 200);

    // 2. Key chuỗi SĐT (std::string)
    HashTable<std::string, std::size_t> phoneIndex;
    phoneIndex.insert("0901234567", 0);
    phoneIndex.insert("0988776655", 1);
    std::size_t idx = 0;
    assert(phoneIndex.search("0988776655", idx) && idx == 1);

    // 3. Value con trỏ
    int x = 42;
    HashTable<std::string, int*> ptrTable;
    ptrTable.insert("ptrX", &x);
    int* p = nullptr;
    assert(ptrTable.search("ptrX", p) && *p == 42);
}

void testCopySemantics() {
    std::cout << "  [RUN] testCopySemantics...\n";
    HashTable<std::string, int> orig;
    orig.insert("k1", 100);
    orig.insert("k2", 200);

    // Copy constructor (deep copy)
    HashTable<std::string, int> copyTable(orig);
    assert(copyTable.size() == 2);
    int val = 0;
    assert(copyTable.search("k1", val) && val == 100);

    // Sửa orig không ảnh hưởng copyTable
    orig.insert("k3", 300);
    orig.remove("k1");
    assert(!orig.search("k1", val));
    assert(copyTable.search("k1", val) && val == 100);
    assert(!copyTable.search("k3", val));

    // Copy assignment
    HashTable<std::string, int> assignedTable;
    assignedTable.insert("old", 1);
    assignedTable = copyTable;
    assert(assignedTable.size() == 2);
    assert(assignedTable.search("k2", val) && val == 200);

    // Self-assignment
    auto& ref = assignedTable;
    assignedTable = ref;
    assert(assignedTable.size() == 2);
    assert(assignedTable.search("k1", val) && val == 100);
}

void testMoveSemantics() {
    std::cout << "  [RUN] testMoveSemantics...\n";
    HashTable<std::string, int> orig;
    orig.insert("m1", 111);
    orig.insert("m2", 222);

    // Move constructor
    HashTable<std::string, int> movedTable(std::move(orig));
    assert(orig.isEmpty());
    assert(orig.size() == 0);
    assert(orig.capacity() == 0);
    assert(movedTable.size() == 2);
    int val = 0;
    assert(movedTable.search("m1", val) && val == 111);

    // Thao tác tiếp trên orig sau khi move
    orig.insert("newKey", 999);
    assert(orig.size() == 1);
    assert(orig.search("newKey", val) && val == 999);

    // Move assignment
    HashTable<std::string, int> dest;
    dest.insert("destOld", 0);
    dest = std::move(movedTable);
    assert(movedTable.isEmpty());
    assert(dest.size() == 2);
    assert(dest.search("m2", val) && val == 222);

    // Self-move assignment
    auto& destRef = dest;
    dest = std::move(destRef);
    assert(dest.size() == 2);
    assert(dest.search("m1", val) && val == 111);
}

void testLargeDataset() {
    std::cout << "  [RUN] testLargeDataset (10,000 items)...\n";
    HashTable<int, int> table;
    const int N = 10000;
    for (int i = 0; i < N; ++i) {
        table.insert(i, i * 2);
    }
    assert(table.size() == N);
    assert(table.loadFactor() <= table.DEFAULT_MAX_LOAD_FACTOR);

    // Tra cứu lại toàn bộ
    for (int i = 0; i < N; ++i) {
        int v = 0;
        assert(table.search(i, v) && v == i * 2);
    }

    // Xóa một nửa
    for (int i = 0; i < N / 2; ++i) {
        assert(table.remove(i));
    }
    assert(table.size() == N / 2);
    for (int i = 0; i < N / 2; ++i) {
        int v = 0;
        assert(!table.search(i, v));
    }
    for (int i = N / 2; i < N; ++i) {
        int v = 0;
        assert(table.search(i, v) && v == i * 2);
    }
}

void testMemoryLeaksAndExceptionSafety() {
    std::cout << "  [RUN] testMemoryLeaksAndExceptionSafety...\n";
    HashLeakTracker::reset();

    // 1. Kiểm tra dọn sạch bộ nhớ thông thường
    {
        HashTable<int, HashLeakTracker> table;
        for (int i = 0; i < 50; ++i) {
            table.insert(i, HashLeakTracker(i));
        }
        assert(HashLeakTracker::liveCount == 50);
        HashTable<int, HashLeakTracker> copyTable = table;
        assert(HashLeakTracker::liveCount == 100);
    }
    assert(HashLeakTracker::liveCount == 0);

    // 2. Kiểm tra Exception Safety khi copy bảng băm
    {
        HashTable<int, HashLeakTracker> table;
        HashLeakTracker::throwAtCopy = 0;
        table.insert(1, HashLeakTracker(10));
        table.insert(2, HashLeakTracker(20));
        table.insert(3, HashLeakTracker(30));

        HashLeakTracker::copyCount = 0;
        HashLeakTracker::throwAtCopy = 2; // Ném ngoại lệ khi sao chép phần tử thứ 2

        int liveBefore = HashLeakTracker::liveCount;
        bool caught = false;
        try {
            HashTable<int, HashLeakTracker> copyTable(table);
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught);
        assert(HashLeakTracker::liveCount == liveBefore);

        // Chuẩn bị dest cho test copy assignment
        HashLeakTracker::throwAtCopy = 0;
        HashTable<int, HashLeakTracker> dest;
        dest.insert(99, HashLeakTracker(990));
        int destLiveBefore = HashLeakTracker::liveCount;

        HashLeakTracker::copyCount = 0;
        HashLeakTracker::throwAtCopy = 2; // Kích hoạt ngoại lệ khi copy assignment
        caught = false;
        try {
            dest = table;
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught);
        assert(dest.size() == 1);
        assert(dest.find(99)->val == 990);
        assert(HashLeakTracker::liveCount == destLiveBefore);

        HashLeakTracker::throwAtCopy = 0;
    }
    assert(HashLeakTracker::liveCount == 0);
}

int main() {
    std::cout << "=== CORE UNIT TEST: HashTable ===\n";
    testEmptyTable();
    testDuplicateKeyAndInPlaceUpdate();
    testCollisionHandlingAndSameBucket();
    testDeleteHeadMidTailInCollisionBucket();
    testRehashAndDynamicResizing();
    testGenericKeyTypes();
    testCopySemantics();
    testMoveSemantics();
    testLargeDataset();
    testMemoryLeaksAndExceptionSafety();
    std::cout << ">>> HASHTABLE: ALL 10 TEST SUITES PASSED! <<<\n\n";
    return 0;
}