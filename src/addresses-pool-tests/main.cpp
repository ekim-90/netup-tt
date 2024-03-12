#include <cstdlib>

#include <algorithm>
#include <exception>
#include <iostream>
#include <limits>
#include <random>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "ipv4_pools.h"


namespace
{
    using namespace netup_tt;


    TEST(TestPool, TestAllEmpty) 
    {
        const Pool old_addresses, new_addresses;
        const auto result = find_diff(old_addresses, new_addresses);
        ASSERT_TRUE(result.empty());
    }


    TEST(TestPool, TestOldEmptyNewNonempty) 
    {
        const Pool old_addresses; 
        const Pool new_addresses{{1, 145}, {147, 986}, {2, 123}, {3, 129}, {4, 15}, {600, 600}};
        const auto result = find_diff(old_addresses, new_addresses);
        ASSERT_TRUE(result.empty());
    }


    TEST(TestPool, TestOldNonEmptyNewEmpty) 
    {
        // "irreducible" case: ranges in old_addresses don't intersect
        {
            const Pool old_addresses{{448, 987}, {7, 9}, {1325, 10164}, {93, 93}}; 
            const Pool new_addresses;
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_TRUE(old_addresses == result);
        }

        // "reducible" case: there are intersecting/nested ranges in old_addresses, 
        // they could be reduced to set of non-intersecting ranges. 
        // Also, in this test we check how intersecting/nested intervals are reduced 
        // to set of non-intersecting intervals
        {
            const Pool old_addresses{
                {1, 17}, 
                {6, 12}, 
                {3, 28}, 
                {6, 17}, 
                {2, 145}, 
                {146, 146}, 
                {147, 193}, 
                {331, 689}, 
                {1024, 5532}, 
                {218, 333}, 
                {332, 354}, 
                {195, 218}
            }; 
            const Pool new_addresses;
            const Pool what_result_should_be{{1, 193}, {195, 689}, {1024, 5532}};
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }
    }


    TEST(TestPool, TestOldEqualNew) 
    {
        // In all cases results should be empty

        {
            const Pool old_addresses{{0, 1500}}; 
            const Pool new_addresses(old_addresses);
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_TRUE(result.empty());
        }

        {
            const Pool old_addresses{{448, 987}, {7, 9}, {1325, 10164}, {93, 93}}; 
            const Pool new_addresses(old_addresses);
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_TRUE(result.empty());
        }

        {
            const Pool old_addresses{
                {1, 17}, 
                {6, 12}, 
                {3, 28}, 
                {6, 17}, 
                {2, 145}, 
                {146, 146}, 
                {147, 193}, 
                {331, 689}, 
                {1024, 5532}, 
                {218, 333}, 
                {332, 354}, 
                {195, 218}
            }; 
            const Pool new_addresses(old_addresses);
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_TRUE(result.empty());
        }
    }


    TEST(TestPool, TestOldBeforeNew) 
    {
        {
            // old: ---[* * * * *]----------------------
            // new: ------------------[* * * * * *]-----
            const Pool old_addresses{{0, 4}}; 
            const Pool new_addresses{{10, 20}};
            const Pool what_result_should_be(old_addresses);
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }

        {
            // old: ---[* *]--[* * *]--[* * *]----------------------------------
            // new: ----------------------------[* *]---[* * *]--[* * * * * *]--
            const Pool old_addresses{
                {0, 4}, 
                {34, 93},
                {121, 345}, 
                {389, 715}
            }; 
            const Pool new_addresses{
                {716, 800}, 
                {899, 998},
                {1000, 1000}
            };
            const Pool what_result_should_be(old_addresses);
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }
    }


