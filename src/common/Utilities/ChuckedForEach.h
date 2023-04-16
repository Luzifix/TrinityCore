#ifndef ChunckedForEach_h__
#define ChunckedForEach_h__

#include <utility>
#include <functional>

namespace Trinity
{
    template<typename Iterator>
    void chuncked_for_earch(Iterator begin, Iterator end, size_t interval_size, std::function<void(Iterator, Iterator)> operation)
    {
        auto to = begin;

        while (to != end)
        {
            auto from = to;

            auto counter = interval_size;
            while (counter > 0 && to != end)
            {
                ++to;
                --counter;
            }

            operation(from, to);
        }
    }
}
//! namespace Trinity

#endif // ChunckedForEach_h__
