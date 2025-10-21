#include "./register.h"
#if (ZST_USE_REGISTER == 1)
#include "../log/c_log.h"

#define LOG_TAG "register"

int reg_data_init(reg_data_t * reg_data, uint16_t hash_capacity)
{
    if (NULL == reg_data)
    {
        ZST_LOGE(LOG_TAG, "reg_data point is NULL");
        return -1;
    }
    if (reg_data->data_pack_type >= DATA_PACK_TYPE_SIGNAL_LAST)
    {
        ZST_LOGE(LOG_TAG, "reg_data not init");
        return -2;
    }
    cc_hash_map_new(&reg_data->data_pack, hash_capacity, NULL, NULL);
    if (NULL == reg_data->data_pack)
    {
        ZST_LOGE(LOG_TAG, "new filed");
        return -3;
    }
    return 0;
}

int reg_data_pack_init(reg_data_t * reg_data, reg_data_pack_init_t * reg_data_pack_init)
{
    if (NULL == reg_data || NULL == reg_data_pack_init || NULL == reg_data_pack_init->element_array || 0 == reg_data_pack_init->element_array_size)
    {
        ZST_LOGE(LOG_TAG, "param is NULL");
        return -1;
    }
    if (NULL == reg_data->data_pack)
    {
        ZST_LOGE(LOG_TAG, "reg_data->data_pack is NULL");
        return -2;
    }
    reg_data_pack_init->reg_data_pack->owner = reg_data;
    for (uint16_t pos = 0; pos < reg_data_pack_init->element_array_size; pos++)
    {
        reg_data_element_t * element = &reg_data_pack_init->element_array[pos];
        element->owner = reg_data_pack_init->reg_data_pack;
    }
    if (0 != cc_array_init(&reg_data_pack_init->reg_data_pack->element_array, (unsigned char *)reg_data_pack_init->element_array, reg_data_pack_init->element_array_size, sizeof(reg_data_element_t)))
    {
        ZST_LOGE(LOG_TAG, "init element_array failed");
        return -1;
    }

    if (DATA_PACK_TYPE_SIGNAL_ADDR == reg_data->data_pack_type)
    {
        for (uint16_t pos = 0; pos < reg_data_pack_init->element_array_size; pos++)
        {
            void * elemrnt_val = (void *)&reg_data_pack_init->element_array[pos];
            if(0 != cc_hash_map_set_new( reg_data->data_pack, (void *)(uintptr_t)(reg_data_pack_init->addr + pos), elemrnt_val))
            {
                ZST_LOGE(LOG_TAG, "add failed");
                return -1;
            }
        }
    }
    else if (DATA_PACK_TYPE_DOUBLE_ADDR == reg_data->data_pack_type)
    {
        if (0 != cc_hash_map_set_new( reg_data->data_pack, (void *)(uintptr_t)(reg_data_pack_init->addr), reg_data_pack_init->reg_data_pack))
        {
            ZST_LOGE(LOG_TAG, "add failed");
            return -1;
        }
    }
    return 0;
}

int reg_data_get_type(const reg_data_t * reg_data, DATA_PACK_TYPE_T * data_pack_type)
{
    if (NULL == reg_data || NULL == data_pack_type) return -1;
    *data_pack_type = reg_data->data_pack_type;
    return 0;
}

int reg_data_element_set_receive_finsh_flag(reg_data_element_t * reg_data_element, uint8_t flag)
{
    if (NULL == reg_data_element) return -1;
    reg_data_element->receive_finsh_flag = flag;
    return 0;
}

int reg_data_element_get_receive_finsh_flag(const reg_data_element_t * reg_data_element, uint8_t * flag)
{
    if (NULL == reg_data_element || NULL == flag) return -1;
    *flag = reg_data_element->receive_finsh_flag;
    return 0;
}


