#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <cstddef>
#include <stdexcept>
#include <utility>

// LÝ DO CHỌN: Danh sách liên kết đơn tự cài đặt, không phụ thuộc std::list,
// quản lý bộ nhớ an toàn qua Rule of Five, O(1) chèn đầu/cuối nhờ con trỏ tail.
// ponytail: danh sách liên kết đơn chưa hỗ trợ iterator hai chiều; nâng cấp thành doubly-linked list nếu nghiệp vụ cần duyệt ngược.

template <typename T>
class LinkedList {
public:
    struct Node {
        T data;
        const Node* next;

    private:
        friend class LinkedList;
        explicit Node(const T& value, Node* nextNode = nullptr)
            : data(value), next(nextNode) {}

        explicit Node(T&& value, Node* nextNode = nullptr)
            : data(std::move(value)), next(nextNode) {}
    };

private:
    Node* head;
    Node* tail;
    int count;

    // LÝ DO CHỌN: Mọi nút trong chuỗi đều do LinkedList cấp phát và sở hữu;
    // chỉ thao tác nội bộ mới chuyển liên kết chỉ-đọc về con trỏ sở hữu để sửa chuỗi.
    static Node* mutableNode(const Node* node) noexcept {
        return const_cast<Node*>(node);
    }

public:
    LinkedList() noexcept
        : head(nullptr), tail(nullptr), count(0) {}

    // Rule of Five: 1. Destructor
    ~LinkedList() noexcept {
        clear();
    }

