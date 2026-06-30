add_library(coroutines INTERFACE)

target_sources(coroutines
        INTERFACE FILE_SET HEADERS FILES
        "generators.h"
        "event_subscribe.h"
)
