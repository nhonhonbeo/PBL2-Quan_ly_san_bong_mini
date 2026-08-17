#ifndef QUEUE_H
#define QUEUE_H

// Hang doi tu cai dat: co the dung cho danh sach cho san (waiting list)
template <typename T>
class Queue {
public:
    Queue();
    ~Queue();
    void enqueue(const T& value);
    bool dequeue(T& out);
    bool rong() const;

private:
    // TODO: em cai dat bang linked list hoac mang vong (circular array)
};

#endif
