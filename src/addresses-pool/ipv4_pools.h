#include <cstdint>

#include <set>
#include <utility>


// "tt" stands for test task
namespace netup_tt
{
    using IPAddress = std::uint32_t;     
    using Range = std::pair<IPAddress, IPAddress>;
    using Pool = std::set<Range>;

    Pool find_diff(const Pool& old_pool, const Pool& new_pool);
} 
