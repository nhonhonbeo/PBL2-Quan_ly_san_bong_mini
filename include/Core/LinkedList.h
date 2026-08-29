#ifndef LINKEDLIST_H
#define LINKEDLIST_H
#include <iostream>

template <typename T>

class LinkedList{
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& value) {
            data = value;
            next = nullptr;
        }
    };
    Node* head;
    Node* tail;
    int count;
public:
    LinkedList{
        head = nullptr;
        tail = nullptr;
        count = 0;
    }
    ~LinkList(){
        clear();
    }

    void pushBack(const T& value){
        Node *newNode = new Node(value);
        if(tail == nullptr){
            head = tail = newNode;
        }else{
            tail->next == newNode;
            tail = newNode;
        }
        count++;
    }

    bool isEmpty() const{
        return count == 0;
    }
    int size() const{
        return count;
    }
    void clear{
        Node *cur = head;
        while(cur != nullptr){
            Node *next = cur->next
            delete cur;
            cur = next;
        }
        head = tail = nullptr;
        count = 0;
    }

    template<typename Func> 
    void forEach(Func f) const{
        Node* cur = head;
        while(cur!=null){
            f(cur->data);
            cur = cur->next;
        }
    }

    template <typename Predicate>
    bool removeIf(Predicate pred) {
        if(head == nullptr) return false;
        if(pred(head->data)){
            Node* old = head;
            head = head->next;
            if(head == nullptr) tail = nullptr;
            delete old;
            count--;
            return true;
        }
        Node* cur = head->next;
        Node* prev = head;
        while(cur != nullptr){
            if(pred(cur->data)){
                prev->next = cur->next;
                if(cur == tail) tail = prev;
                delete cur;
                count--;
                return true;
            }
        }
        return false;
    }
};