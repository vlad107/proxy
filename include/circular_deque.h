#ifndef CIRCULAR_DEQUE_H
#define CIRCULAR_DEQUE_H
#include <deque>

const int MAX_BUFFER_SIZE = 1<<16;
class circular_deque
{
    char data[MAX_BUFFER_SIZE];
    int beg;
    int sz;
public:
    circular_deque();
    int size();
    void push_back(const std::deque<char> & other);
    std::deque<char> extract_front(int len);
};


#endif // CIRCULAR_DEQUE_H
