cmake_minimum_required(VERSION 3.16)
project(netup-test-task LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(GTest CONFIG REQUIRED)


set(AddressesPoolTargetName "AddressesPool")
add_library(${AddressesPoolTargetName}
    src/addresses-pool/ipv4_pools.h
    src/addresses-pool/ipv4_pools.cpp
)


set(AddressesPoolTestsTargetName "AddressesPoolTests")
add_executable(${AddressesPoolTestsTargetName} 
    src/addresses-pool-tests/main.cpp 
)
target_link_libraries(${AddressesPoolTestsTargetName} 
    PRIVATE ${AddressesPoolTargetName} 
    PRIVATE GTest::gtest GTest::gtest_main
)
target_include_directories(${AddressesPoolTestsTargetName}  
    PRIVATE src/addresses-pool/
)
