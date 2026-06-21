add_library(queue INTERFACE)

target_sources(queue
        INTERFACE FILE_SET HEADERS FILES
        "lock_queue.h"
        "lock_free_queue.h"
)
