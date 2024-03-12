#include "ipv4_pools.h"

#include <algorithm>
#include <optional>


namespace netup_tt
{

    namespace
    {

        std::optional<Range> getNextReducedRange(
            Pool::const_iterator& current, 
            const Pool::const_iterator end
        )
        {
            if (current == end)
            {
                return std::nullopt;
            }

            IPAddress range_first = current->first;
            IPAddress range_last = current->second;

            while (++current != end)
            {
                // Simpler condition like `range_last + 1 < current->first` 
                // doesn't work well when `range_last` equals to maximal value of `IPAddress` type
                if (current->first > range_last && current->first - range_last > 1)
                {
                    break;
                }
                range_last = std::max(range_last, current->second);
            }

            return std::make_optional<Range>(range_first, range_last);
        }

    } // anonymous namespace


    Pool find_diff(const Pool& old_pool, const Pool& new_pool)
    {
        Pool diff;

        auto old_iter = old_pool.cbegin(), new_iter = new_pool.cbegin();
        const auto old_end = old_pool.cend(), new_end = new_pool.cend();
        std::optional<Range> old_range, new_range;
        bool advance_old{true}, advance_new{true};
        std::optional<IPAddress> noncovered_start;

        while (old_iter != old_end || new_iter != new_end)
        {
            if (advance_old)
            {
                old_range = getNextReducedRange(old_iter, old_end);
                if (old_range)
                {
                    noncovered_start = old_range->first;
                }
                else
                {
                    noncovered_start.reset();
                }
                advance_old = false;
            }

            if (advance_new)
            {
                new_range = getNextReducedRange(new_iter, new_end);
                advance_new = false;
            }

            if (!old_range || !new_range)
            {
                break;
            }

            if (old_range->second < new_range->first)
            {
                // Current `old_range` is strictly before current `new_range`. 

                if (noncovered_start)
                {
                    // Example 1:
                    //                  v <- noncovered_start
                    // old: -----[a b c d e]-----------------
                    // new: -[0 1 a b c]--------[k l m n o]----
                    //                          ^ current new_range 
                    // => should add [de] to diff
                    // Example 2:
                    //          v <- noncovered_start
                    // old: ---[a b c d e]-----------------
                    // new: ----------------[k l m n o]----
                    //                      ^ current new_range 
                    // => should add [a..e] to diff
                    diff.emplace(*noncovered_start, old_range->second);
                    noncovered_start.reset();
                }
                advance_old = true;
            }
            else if (new_range->second < old_range->first)
            {
                // Current `new_range` is strictly before current `old_range`. 
                // For example, 
                // old: ----------------[k l m n o]----
                // new: ---[a b c d e]-----------------
                advance_new = true;
            }
            else
            {
                // Current `new_range` and current `old_range` have nonempty intersection

                if (noncovered_start)
                {
                    // Let's add noncovered part. 
                    // Example 1:
                    //                        v <- noncovered_start
                    // old: ---[a b c d e f g h i j k l m n o p]---------------
                    // new: -------[c d e f g]-----[k l m n o p q r s ...]-----
                    //                              ^ current new_range 
                    // => should add [h i j] to diff
                    // Example 2:
                    //          v <- noncovered_start
                    // old: ---[a b c d e f g h i j k l m n o p]-----
                    // new: -------[c d e f g]-----[...]-------------
                    //             ^ current new_range 
                    // => should add [a b] to diff
                    if (*noncovered_start < new_range->first)
                    {
                        diff.emplace(*noncovered_start, new_range->first - 1);
                    }
                    noncovered_start.reset();
                }

                if (new_range->second < old_range->second)
                {
                    // Example 1:
                    // old: ---[a b c d e f g h i j k l m n o p]-----
                    // new: -------[c d e f g]-----[...]-------------
                    //             ^ current new_range 
                    // => should mark h as noncovered_start
                    // 
                    // Example 2:
                    // old: ------[c d e f g h ...]------------
                    // new: --[a b c d e]------[.........]-----
                    // => should mark "f" as noncovered_start
                    noncovered_start = new_range->second + 1;
                    advance_new = true;
                }
                else
                {
                    // Example 1:
                    // old: ---[a b c d e]-----------
                    // new: -------[c d e f g h]----- 
                    // 
                    // Example 2:
                    // old: --------[c d e f g h]----[. . .]-------------
                    // new: ----[a b c d e f g h i j k l m n o ...]------
                    advance_old = true;
                }
            }
        }

        // At this point we may reach end of `new_pool`, but there may be unhandled 
        // ranges in `old_pool`. Let's add them 
        if (noncovered_start)
        {
            diff.emplace(*noncovered_start, old_range->second);
        }
        // parentheses around assignment to `old_range` added to silence clang warning
        while ((old_range = getNextReducedRange(old_iter, old_end)))
        {
            diff.insert(*old_range);
        }

        return diff;
    }

} // namespace netup_tt 
