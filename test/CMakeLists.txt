add_executable(tests)

target_sources(tests
        PUBLIC FILE_SET HEADERS FILES
        "test_helper.h"
        PRIVATE
        "lock_queue_test.cpp"
        "lock_free_stack_test.cpp"
        "lock_free_queue_test.cpp"
        "simple_coroutines_test.cpp"
        "coro_subscribe_test.cpp"
)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.17.0
)
FetchContent_MakeAvailable(googletest)

target_link_libraries(tests PRIVATE queue stack coroutines)
target_link_libraries(tests PRIVATE GTest::gtest_main)