    TEST(TestPool, TestNewInsideOld) 
    {
        {
            // old: ---[* * * * * * *]---
            // new: -----[* * * *]-------
            const Pool old_addresses{{3, 14}}; 
            const Pool new_addresses{{7, 12}};
            const Pool what_result_should_be{{3, 6}, {13, 14}};
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }

        {
            // old: ---[* * * * * * *]---
            // new: ---[* * * *]---------
            const Pool old_addresses{{3, 14}}; 
            const Pool new_addresses{{3, 12}};
            const Pool what_result_should_be{{13, 14}};
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }

        {
            // old: ---[* * * * * * *]---
            // new: ---------[* * * *]---
            const Pool old_addresses{{3, 14}}; 
            const Pool new_addresses{{7, 14}};
            const Pool what_result_should_be{{3, 6}};
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }

        {
            // old: -----[* * * * * *]---[* * * * * * * *]----[* * * * *]------
            // new: -------[* * *]---------[* * * * *]----------[* * *]--------
            const Pool old_addresses{{1, 37}, {40, 76}, {80, 100}, {200, 300}}; 
            const Pool new_addresses{{10, 20}, {44, 57}, {85, 99}, {233, 287}};
            const Pool what_result_should_be{
                {1, 9}, {21, 37}, 
                {40, 43}, {58, 76}, 
                {80, 84}, {100, 100}, 
                {200, 232}, {288, 300}
            };
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }

        {
            // More complex test with nested/intersecting intervals in "new". 
            // Reduced intervals in "new" are 
            // {1, 193}, {233, 233}, {240, 248}, {261, 303}
            // So reduced configuration looks like:
            // old: -----[* * * * * * * * * * * * * * * * * * * * * * * * * * * * * *]---
            // new: -------[* * *]-----[* * * * *]---[* * *]-------[* * * * * * *]-------
            const Pool old_addresses{{0, 1400}}; 
            const Pool new_addresses{
                {147, 193}, 
                {1, 17}, 
                {146, 146}, 
                {261, 261}, 
                {261, 280}, 
                {233, 233}, 
                {2, 145}, 
                {6, 12}, 
                {267, 303}, 
                {299, 301}, 
                {302, 303}, 
                {240, 248}, 
                {3, 28}, 
                {6, 17}, 
                {302, 302}, 
                {261, 267}, 
                {265, 292}
            };
            const Pool what_result_should_be{
                {0, 0}, 
                {194, 232}, 
                {234, 239}, 
                {249, 260}, 
                {304, 1400}
            };
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }
    }


    TEST(TestPool, TestOldInsideNew) 
    {
        // Cases taken from `TestNewInsideOld`, 
        // but contents of `old_addresses` and `new_addresses` swapped. 
        // And, of course, all results should be empty 

        {
            const Pool old_addresses{{7, 12}}; 
            const Pool new_addresses{{3, 14}};
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_TRUE(result.empty());
        }

        {
            const Pool old_addresses{{3, 12}}; 
            const Pool new_addresses{{3, 14}};
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_TRUE(result.empty());
        }

        {
            const Pool old_addresses{{7, 14}}; 
            const Pool new_addresses{{3, 14}};
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_TRUE(result.empty());
        }

        {
            const Pool old_addresses{{3, 5}, {7, 12}, {14, 20}}; 
            const Pool new_addresses{{3, 20}};
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_TRUE(result.empty());
        }

        {
            const Pool old_addresses{
                {147, 193}, 
                {1, 17}, 
                {146, 146}, 
                {261, 261}, 
                {261, 280}, 
                {233, 233}, 
                {2, 145}, 
                {6, 12}, 
                {267, 303}, 
                {299, 301}, 
                {302, 303}, 
                {240, 248}, 
                {3, 28}, 
                {6, 17}, 
                {302, 302}, 
                {261, 267}, 
                {265, 292}
            }; 
            const Pool new_addresses{
                {0, 1400}
            };
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_TRUE(result.empty());
        }
    }


    TEST(TestPool, TestNewBeforeOld) 
    {
        {
            // old: ------------------[* * * * * *]-----
            // new: ---[* * * * *]----------------------
            const Pool old_addresses{{10, 20}}; 
            const Pool new_addresses{{0, 4}};
            const Pool what_result_should_be(old_addresses);
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }

        {
            // old: ----------------------------[* *]---[* * *]--[* * * * * *]--
            // new: ---[* *]--[* * *]--[* * *]----------------------------------
            const Pool old_addresses{
                {716, 800}, 
                {899, 998},
                {1000, 1000}
            }; 
            const Pool new_addresses{
                {0, 4}, 
                {34, 93},
                {121, 345}, 
                {389, 715}
            };
            const Pool what_result_should_be(old_addresses);
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }
    }


    TEST(TestPool, TestNonintersectingCaseInterleaving) 
    {
        {
            // old: ---[* * *]-[*]--[* * * *]--------
            // new: ---------[*]-[* *]------[* * *]--
            const Pool old_addresses{
                {0, 4}, 
                {6, 6},
                {10, 15}
            }; 
            const Pool new_addresses{
                {5, 5}, 
                {7, 9},
                {16, 199}
            };
            const Pool what_result_should_be(old_addresses);
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }

        {
            // old: ---------[*]-[* *]------[* * *]--
            // new: ---[* * *]-[*]--[* * * *]--------
            const Pool old_addresses{
                {5, 5}, 
                {7, 9},
                {16, 199}
            }; 
            const Pool new_addresses{
                {0, 4}, 
                {6, 6},
                {10, 15}
            };
            const Pool what_result_should_be(old_addresses);
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }
    }


