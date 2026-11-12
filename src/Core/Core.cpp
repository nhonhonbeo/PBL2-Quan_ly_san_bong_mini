#include "Core/Core.h"
#include "Core/LinkedList.h"
#include "Core/HashTable.h"
#include "Core/Queue.h"

namespace Core {

const char* getVersion() {
    return "1.0.0-pbl2";
}

bool runSelfCheck() {
    // LÝ DO CHỌN: Tự kiểm tra tính toàn vẹn của LinkedList, HashTable, Queue tại runtime.
    LinkedList<int> list;
    list.push_back(10);
    list.push_back(20);
    if (list.size() != 2) return false;

    HashTable<std::string, int> table;
    table.insert("k1", 100);
    int val = 0;
    if (!table.search("k1", val) || val != 100) return false;

    Queue<int> q;
    q.enqueue(5);
    if (q.isEmpty() || q.size() != 1) return false;
    q.dequeue();
    if (!q.isEmpty()) return false;

    return true;
}

}  // namespace Core
