#include "circular_deque.h"

circular_deque::circular_deque()
{

}

int circular_deque::size()
{
    return sz;
}

void circular_deque::push_back(const std::deque<char> &other)
{
    for (auto c : other)
    {
        data[(beg+(sz++))%MAX_BUFFER_SIZE] = c;
    }
}

std::deque<char> circular_deque::extract_front(int len)
{
    if (sz + len <= MAX_BUFFER_SIZE)
    {
        return std::deque<char>(data+sz, data+sz+len);
    } else
    {

    }
}
