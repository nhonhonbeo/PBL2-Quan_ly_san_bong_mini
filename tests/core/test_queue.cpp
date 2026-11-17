#include <cassert>
#include <iostream>
#include <string>
#include <stdexcept>
#include "Core/Queue.h"

// LÝ DO CHỌN: Tracker đối tượng để kiểm tra rò rỉ bộ nhớ (memory leak) và an toàn ngoại lệ (exception safety) cho Queue.
struct QueueLeakTracker {
    static int liveCount;
    static int copyCount;
    static int throwAtCopy;

    int id;

    explicit QueueLeakTracker(int v = 0) : id(v) {
        liveCount++;
    }

    QueueLeakTracker(const QueueLeakTracker& other) : id(other.id) {
        copyCount++;
        if (throwAtCopy > 0 && copyCount >= throwAtCopy) {
            throw std::runtime_error("Simulated copy failure in QueueLeakTracker");
        }
        liveCount++;
    }

    QueueLeakTracker(QueueLeakTracker&& other) noexcept : id(other.id) {
        liveCount++;
    }

    QueueLeakTracker& operator=(const QueueLeakTracker& other) {
        if (this != &other) {
            copyCount++;
            if (throwAtCopy > 0 && copyCount >= throwAtCopy) {
                throw std::runtime_error("Simulated copy assignment failure in QueueLeakTracker");
            }
            id = other.id;
        }
        return *this;
    }

    QueueLeakTracker& operator=(QueueLeakTracker&& other) noexcept {
        id = other.id;
        return *this;
    }

    ~QueueLeakTracker() {
        liveCount--;
    }

    static void reset() {
        liveCount = 0;
        copyCount = 0;
        throwAtCopy = 0;
    }
};

int QueueLeakTracker::liveCount = 0;
int QueueLeakTracker::copyCount = 0;
int QueueLeakTracker::throwAtCopy = 0;

void testEmptyQueue() {
    std::cout << "  [RUN] testEmptyQueue...\n";
    Queue<int> q;
    assert(q.isEmpty());
    assert(q.size() == 0);
    assert(!q.dequeue()); // Dequeue trên Queue rỗng trả về false an toàn

    bool caughtFront = false;
    try {
        q.front();
    } catch (const std::underflow_error&) {
        caughtFront = true;
    }
    assert(caughtFront);

    bool caughtBack = false;
    try {
        q.back();
    } catch (const std::underflow_error&) {
        caughtBack = true;
    }
    assert(caughtBack);

    q.clear();
    assert(q.isEmpty());
}

void testSingleElementBoundary() {
    std::cout << "  [RUN] testSingleElementBoundary...\n";
    Queue<int> q;
    q.enqueue(42);
    assert(!q.isEmpty());
    assert(q.size() == 1);
    assert(q.front() == 42);
    assert(q.back() == 42);

    assert(q.dequeue());
    assert(q.isEmpty());
    assert(q.size() == 0);
    assert(!q.dequeue());
}

void testFifoOrdering() {
    std::cout << "  [RUN] testFifoOrdering...\n";
    Queue<int> q;
    for (int i = 1; i <= 10; ++i) {
        q.enqueue(i);
    }
    assert(q.size() == 10);
    assert(q.front() == 1);
    assert(q.back() == 10);

    for (int i = 1; i <= 10; ++i) {
        assert(q.front() == i);
        assert(q.dequeue());
    }
    assert(q.isEmpty());
}

void testDrainAndRefill() {
    std::cout << "  [RUN] testDrainAndRefill...\n";
    Queue<std::string> q;
    q.enqueue("first");
    q.enqueue("second");

    assert(q.dequeue());
    assert(q.dequeue());
    assert(q.isEmpty());

    // Nạp lại sau khi rỗng: con trỏ frontPtr và rearPtr phải được cập nhật chính xác
    q.enqueue("reborn1");
    assert(!q.isEmpty());
    assert(q.size() == 1);
    assert(q.front() == "reborn1");
    assert(q.back() == "reborn1");

    q.enqueue("reborn2");
    assert(q.size() == 2);
    assert(q.front() == "reborn1");
    assert(q.back() == "reborn2");
    assert(q.dequeue());
    assert(q.front() == "reborn2");
}

void testCopySemantics() {
    std::cout << "  [RUN] testCopySemantics...\n";
    Queue<int> orig;
    orig.enqueue(10);
    orig.enqueue(20);
    orig.enqueue(30);

    // Copy constructor (deep copy)
    Queue<int> copyQ(orig);
    assert(copyQ.size() == 3);
    assert(copyQ.front() == 10);
    assert(copyQ.back() == 30);

    // Sửa orig không ảnh hưởng copyQ
    orig.dequeue();
    orig.enqueue(40);
    assert(orig.front() == 20);
    assert(orig.back() == 40);
    assert(copyQ.size() == 3);
    assert(copyQ.front() == 10);
    assert(copyQ.back() == 30);

    // Copy assignment
    Queue<int> assignedQ;
    assignedQ.enqueue(999);
    assignedQ = copyQ;
    assert(assignedQ.size() == 3);
    assert(assignedQ.front() == 10);

    // Self-assignment
    auto& ref = assignedQ;
    assignedQ = ref;
    assert(assignedQ.size() == 3);
    assert(assignedQ.front() == 10);
}

