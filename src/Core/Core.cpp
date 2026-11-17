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

    // Kiểm tra Rule of Five và thao tác của LinkedList
    LinkedList<int> copyList = list;
    if (copyList.size() != 2 || copyList.front() != 10) return false;
    copyList.pop_front();
    if (copyList.size() != 1 || copyList.front() != 20) return false;
    LinkedList<int> moveList = std::move(copyList);
    if (moveList.size() != 1 || !copyList.isEmpty()) return false;

    // Kiểm tra Rule of Five và thao tác của HashTable
    HashTable<std::string, int> table;
    table.insert("k1", 100);
    table.insert("k2", 200);
    int val = 0;
    if (!table.search("k1", val) || val != 100) return false;
    HashTable<std::string, int> copyTable = table;
    if (copyTable.size() != 2 || !copyTable.search("k2", val) || val != 200) return false;
    copyTable.remove("k1");
    if (copyTable.size() != 1 || copyTable.search("k1", val)) return false;
    HashTable<std::string, int> moveTable = std::move(copyTable);
    if (moveTable.size() != 1 || !copyTable.isEmpty()) return false;

    // Kiểm tra Rule of Five và thao tác của Queue
    Queue<int> q;
    q.enqueue(5);
    q.enqueue(15);
    if (q.isEmpty() || q.size() != 2) return false;
    if (q.front() != 5 || q.back() != 15) return false;
    Queue<int> copyQ = q;
    if (copyQ.size() != 2 || copyQ.front() != 5) return false;
    copyQ.dequeue();
    if (copyQ.front() != 15) return false;
    Queue<int> moveQ = std::move(copyQ);
    if (moveQ.size() != 1 || !copyQ.isEmpty()) return false;
    q.clear();
    if (!q.isEmpty()) return false;

    return true;
}

}  // namespace Core