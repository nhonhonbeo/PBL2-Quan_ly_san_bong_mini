#ifndef LINKEDLIST_H
#define LINKEDLIST_H

// Danh sach lien ket don, tu cai dat, dung chung cho toan he thong
// Vi du: luu danh sach san, danh sach danh gia, danh sach den...
template <typename T>
class LinkedList {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& d) : data(d), next(nullptr) {}
    };
    Node* head;
    int soLuong;

public:
    LinkedList() : head(nullptr), soLuong(0) {}
    ~LinkedList();

    void themCuoi(const T& value);
    bool xoa(const T& value);
    bool timKiem(const T& value) const;
    int laySoLuong() const { return soLuong; }
    bool rong() const { return head == nullptr; }

    // TODO: em bo sung iterator hoac ham duyet (forEach) de lay du lieu ra ngoai
};

#endif