    TEST(TestPool, TestMiscCases) 
    {
        {
            // old: ---------[* * * * * *]-----
            // new: ---[* * * * *]-------------
            const Pool old_addresses{{100, 200}}; 
            const Pool new_addresses{{50, 150}};
            const Pool what_result_should_be{{151, 200}};
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }

        {
            // old: ---[* * * * *]-------------
            // new: ---------[* * * * * *]-----
            const Pool old_addresses{{100, 200}}; 
            const Pool new_addresses{{150, 250}};
            const Pool what_result_should_be{{100, 149}};
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }

        {
            // old: -------[* * * * *]--------[* * * * *]--------[* * * * *]----
            // new: ---[* * * * *]---------[* * * * *]--------[* * * * *]----------
            const Pool old_addresses{{100, 200}, {300, 400}, {500, 600}}; 
            const Pool new_addresses{{50, 150}, {250, 350}, {450, 550}};
            const Pool what_result_should_be{{151, 200}, {351, 400}, {551, 600}};
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }

        {
            // old: ----[* * * * *]----------[* * * * *]----------[* * * * *]-------
            // new: --------[* * * * *]----------[* * * * *]----------[* * * * *]---
            const Pool old_addresses{{100, 200}, {300, 400}, {500, 600}}; 
            const Pool new_addresses{{150, 250}, {350, 450}, {550, 650}};
            const Pool what_result_should_be{{100, 149}, {300, 349}, {500, 549}};
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }

        {
            // old: -----[* * * * * *]---------[* * * * *]--------[* * * * *]-------
            // new: -----------[* * * * *]---------[* * * * *]--------[* * * * *]---
            const Pool old_addresses{{1, 37}, {37, 89}, {80, 100}, {200, 300}}; 
            const Pool new_addresses{{10, 20}, {30, 40}};
            const Pool what_result_should_be{{1, 9}, {21, 29}, {41, 100}, {200, 300}};
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }

        {
            // old: -----[* * * * * *]---[* * * * * * * *]---[* * * * *]----
            // new: -------------[* * * * * *]-------[* * * * * * *]--------
            const Pool old_addresses{{1, 37}, {40, 76}, {80, 100}}; 
            const Pool new_addresses{{10, 50}, {60, 95}};
            const Pool what_result_should_be{{1, 9}, {51, 59}, {96, 100}};
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }

        {
            // old: -----[* * * * * *]-------[* * * * * * *]--------
            // new: ------------[* * * * * * * *]---[* * * * * *]---
            const Pool old_addresses{{10, 50}, {60, 95}}; 
            const Pool new_addresses{{40, 76}, {80, 100}};
            const Pool what_result_should_be{{10, 39}, {77, 79}};
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }


        {
            // old: ----------[* * * * * * * *]---[* * * * * *]--
            // new: -----[* * * * * *]----[* * * * * * *]--------
            const Pool old_addresses{{40, 76}, {80, 100}}; 
            const Pool new_addresses{{10, 50}, {60, 95}};
            const Pool what_result_should_be{{51, 59}, {96, 100}};
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }

        {
            // Reduced configuration looks like: 
            // old: ---[* * * * * * * * * * * * *]  [* * * * * * *]---[* * * * * * * *]--
            // new: -------[* * *]-----[* * *]-------------------------------------------
            const Pool old_addresses{{1, 37}, {37, 89}, {80, 100}, {200, 300}, {400, 1000}}; 
            const Pool new_addresses{{10, 20}, {30, 40}};
            const Pool what_result_should_be{{1, 9}, {21, 29}, {41, 100}, {200, 300}, {400, 1000}};
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }

        {
            // Reduced configuration looks like: 
            // old: ---[* * * * * * * * * * * * *]  [* * * * * * *]----------------------
            // new: -------[* * *]-----[* * *]------------------------[* * * * * * * *]--
            const Pool old_addresses{{1, 37}, {37, 89}, {80, 100}, {200, 300}}; 
            const Pool new_addresses{{10, 20}, {30, 40}, {400, 1000}};
            const Pool what_result_should_be{{1, 9}, {21, 29}, {41, 100}, {200, 300}};
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }

        {
            // Reduced configuration looks like: 
            // ---[* * * * * * * * * * * * *]--------------------------[* * * *]--
            // -------[* * *]---[* * *]-[* * * * *]---[* * * * *]-----------------
            const Pool old_addresses{{1, 37}, {37, 89}, {80, 100}, {200, 300}}; 
            const Pool new_addresses{
                {10, 20}, {30, 40}, {50, 80}, {80, 110}, {50, 110}, {150, 180}
            };
            const Pool what_result_should_be{{1, 9}, {21, 29}, {41, 49}, {200, 300}};
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }

        {
            // Reduced configuration looks like: 
            // ---[* * * * * * * * * * * * *]--------------------------[* * * *]--
            // -------[* * *]---[* * *]-[* * * * *]---[* * * * *]----[* * *]------
            const Pool old_addresses{{1, 37}, {37, 89}, {80, 100}, {200, 300}}; 
            const Pool new_addresses{
                {10, 20}, {30, 40}, {50, 80}, {80, 110}, {50, 110}, {150, 180}, {190, 202}
            };
            const Pool what_result_should_be{{1, 9}, {21, 29}, {41, 49}, {203, 300}};
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }

        {
            // Reduced configuration looks like: 
            // ---[* * * * * * * * * * * * *]--------------------------[* * * * * * * * * * *]--
            // -------[* * *]---[* * *]-[* * * * *]---[* * * * *]----[* * *]---[* * * *]--------
            const Pool old_addresses{{1, 37}, {37, 89}, {80, 100}, {200, 300}}; 
            const Pool new_addresses{
                {10, 20}, {30, 40}, {50, 80}, {80, 110}, {50, 110}, {150, 180}, {190, 202}, {220, 235}
            };
            const Pool what_result_should_be{
                {1, 9}, {21, 29}, {41, 49}, {203, 219}, {236, 300}
            };
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }

        {
            // Reduced configuration looks like: 
            // ---[* * * * * * * * * * * * *]--------------------------[* * * * * * * * * * * * *]------------
            // -------[* * *]---[* * *]-[* * * * *]---[* * * * *]----[* * *]---[* * * *]--[* *][* * * * * *]--
            const Pool old_addresses{{1, 37}, {37, 89}, {80, 100}, {200, 300}}; 
            const Pool new_addresses{
                {10, 20}, {30, 40}, {50, 80}, {80, 110}, {50, 110}, {150, 180}, 
                {190, 202}, {220, 235}, {280, 299}, {300, 325}
            };
            const Pool what_result_should_be{
                {1, 9}, {21, 29}, {41, 49}, {203, 219}, {236, 279}
            };
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }
    }


