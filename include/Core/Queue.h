#ifndef QUEUE_H
#define QUEUE_H

#include <cstddef>
#include <stdexcept>
#include <utility>

// LÝ DO CHỌN: Hàng đợi FIFO tự cài đặt bằng danh sách liên kết đơn,
// quản lý bộ nhớ an toàn qua Rule of Five, thao tác enqueue/dequeue đạt O(1).
// ponytail: hàng đợi danh sách liên kết đơn cấp phát động từng node; nâng cấp thành circular buffer (mảng vòng) nếu cần tối ưu cache locality và giảm phân mảnh bộ nhớ.

template <typename T>
class Queue {
private:
    struct Node {
        T data;
        Node* next;

        explicit Node(const T& value, Node* nextNode = nullptr)
            : data(value), next(nextNode) {}

        explicit Node(T&& value, Node* nextNode = nullptr)
            : data(std::move(value)), next(nextNode) {}
    };

    Node* frontPtr;
    Node* rearPtr;
    int count;

public:
    Queue() noexcept
        : frontPtr(nullptr), rearPtr(nullptr), count(0) {}

    // Rule of Five: 1. Destructor
    ~Queue() noexcept {
        clear();
    }

    // Rule of Five: 2. Copy Constructor (Deep copy, Exception-safe)
    // LÝ DO CHỌN: Sao chép sâu toàn bộ phần tử theo thứ tự FIFO, dọn dẹp sạch nếu gặp ngoại lệ giữa chừng.
    Queue(const Queue& other)
        : frontPtr(nullptr), rearPtr(nullptr), count(0) {
        try {
            for (const Node* cur = other.frontPtr; cur != nullptr; cur = cur->next) {
                enqueue(cur->data);
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    // Rule of Five: 3. Copy Assignment Operator (Strong Exception Guarantee & Self-assignment safe)
    // LÝ DO CHỌN: Copy-and-swap idiom tự động an toàn khi tự gán và giữ nguyên trạng thái cũ nếu cấp phát thất bại.
    Queue& operator=(const Queue& other) {
        if (this != &other) {
            Queue temp(other);
            swap(temp);
        }
        return *this;
    }

    // Rule of Five: 4. Move Constructor
    // LÝ DO CHỌN: Tiếp quản trực tiếp các con trỏ O(1), đưa đối tượng nguồn về rỗng an toàn, noexcept.
    Queue(Queue&& other) noexcept
        : frontPtr(other.frontPtr), rearPtr(other.rearPtr), count(other.count) {
        other.frontPtr = nullptr;
        other.rearPtr = nullptr;
        other.count = 0;
    }

    // Rule of Five: 5. Move Assignment Operator
    // LÝ DO CHỌN: Giải phóng dữ liệu hiện tại, chuyển quyền sở hữu con trỏ an toàn khi tự di chuyển.
    Queue& operator=(Queue&& other) noexcept {
        if (this != &other) {
            clear();
            frontPtr = other.frontPtr;
            rearPtr = other.rearPtr;
            count = other.count;
            other.frontPtr = nullptr;
            other.rearPtr = nullptr;
            other.count = 0;
        }
        return *this;
    }

    void swap(Queue& other) noexcept {
        std::swap(frontPtr, other.frontPtr);
        std::swap(rearPtr, other.rearPtr);
        std::swap(count, other.count);
    }

    friend void swap(Queue& a, Queue& b) noexcept {
        a.swap(b);
    }

    void enqueue(const T& value) {
        Node* newNode = new Node(value);
        if (rearPtr == nullptr) {
            frontPtr = rearPtr = newNode;
        } else {
            rearPtr->next = newNode;
            rearPtr = newNode;
        }
        count++;
    }

    void enqueue(T&& value) {
        Node* newNode = new Node(std::move(value));
        if (rearPtr == nullptr) {
            frontPtr = rearPtr = newNode;
        } else {
            rearPtr->next = newNode;
            rearPtr = newNode;
        }
        count++;
    }

    bool dequeue() noexcept {
        if (isEmpty()) {
            return false;
        }
        Node* temp = frontPtr;
        frontPtr = frontPtr->next;
        if (frontPtr == nullptr) {
            rearPtr = nullptr;
        }
        delete temp;
        count--;
        return true;
    }

    bool isEmpty() const noexcept {
        return count == 0;
    }

    int size() const noexcept {
        return count;
    }

    T& front() {
        if (isEmpty()) {
            throw std::underflow_error("Queue is empty: cannot access front()");
        }
        return frontPtr->data;
    }

    const T& front() const {
        if (isEmpty()) {
            throw std::underflow_error("Queue is empty: cannot access front()");
        }
        return frontPtr->data;
    }

    T& back() {
        if (isEmpty()) {
            throw std::underflow_error("Queue is empty: cannot access back()");
        }
        return rearPtr->data;
    }

    const T& back() const {
        if (isEmpty()) {
            throw std::underflow_error("Queue is empty: cannot access back()");
        }
        return rearPtr->data;
    }

    void clear() noexcept {
        Node* cur = frontPtr;
        while (cur != nullptr) {
            Node* next = cur->next;
            delete cur;
            cur = next;
        }
        frontPtr = rearPtr = nullptr;
        count = 0;
    }
};

#endif