void testMoveSemantics() {
    std::cout << "  [RUN] testMoveSemantics...\n";
    Queue<int> orig;
    orig.enqueue(100);
    orig.enqueue(200);

    // Move constructor
    Queue<int> movedQ(std::move(orig));
    assert(orig.isEmpty());
    assert(orig.size() == 0);
    assert(movedQ.size() == 2);
    assert(movedQ.front() == 100);
    assert(movedQ.back() == 200);

    // Thao tác tiếp trên đối tượng nguồn sau khi move
    orig.enqueue(500);
    assert(!orig.isEmpty());
    assert(orig.size() == 1);
    assert(orig.front() == 500);

    // Move assignment
    Queue<int> dest;
    dest.enqueue(1);
    dest = std::move(movedQ);
    assert(movedQ.isEmpty());
    assert(dest.size() == 2);
    assert(dest.front() == 100);

    // Self-move assignment
    auto& ref = dest;
    dest = std::move(ref);
    assert(dest.size() == 2);
    assert(dest.front() == 100);
}

void testInterleavedOperations() {
    std::cout << "  [RUN] testInterleavedOperations...\n";
    Queue<int> q;
    int pushVal = 0;
    int popVal = 0;

    for (int i = 0; i < 1000; ++i) {
        q.enqueue(pushVal++);
        q.enqueue(pushVal++);
        assert(q.front() == popVal);
        assert(q.dequeue());
        popVal++;
    }
    // Đã push 2000, pop 1000 -> còn 1000
    assert(q.size() == 1000);
    while (!q.isEmpty()) {
        assert(q.front() == popVal++);
        assert(q.dequeue());
    }
    assert(popVal == 2000);
    assert(q.isEmpty());
}

void testLargeDataset() {
    std::cout << "  [RUN] testLargeDataset (30,000 items)...\n";
    Queue<int> q;
    const int N = 30000;
    for (int i = 0; i < N; ++i) {
        q.enqueue(i);
    }
    assert(q.size() == N);
    assert(q.front() == 0);
    assert(q.back() == N - 1);

    for (int i = 0; i < N; ++i) {
        assert(q.front() == i);
        assert(q.dequeue());
    }
    assert(q.isEmpty());
}

void testMemoryLeaksAndExceptionSafety() {
    std::cout << "  [RUN] testMemoryLeaksAndExceptionSafety...\n";
    QueueLeakTracker::reset();

    // 1. Kiểm tra dọn sạch bộ nhớ thông thường
    {
        Queue<QueueLeakTracker> q;
        for (int i = 0; i < 50; ++i) {
            q.enqueue(QueueLeakTracker(i));
        }
        assert(QueueLeakTracker::liveCount == 50);
        Queue<QueueLeakTracker> copyQ = q;
        assert(QueueLeakTracker::liveCount == 100);
    }
    assert(QueueLeakTracker::liveCount == 0);

    // 2. Kiểm tra Exception Safety khi copy gặp ngoại lệ
    {
        Queue<QueueLeakTracker> q;
        q.enqueue(QueueLeakTracker(1));
        q.enqueue(QueueLeakTracker(2));
        q.enqueue(QueueLeakTracker(3));

        QueueLeakTracker::copyCount = 0;
        QueueLeakTracker::throwAtCopy = 2; // Ném ngoại lệ khi sao chép Node thứ 2

        int liveBefore = QueueLeakTracker::liveCount;
        bool caught = false;
        try {
            Queue<QueueLeakTracker> copyQ(q);
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught);
        // Không được rò rỉ bất kỳ Node nào
        assert(QueueLeakTracker::liveCount == liveBefore);

        // Strong Exception Guarantee trên copy assignment
        Queue<QueueLeakTracker> dest;
        dest.enqueue(QueueLeakTracker(888));
        int destLiveBefore = QueueLeakTracker::liveCount;

        QueueLeakTracker::copyCount = 0;
        QueueLeakTracker::throwAtCopy = 2;
        caught = false;
        try {
            dest = q;
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught);
        assert(dest.size() == 1);
        assert(dest.front().id == 888);
        assert(QueueLeakTracker::liveCount == destLiveBefore);
    }
    assert(QueueLeakTracker::liveCount == 0);
}

int main() {
    std::cout << "=== CORE UNIT TEST: Queue ===\n";
    testEmptyQueue();
    testSingleElementBoundary();
    testFifoOrdering();
    testDrainAndRefill();
    testCopySemantics();
    testMoveSemantics();
    testInterleavedOperations();
    testLargeDataset();
    testMemoryLeaksAndExceptionSafety();
    std::cout << ">>> QUEUE: ALL 9 TEST SUITES PASSED! <<<\n\n";
    return 0;
}