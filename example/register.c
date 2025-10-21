#include <stdio.h>
#include "zs_tools/zs_tool.h"

void zst_event_cb(zst_event_t *e);

// 全局注册数据
reg_data_t reg_data = {
    .data_pack_type = DATA_PACK_TYPE_DOUBLE_ADDR,
};

reg_data_element_t data_element[3] = {
    [0] = {
        .user_data = (void *)"q0",
    },
    [1] = {
        .user_data = (void *)"q1",
    },
    [2] = {
        .user_data = (void *)"q2",
    },
};
reg_data_pack_t data_pack = {
    .comparison_buffer = NULL,
};

reg_data_element_t data_element1[3] = {
    [0] = {
        .user_data = (void *)"1q0",
    },
    [1] = {
        .user_data = (void *)"1q1",
    },
    [2] = {
        .user_data = (void *)"q2",
    },
};
reg_data_pack_t data_pack1 = {0};

zst_event_dsc_t event_dsc_pack;
zst_event_dsc_t event_dsc_element;

int main()
{
    reg_data_init(&reg_data, 40);

    reg_data_pack_init_t data_pack_init = {
        .reg_data_pack = &data_pack,
        .element_array = data_element,
        .element_array_size = 3,
        .addr = 10,
    };
    reg_data_pack_init(&reg_data, &data_pack_init);

    reg_data_pack_init_t data_pack_init1 = {
        .reg_data_pack = &data_pack1,
        .element_array = data_element1,
        .element_array_size = 3,
        .addr = 30,
    };
    reg_data_pack_init(&reg_data, &data_pack_init1);

    zst_target_add_event_cb_static(&data_pack1, &event_dsc_pack, zst_event_cb, DATA_PACK_ENENT_RECEIVE_FINSH, (void *) 1);
    zst_target_add_event_cb_static(&data_element1[2], &event_dsc_element, zst_event_cb, DATA_PACK_ENENT_RECEIVE_FINSH, (void *) 2);
    data_element1[2].receive_finsh_flag = 1;

    // reg_data_element_t * element = reg_data_get_element_4addr_s(&reg_data, 12);
    // printf("element: %s\n", (char *)element->user_data);

    reg_data_core_run(&reg_data);

    return 0;
}


void zst_event_cb(zst_event_t *e)
{
    void * user_data = zst_event_get_user_data(e);
    printf("user_data: %d\n", (uintptr_t)user_data);
}