    TEST(TestPool, TestLimitCases) 
    {
        constexpr auto lower_limit = std::numeric_limits<IPAddress>::min();
        constexpr auto upper_limit = std::numeric_limits<IPAddress>::max();

        {
            const Pool old_addresses{
                {lower_limit, lower_limit}, 
                {lower_limit, 1},
                {10, 50}, 
                {100, upper_limit}, 
                {upper_limit, upper_limit}
            }; 
            const Pool new_addresses;
            const Pool what_result_should_be{
                {lower_limit, 1}, 
                {10, 50}, 
                {100, upper_limit}
            };
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }

        {
            const Pool old_addresses{
                {lower_limit, lower_limit}, 
                {lower_limit, 1}, 
                {10, 10}, 
                {10, 50}, 
                {50, 50}, 
                {100, upper_limit}, 
                {upper_limit, upper_limit}
            }; 
            const Pool new_addresses{
                {lower_limit, 2}, 
                {11, 49}, 
                {60, 70}, 
                {80, 90}, 
                {100, 110}, 
                {120, 130}, 
                {140, 150}, 
                {160, upper_limit}, 
                {170, upper_limit}
            };
            const Pool what_result_should_be{
                {10, 10}, {50, 50}, {111, 119}, {131, 139}, {151, 159}
            };
            const auto result = find_diff(old_addresses, new_addresses);
            ASSERT_EQ(what_result_should_be, result);
        }
    }


