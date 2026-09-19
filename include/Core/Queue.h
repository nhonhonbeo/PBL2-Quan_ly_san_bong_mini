#ifndef QUEUE_H
#define QUEUE_H
#include<iostream>

template<typename T>
class Queue{
private:
    struct Node{
        T data;
        Node* next;
        Node(const T& value) {
            data = value;
            next = nullptr;
        }    
    };
    Node* frontPtr;
    Node* rearPtr;
    int count;
public:
    Queue(){
        frontPtr = nullptr;
        rearPtr = nullptr;
        count = 0;
    }
    ~Queue() {
        while (!isEmpty()) {
            dequeue();
        }
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

    bool isEmpty() const { 
        return count == 0; 
    }
    int size() const { 
        return count; 
    }
    void dequeue() {
        if (isEmpty()) return;
        Node* temp = frontPtr;
        frontPtr = frontPtr->next;
        if (frontPtr == nullptr) {
            rearPtr = nullptr;
        }
        delete temp;
        count--;
    }
};
#endif