    // Rule of Five: 2. Copy Constructor (Deep copy, Exception-safe)
    // LÝ DO CHỌN: Cấp phát sâu từng Node, dọn sạch tài nguyên nếu có lỗi giữa chừng để chống rò rỉ bộ nhớ.
    LinkedList(const LinkedList& other)
        : head(nullptr), tail(nullptr), count(0) {
        try {
            for (const Node* cur = other.head; cur != nullptr; cur = cur->next) {
                push_back(cur->data);
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    // Rule of Five: 3. Copy Assignment Operator (Strong Exception Guarantee & Self-assignment safe)
    // LÝ DO CHỌN: Copy-and-swap idiom tự động an toàn khi tự gán và giữ nguyên trạng thái cũ nếu cấp phát thất bại.
    LinkedList& operator=(const LinkedList& other) {
        if (this != &other) {
            LinkedList temp(other);
            swap(temp);
        }
        return *this;
    }

    // Rule of Five: 4. Move Constructor
    // LÝ DO CHỌN: Tiếp quản con trỏ O(1) không cấp phát mới, đối tượng nguồn về trạng thái rỗng an toàn.
    LinkedList(LinkedList&& other) noexcept
        : head(other.head), tail(other.tail), count(other.count) {
        other.head = nullptr;
        other.tail = nullptr;
        other.count = 0;
    }

    // Rule of Five: 5. Move Assignment Operator
    // LÝ DO CHỌN: Giải phóng bộ nhớ hiện tại và chuyển quyền sở hữu con trỏ an toàn khi tự di chuyển.
    LinkedList& operator=(LinkedList&& other) noexcept {
        if (this != &other) {
            clear();
            head = other.head;
            tail = other.tail;
            count = other.count;
            other.head = nullptr;
            other.tail = nullptr;
            other.count = 0;
        }
        return *this;
    }

    void swap(LinkedList& other) noexcept {
        std::swap(head, other.head);
        std::swap(tail, other.tail);
        std::swap(count, other.count);
    }

    friend void swap(LinkedList& a, LinkedList& b) noexcept {
        a.swap(b);
    }

    void push_back(const T& value) {
        Node* newNode = new Node(value);
        if (tail == nullptr) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        count++;
    }

    void push_back(T&& value) {
        Node* newNode = new Node(std::move(value));
        if (tail == nullptr) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        count++;
    }

    void push_front(const T& value) {
        Node* newNode = new Node(value, head);
        head = newNode;
        if (tail == nullptr) {
            tail = newNode;
        }
        count++;
    }

    void push_front(T&& value) {
        Node* newNode = new Node(std::move(value), head);
        head = newNode;
        if (tail == nullptr) {
            tail = newNode;
        }
        count++;
    }

    bool pop_front() noexcept {
        if (head == nullptr) {
            return false;
        }
        Node* old = head;
        head = mutableNode(head->next);
        if (head == nullptr) {
            tail = nullptr;
        }
        delete old;
        count--;
        return true;
    }

    bool pop_back() noexcept {
        if (head == nullptr) {
            return false;
        }
        if (head == tail) {
            delete head;
            head = tail = nullptr;
            count = 0;
            return true;
        }
        Node* cur = head;
        while (cur->next != tail) {
            cur = mutableNode(cur->next);
        }
        delete tail;
        tail = cur;
        tail->next = nullptr;
        count--;
        return true;
    }

    bool isEmpty() const noexcept {
        return count == 0;
    }

    int size() const noexcept {
        return count;
    }

    const Node* get_head() noexcept {
        return head;
    }

    const Node* get_head() const noexcept {
        return head;
    }

    const Node* get_tail() noexcept {
        return tail;
    }

    const Node* get_tail() const noexcept {
        return tail;
    }

    T& front() {
        if (head == nullptr) {
            throw std::underflow_error("LinkedList is empty: cannot access front()");
        }
        return head->data;
    }

    const T& front() const {
        if (head == nullptr) {
            throw std::underflow_error("LinkedList is empty: cannot access front()");
        }
        return head->data;
    }

    T& back() {
        if (tail == nullptr) {
            throw std::underflow_error("LinkedList is empty: cannot access back()");
        }
        return tail->data;
    }

    const T& back() const {
        if (tail == nullptr) {
            throw std::underflow_error("LinkedList is empty: cannot access back()");
        }
        return tail->data;
    }

    void clear() noexcept {
        Node* cur = head;
        while (cur != nullptr) {
            Node* next = mutableNode(cur->next);
            delete cur;
            cur = next;
        }
        head = tail = nullptr;
        count = 0;
    }

    template <typename Func>
    void forEach(Func f) {
        Node* cur = head;
        while (cur != nullptr) {
            f(cur->data);
            cur = mutableNode(cur->next);
        }
    }

    template <typename Func>
    void forEach(Func f) const {
        const Node* cur = head;
        while (cur != nullptr) {
            f(cur->data);
            cur = cur->next;
        }
    }

    // Xóa tất cả các phần tử thỏa mãn predicate (ngữ nghĩa remove_if chuẩn).
    // LÝ DO CHỌN: Cập nhật đồng bộ head, tail và count, trả về true nếu có ít nhất một phần tử bị xóa.
    template <typename Predicate>
    bool removeIf(Predicate pred) {
        bool removedAny = false;
        while (head != nullptr && pred(head->data)) {
            Node* old = head;
            head = mutableNode(head->next);
            delete old;
            count--;
            removedAny = true;
        }
        if (head == nullptr) {
            tail = nullptr;
            return removedAny;
        }
        Node* prev = head;
        Node* cur = mutableNode(head->next);
        while (cur != nullptr) {
            if (pred(cur->data)) {
                prev->next = cur->next;
                if (cur == tail) {
                    tail = prev;
                }
                delete cur;
                count--;
                removedAny = true;
                cur = mutableNode(prev->next);
            } else {
                prev = cur;
                cur = mutableNode(cur->next);
            }
        }
        return removedAny;
    }

    // Xóa phần tử đầu tiên thỏa mãn predicate.
    template <typename Predicate>
    bool removeFirstIf(Predicate pred) {
        if (head == nullptr) return false;
        if (pred(head->data)) {
            return pop_front();
        }
        Node* prev = head;
        Node* cur = mutableNode(head->next);
        while (cur != nullptr) {
            if (pred(cur->data)) {
                prev->next = cur->next;
                if (cur == tail) {
                    tail = prev;
                }
                delete cur;
                count--;
                return true;
            }
            prev = cur;
            cur = mutableNode(cur->next);
        }
        return false;
    }

    template <typename Predicate>
    T* findIf(Predicate pred) {
        for (Node* cur = head; cur != nullptr; cur = mutableNode(cur->next)) {
            if (pred(cur->data)) return &cur->data;
        }
        return nullptr;
    }

    template <typename Predicate>
    const T* findIf(Predicate pred) const {
        for (const Node* cur = head; cur != nullptr; cur = cur->next) {
            if (pred(cur->data)) return &cur->data;
        }
        return nullptr;
    }

    template <typename Predicate, typename Value>
    bool replaceFirstIf(Predicate pred, Value&& value) {
        T* current = findIf(pred);
        if (current == nullptr) return false;
        *current = std::forward<Value>(value);
        return true;
    }

    bool remove(const T& value) {
        return removeIf([&](const T& data) { return data == value; });
    }

    bool removeFirst(const T& value) {
        return removeFirstIf([&](const T& data) { return data == value; });
    }

    bool delete_val(const T& value) {
        return remove(value);
    }
};

#endif