    TEST(TestPool, TestGeneric_0) 
    {
        const Pool old_addresses{
            {0, 4}, 
            {6, 10}, 
            {12, 345}, 
            {845, 920}, 
            {1300, 1300}, 
            {9456, 77800}, 
            {99000, 99000}, 
            {123889, 731456}
        }; 
        const Pool new_addresses{
            {45, 98}, 
            {99, 115}, 
            {117, 200}, 
            {845, 860}, 
            {879, 10000}
        };
        const Pool what_result_should_be{
            {0, 4}, 
            {6, 10}, 
            {12, 44}, 
            {116, 116}, 
            {201, 345}, 
            {861, 878}, 
            {10001, 77800}, 
            {99000, 99000}, 
            {123889, 731456}
        };
        const auto result = find_diff(old_addresses, new_addresses);
        ASSERT_EQ(what_result_should_be, result);
    }


    TEST(TestPool, TestGeneric_1) 
    {
        const Pool old_addresses{
            {153, 212}, 
            {512, 630}, 
            {815, 938}, 
            {940, 941}
        }; 
        const Pool new_addresses{
            {17, 38}, 
            {45, 66}, 
            {98, 153}, 
            {212, 344}, 
            {500, 700}, 
            {939, 940}, 
            {941, 1000},
            {1200, 1500}, 
            {2000, 3000}
        };
        const Pool what_result_should_be{
            {154, 211}, 
            {815, 938}
        };
        const auto result = find_diff(old_addresses, new_addresses);
        ASSERT_EQ(what_result_should_be, result);
    }


    std::vector<bool> paintRanges(const Pool& pool, std::vector<bool>& mask, const bool color)
    {
        for (const auto& range : pool)
        {
            for (std::size_t i = range.first; i <= range.second; ++i)
            {
                mask.at(i) = color;
            }
        }
        return mask;
    }


    Pool makeRandomFilledPool(
        std::uniform_int_distribution<IPAddress>& range_start_distribution, 
        std::uniform_int_distribution<IPAddress>& range_length_distribution, 
        std::mt19937& generator, 
        const IPAddress range_last_max, 
        const std::size_t pool_size
    )
    {
        Pool pool;
        while (pool.size() < pool_size)
        {
            const auto range_start = range_start_distribution(generator);
            const auto range_length = std::min(
                range_last_max - range_start + 1, 
                range_length_distribution(generator)
            );
            pool.emplace(range_start, range_start + range_length - 1);
        }
        return pool;
    }


    TEST(TestPool, PerformRandomizedTests) 
    {
        const std::vector<std::size_t> seeds{9055234, 783423, 112348, 8682340, 2096436};

        struct TestParams
        {
            IPAddress mask_size;
            IPAddress range_max_len;
            std::size_t old_pool_size;
            std::size_t new_pool_size;
        };

        const std::vector<TestParams> tests_params
        {
            {1000, 40, 40, 40},
            {10'000, 50, 300, 100},
            {10'000, 10'000, 2000, 1}, 
            {10'000, 10'000, 2000, 10},
            {10'000, 50, 2000, 100},
            {10'000, 100, 2000, 1000}
        };

        for (const auto seed : seeds)
        {
            for (const auto& params : tests_params)
            {
                std::mt19937 gen(seed); 
                std::uniform_int_distribution<IPAddress> range_start_distribution(0, params.mask_size - 1);
                std::uniform_int_distribution<IPAddress> range_length_distribution(1, params.range_max_len);

                std::vector<bool> what_mask_should_be(params.mask_size, false);
                const Pool old_pool = makeRandomFilledPool(
                    range_start_distribution, 
                    range_length_distribution, 
                    gen, 
                    params.mask_size - 1, 
                    params.old_pool_size
                );
                paintRanges(old_pool, what_mask_should_be, true);
                const Pool new_pool = makeRandomFilledPool(
                    range_start_distribution, 
                    range_length_distribution, 
                    gen, 
                    params.mask_size - 1, 
                    params.new_pool_size
                );
                paintRanges(new_pool, what_mask_should_be, false);

                std::vector<bool> mask(params.mask_size, false);
                const auto diff = find_diff(old_pool, new_pool);
                paintRanges(diff, mask, true);

                ASSERT_EQ(what_mask_should_be, mask);
            }
        }
    }

} // anonymous namespace


int main(int argc, char** argv)
try
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
catch (const std::exception& ex)
{
    std::cerr << "Exception has been thrown: " << ex.what() << '\n';
    return EXIT_FAILURE;
}
catch (...)
{
    std::cerr << "Unknown exception has been thrown\n";
    return EXIT_FAILURE;
}
