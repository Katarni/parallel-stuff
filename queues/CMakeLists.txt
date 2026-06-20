add_library(queues INTERFACE)

target_sources(queues
        INTERFACE FILE_SET HEADERS FILES
        "lock_queue.h"
)
