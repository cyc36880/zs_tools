cmake_minimum_required(VERSION 3.10)


set(CONTAINER_SRC 
    src/container/src/cc_array_chain.c
    src/container/src/cc_array_sort.c
    src/container/src/cc_array_stack.c
    src/container/src/cc_array.c
    src/container/src/cc_binary.c
    src/container/src/cc_common.c
    src/container/src/cc_hash_map.c
    src/container/src/cc_list_map.c
    src/container/src/cc_list_queue.c
    src/container/src/cc_list_stack.c
    src/container/src/cc_list.c
    src/container/src/cc_mem.c
    src/container/src/cc_ring.c
    src/container/src/cc_string_builder.c
)

set(DEVICE_SRC
    src/device/iremote/iremote.c
)

set(EVENT_SRC
    src/event/zst_event.c
)

set(HAL_SRC
    src/hal/zst_hal_tick.c
)

set(LOG_SRC
    src/log/c_log.c
)

set(MISC_SRC
    src/misc/tlsf/tlsf/tlsf.c
    src/misc/tlsf/mm_tlsf.c
    src/misc/pid.c
    src/misc/sha256.c
    src/misc/zst_mem.c
)

set(POLLING_TASKS_SRC
    src/polling_tasks/polling_task.c
)

set(REGISTER_SRC
    src/register/register.c
)

set(RUN_ONE_SRC
    src/run_one/run_one.c
)

set(TIMER_SRC
    src/timer/zst_timer.c
)



add_library(zs_tools 
    ${CONTAINER_SRC} 
    ${DEVICE_SRC}
    ${EVENT_SRC} 
    ${HAL_SRC}
    ${LOG_SRC}
    ${MISC_SRC}
    ${POLLING_TASKS_SRC}
    ${REGISTER_SRC}
    ${RUN_ONE_SRC}
    ${TIMER_SRC}

    src/zst_core.c
)
target_include_directories(zs_tools PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
