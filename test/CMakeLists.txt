add_executable(parallel-tests)

target_sources(parallel-tests
        PUBLIC FILE_SET HEADERS FILES
        "test_helper.h"
        PRIVATE
        "lock_queue_test.cpp"
)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.17.0
)
FetchContent_MakeAvailable(googletest)

target_link_libraries(parallel-tests PRIVATE queue)
target_link_libraries(parallel-tests PRIVATE GTest::gtest_main)