int reg_data_core_run(reg_data_t * reg_data)
{
    if (NULL == reg_data) return -1;
    DATA_PACK_TYPE_T data_pack_type;
    if (0 != reg_data_get_type(reg_data, &data_pack_type)) return -1;

    // signal addr
    if (DATA_PACK_TYPE_SIGNAL_ADDR == data_pack_type)
    {
        uint8_t is_receive_finsh = 0;
        reg_data_iter_s_t iter;
        reg_data_get_pack_iter_init_s(&iter, reg_data);
        reg_data_element_t * element = NULL;
        uint16_t addr;
        size_t index = 0;
        size_t total_element = 0;
        while (0 == reg_data_get_pack_iter_next_s(&iter, &element, &addr, &index))
        {
            if (1 == element->receive_finsh_flag)
            {
                is_receive_finsh=1;
                // 向每一个 element 发送事件
                zst_event_send_exe_now(element, DATA_PACK_ENENT_RECEIVE_FINSH, (void *)(uintptr_t)addr);
            }
            if (index - total_element == element->owner->element_array.elem_nums-1)
            {
                total_element += element->owner->element_array.elem_nums;
                if (is_receive_finsh)
                {
                    is_receive_finsh = 0;
                    // 向 reg_data_pack 发送事件
                    zst_event_send_exe_now(element->owner, DATA_PACK_ENENT_RECEIVE_FINSH, element->owner);
                }
            }
        }
        reg_data_get_pack_iter_init_s(&iter, reg_data);
        while (0 == reg_data_get_pack_iter_next_s(&iter, &element, &addr, &index))
        {
            element->receive_finsh_flag = 0;
        }
    }
    // double addr
    else if (DATA_PACK_TYPE_DOUBLE_ADDR == data_pack_type)
    {
        uint8_t is_receive_finsh = 0;
        reg_data_iter_d_t data_iter;
        reg_data_get_pack_iter_init_d(&data_iter, reg_data);
        reg_data_pack_t * data_pack = NULL;
        uint16_t addr;
        size_t index = 0;
        while (0 == reg_data_get_pack_iter_next_d(&data_iter, &data_pack, &addr, &index))
        {
            reg_data_element_iter_t element_iter;
            reg_data_get_element_iter_init_d(&element_iter, data_pack);
            reg_data_element_t * element = NULL;
            uint16_t addr2;
            size_t index2 = 0;
            while (0 == reg_data_get_element_iter_next_d(&element_iter, &element, &addr2, &index2))
            {
                if (1 == element->receive_finsh_flag)
                {
                    is_receive_finsh=1;
                    // 向每一个 element 发送事件
                    zst_event_send_exe_now(element, DATA_PACK_ENENT_RECEIVE_FINSH, (void *)(uintptr_t)addr2);
                }
            }
            if (is_receive_finsh)
            {
                is_receive_finsh = 0;
                zst_event_send_exe_now(data_pack, DATA_PACK_ENENT_RECEIVE_FINSH, (void *)data_pack);
            }
        }
        reg_data_get_pack_iter_init_d(&data_iter, reg_data);
        while (0 == reg_data_get_pack_iter_next_d(&data_iter, &data_pack, &addr, &index))
        {
            reg_data_element_iter_t element_iter;
            reg_data_get_element_iter_init_d(&element_iter, data_pack);
            reg_data_element_t * element = NULL;
            uint16_t addr2;
            size_t index2 = 0;
            while (0 == reg_data_get_element_iter_next_d(&element_iter, &element, &addr2, &index2))
            {
                element->receive_finsh_flag = 0;
            }
        }
    }
    return 0;
}

// double addr
int reg_data_get_pack_iter_init_d(reg_data_iter_d_t * iter, reg_data_t * reg_data)
{
    return reg_data_get_pack_iter_init_s(iter, reg_data);
}

int reg_data_get_pack_iter_next_d(reg_data_iter_d_t * iter, reg_data_pack_t ** reg_data_pack, uint16_t * addr, size_t * index)
{
    void * item = NULL;
    int ret = cc_hash_map_iter_next(iter, &item, index);
    if (0 != ret) return ret;
    cc_map_item_t * map_item = (cc_map_item_t *)item;
    if (NULL != addr) *addr = (uint16_t)(uintptr_t)map_item->key;
    if (NULL != reg_data_pack) *reg_data_pack = (reg_data_pack_t *)map_item->value;
    return ret;
}

int reg_data_get_element_iter_init_d(reg_data_element_iter_t * iter, reg_data_pack_t * reg_data_pack)
{
    return cc_array_iter_init(iter, &reg_data_pack->element_array);
}

int reg_data_get_element_iter_next_d(reg_data_element_iter_t * iter, reg_data_element_t ** reg_data_element, uint16_t * addr, size_t * index)
{
    void * item = NULL;
    size_t cc_index = 0;
    int ret = cc_array_iter_next(iter, &item, &cc_index);
    if (0 != ret) return ret;
    reg_data_element_t * element = (reg_data_element_t *)item;
    if (NULL != addr) *addr = cc_index;
    if (NULL != reg_data_element) *reg_data_element = element;
    if (NULL != index) *index = cc_index;
    return ret;
}

reg_data_pack_t * reg_data_get_pack_4addr_d(const reg_data_t * reg_data, uint16_t addr)
{
    reg_data_pack_t * data_pack = NULL;
    if (NULL == reg_data || NULL == reg_data->data_pack) return NULL;
    cc_hash_map_get(reg_data->data_pack, (void *)(uintptr_t)addr, (void **)&data_pack);
    return data_pack;
}

reg_data_element_t * reg_data_get_element_4addr_d(reg_data_pack_t * data_pack, uint16_t addr2)
{
    if (NULL == data_pack) return NULL;
    reg_data_element_t * element = NULL;
    cc_array_get_ref(&data_pack->element_array, addr2, (void **)&element);
    return element;
}

// signal addr

int reg_data_get_pack_iter_init_s(reg_data_iter_s_t * iter, reg_data_t * reg_data)
{
    return cc_hash_map_iter_init(iter, reg_data->data_pack);
}

int reg_data_get_pack_iter_next_s(reg_data_iter_s_t * iter, reg_data_element_t ** reg_data_element, uint16_t * addr, size_t * index)
{
    void * item = NULL;
    int ret = cc_hash_map_iter_next(iter, &item, index);
    if (0 != ret) return ret;
    cc_map_item_t * map_item = (cc_map_item_t *)item;
    if (NULL != addr) *addr = (uint16_t)(uintptr_t)map_item->key;
    if (NULL != reg_data_element) *reg_data_element = (reg_data_element_t *)map_item->value;
    return ret;
}

reg_data_element_t * reg_data_get_element_4addr_s(const reg_data_t * reg_data, uint16_t addr)
{
    reg_data_element_t * element = NULL;
    if (NULL == reg_data || NULL == reg_data->data_pack) return NULL;
    cc_hash_map_get(reg_data->data_pack, (void *)(uintptr_t)addr, (void **)&element);
    return element;
}

#endif
