#include "./register.h"
#if (ZST_USE_REGISTER == 1)
#include "../log/c_log.h"
#include "string.h"
#define LOG_TAG "register"


/***************************
 * function declaration
 **************************/
static void reg_data_element_check(reg_data_element_t * element);
static void reg_data_element_aftermath(reg_data_element_t * element);
static void reg_data_pack_check(reg_data_pack_t * data_pack);
static void reg_data_pack_aftermath(reg_data_pack_t * data_pack);
static void reg_data_aftermath(reg_data_t * reg_data);

/****************************
 * global function
 ***************************/

/**
 * @description: 初始化 reg_data
 * @param {reg_data_t *} reg_data 实体句柄
 * @param {uint16_t} hash_capacity hash表容量
 * @return {*}
 */
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

/**
 * @description: 初始化 reg_data_pack
 * @param {reg_data_t *} reg_data
 * @param {reg_data_pack_init_t *} reg_data_pack_init
 * @return {*}
 */
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
    reg_data_pack_init->reg_data_pack->comparison_buffer = reg_data_pack_init->comparison_buffer;
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

/**
 * @description: 得到 reg_data_pack 的 type，参考 `DATA_PACK_TYPE_T`
 * @param {reg_data_t *} reg_data
 * @param {DATA_PACK_TYPE_T *} data_pack_type
 * @return {*}
 */
int reg_data_get_type(const reg_data_t * reg_data, DATA_PACK_TYPE_T * data_pack_type)
{
    if (NULL == reg_data || NULL == data_pack_type) return -1;
    *data_pack_type = reg_data->data_pack_type;
    return 0;
}

/**
 * @description: 设置 `element` 的接收完成标志
 *  @note 不要在中断中使用它，由于不具备 volatile 属性与原子操作（结构体），可能会导致标志位设置失败导致丢失
 * @param {reg_data_element_t *} reg_data_element
 * @param {uint8_t} flag
 * @return {*}
 */
int reg_data_element_set_receive_finsh_flag(reg_data_element_t * reg_data_element, uint8_t flag)
{
    if (NULL == reg_data_element) return -1;
    reg_data_element->flag.receive_finsh_flag = flag;
    return 0;
}

/**
 * @description: 获取 `element` 的接收完成标志
 * @param {reg_data_element_t *} reg_data_element
 * @param {uint8_t *} flag 1表示接收完成; 0表示未接收完成
 * @return {*}
 */
int reg_data_element_get_receive_finsh_flag(const reg_data_element_t * reg_data_element, uint8_t * flag)
{
    if (NULL == reg_data_element || NULL == flag) return -1;
    *flag = reg_data_element->flag.receive_finsh_flag;
    return 0;
}

/**
 * @description: `element` 与原始数据的差异比较
 * @param {reg_data_element_t *} reg_data_element
 * @return {*} 0:无差异； 1:有差异; -1:错误
 */
int reg_data_element_diff_camp(reg_data_element_t * reg_data_element)
{
    if (NULL == reg_data_element) return -1;
    uint8_t * camp_buffer = reg_data_element->owner->comparison_buffer;
    if (NULL == camp_buffer || NULL == reg_data_element->data) return -1;
    reg_data_pack_t * reg_data_pack = reg_data_element->owner;
    // 第几个 element
    size_t element_index = ((uintptr_t)reg_data_element - (uintptr_t)reg_data_pack->element_array.data) / sizeof(reg_data_element_t);
    // 偏移
    size_t camp_offset = 0;
    for (size_t index=0; index<element_index; index++)
    {
        camp_offset += ( (reg_data_element_t *)(reg_data_pack->element_array.data) )[index].data_size;
    }
    uint8_t * camp_element_buffer = camp_buffer + camp_offset;
    if (0 == memcmp(camp_element_buffer, reg_data_element->data, reg_data_element->data_size))
    {
        return 0;
    }
    return 1;
}

/**
 * @description: 得到 `element` 的地址
 * @param {reg_data_element_t *} reg_data_element
 * @return {*} addr: >= 0; -1:错误
 */
int reg_data_element_get_addr(reg_data_element_t * reg_data_element)
{
    if (NULL == reg_data_element) return -1;
    reg_data_pack_t * reg_data_pack = reg_data_element->owner;
    // 第几个 element
    size_t element_index = ((uintptr_t)reg_data_element - (uintptr_t)reg_data_pack->element_array.data) / sizeof(reg_data_element_t);
    return element_index;
}

/**
 * @description: 消除 `element` 与 原始数据的差异
 *  @note 这是将 `element` 的数据拷贝到 `comparison_buffer` 中
 * @param {reg_data_element_t *} reg_data_element
 * @return {*}
 */
int reg_data_elemet_diff_eliminate(reg_data_element_t * reg_data_element)
{
    if (NULL == reg_data_element) return -1;
    uint8_t * camp_buffer = reg_data_element->owner->comparison_buffer;
    if (NULL == camp_buffer || NULL == reg_data_element->data) return -1;
    reg_data_pack_t * reg_data_pack = reg_data_element->owner;
    // 第几个 element
    size_t element_index = ((uintptr_t)reg_data_element - (uintptr_t)reg_data_pack->element_array.data) / sizeof(reg_data_element_t);
    // 偏移
    size_t camp_offset = 0;
    for (size_t index=0; index<element_index; index++)
    {
        camp_offset += ( (reg_data_element_t *)(reg_data_pack->element_array.data) )[index].data_size;
    }
    uint8_t * camp_element_buffer = camp_buffer + camp_offset;
    memcpy(camp_element_buffer, reg_data_element->data, reg_data_element->data_size);
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
        reg_data_iter_s_t iter;
        reg_data_get_pack_iter_init_s(&iter, reg_data);
        reg_data_element_t * element = NULL;
        uint16_t addr;
        size_t index = 0;
        size_t total_element = 0;
        while (0 == reg_data_get_pack_iter_next_s(&iter, &element, &addr, &index))
        {
            reg_data_element_check(element);
            if (index - total_element == element->owner->element_array.elem_nums-1)
            {
                total_element += element->owner->element_array.elem_nums;
                reg_data_pack_check(element->owner);
            }
        }
        // aftermath
        reg_data_get_pack_iter_init_s(&iter, reg_data);
        total_element = 0;
        while (0 == reg_data_get_pack_iter_next_s(&iter, &element, &addr, &index))
        {
            reg_data_element_aftermath(element);
            if (index - total_element == element->owner->element_array.elem_nums-1)
            {
                total_element += element->owner->element_array.elem_nums;
                reg_data_pack_aftermath(element->owner);
            }
        }
    }
    // double addr
    else if (DATA_PACK_TYPE_DOUBLE_ADDR == data_pack_type)
    {
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
                reg_data_element_check(element);
            }
            reg_data_pack_check(data_pack);
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
                reg_data_element_aftermath(element);
            }
            reg_data_pack_aftermath(data_pack);
        }
    }
    reg_data_aftermath(reg_data);
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

/****************************
 * static function
 ***************************/
static void reg_data_element_receive_finish_check_core(reg_data_element_t * element);
static void reg_data_element_receive_finish_aftermath_core(reg_data_element_t * element);
static void reg_data_pack_receive_finish_check_core(reg_data_pack_t * data_pack);
static void reg_data_pack_receive_finish_aftermath_core(reg_data_pack_t * data_pack);
static void reg_data_receive_finish_aftermath_core(reg_data_t * reg_data);

static void subscribe_element_check_core(reg_data_element_t * element);
static void subscribe_element_aftermath_core(reg_data_element_t * element);
static void subscribe_pack_check_core(reg_data_pack_t * data_pack);
static void subscribe_pack_aftermath_core(reg_data_pack_t * data_pack);
static void subscribe_reg_data_aftermath_core(reg_data_t * reg_data);

// reg data core

static void reg_data_element_check(reg_data_element_t * element)
{
    reg_data_element_receive_finish_check_core(element);
    subscribe_element_check_core(element);
}

static void reg_data_element_aftermath(reg_data_element_t * element)
{
    reg_data_element_receive_finish_aftermath_core(element);
    subscribe_element_aftermath_core(element);
}

static void reg_data_pack_check(reg_data_pack_t * data_pack)
{
    reg_data_pack_receive_finish_check_core(data_pack);
    subscribe_pack_check_core(data_pack);
}

static void reg_data_pack_aftermath(reg_data_pack_t * data_pack)
{
    reg_data_pack_receive_finish_aftermath_core(data_pack);
    subscribe_pack_aftermath_core(data_pack);
}


static void reg_data_aftermath(reg_data_t * reg_data)
{
    reg_data_receive_finish_aftermath_core(reg_data);
    subscribe_reg_data_aftermath_core(reg_data);
}

// ======================== reg data 接收完成事件 =====================================
static void reg_data_element_receive_finish_check_core(reg_data_element_t * element)
{
    if (element->flag.receive_finsh_flag)
    {
        element->flag.receive_finsh_flag = 0;
        element->owner->flag.receive_finsh_flag = 1;
        zst_event_send_exe_now(element, DATA_PACK_ENENT_RECEIVE_FINSH, NULL);

        // 写入到 ower 的 receive 的 事件列表中
        element->ent_rev_next = NULL;
        if (NULL == element->owner->ent_rev_elements)
        {
            element->owner->ent_rev_elements = element;
        }
        else
        {
            element->ent_rev_next = element->owner->ent_rev_elements;
            element->owner->ent_rev_elements = element;
        }
    }
}

static void reg_data_element_receive_finish_aftermath_core(reg_data_element_t * element)
{
}

static void reg_data_pack_receive_finish_check_core(reg_data_pack_t * data_pack)
{
    if (data_pack->flag.receive_finsh_flag)
    {
        data_pack->flag.receive_finsh_flag = 0;
        data_pack->owner->flag.receive_finsh_flag = 1;
        zst_event_send_exe_now(data_pack, DATA_PACK_ENENT_RECEIVE_FINSH, NULL);

        // 写入到 ower 的 receive 的 事件列表中
        data_pack->ent_rev_next = NULL;
        if (NULL == data_pack->owner->ent_rev_packs)
        {
            data_pack->owner->ent_rev_packs = data_pack;
        }
        else
        {
            data_pack->ent_rev_next = data_pack->owner->ent_rev_packs;
            data_pack->owner->ent_rev_packs = data_pack;
        }
    }
}

static void reg_data_pack_receive_finish_aftermath_core(reg_data_pack_t * data_pack)
{
}

static void reg_data_receive_finish_aftermath_core(reg_data_t * reg_data)
{
    if (reg_data->flag.receive_finsh_flag)
    {
        reg_data->flag.receive_finsh_flag = 0;
        zst_event_send_exe_now(reg_data, DATA_PACK_ENENT_RECEIVE_FINSH, NULL);

        // 对 receive 事件列表清空
        reg_data_pack_t * data_pack = reg_data->ent_rev_packs;
        while (NULL != data_pack)
        {
            reg_data_element_t * element = data_pack->ent_rev_elements;
            while (NULL != element)
            {
                reg_data_element_t * temp_element = element->ent_rev_next;
                element->ent_rev_next = NULL;
                element = temp_element;
            }
            reg_data_pack_t * temp_pack = data_pack->ent_rev_next;
            data_pack->ent_rev_elements = NULL;
            data_pack->ent_rev_next = NULL;
            data_pack = temp_pack;
        }
        reg_data->ent_rev_packs = NULL;
    }
}


// ======================== reg data 订阅事件 ==================================
static void subscribe_element_check_core(reg_data_element_t * element)
{
    if (1 == element->subscribe)
    {
        if (1 == reg_data_element_diff_camp(element))
        {
            element->flag.camp_diff_flag = 1;
            zst_event_send_exe_now(element, DATA_PACK_ENENT_ELEMENT_DIFF_CHECK, NULL);
        }
    }
}

static void subscribe_element_aftermath_core(reg_data_element_t * element)
{
    if (1 == element->flag.camp_diff_flag)
    {
        element->flag.camp_diff_flag = 0;
        element->owner->flag.camp_diff_flag = 1;
        reg_data_elemet_diff_eliminate(element);
        zst_event_send_exe_now(element, DATA_PACK_ENENT_ELEMENT_DIFF_AFTERMATH, NULL);
    }
}

static void subscribe_pack_check_core(reg_data_pack_t * data_pack)
{
    if (data_pack->flag.camp_diff_flag)
    {
        zst_event_send_exe_now(data_pack, DATA_PACK_ENENT_ELEMENT_DIFF_CHECK, NULL);
    }
}

static void subscribe_pack_aftermath_core(reg_data_pack_t * data_pack)
{
    if (data_pack->flag.camp_diff_flag)
    {
        data_pack->flag.camp_diff_flag = 0;
        data_pack->owner->flag.camp_diff_flag = 1;
        zst_event_send_exe_now(data_pack, DATA_PACK_ENENT_ELEMENT_DIFF_AFTERMATH, NULL);
    }
}

static void subscribe_reg_data_aftermath_core(reg_data_t * reg_data)
{
    if (reg_data->flag.camp_diff_flag)
    {
        reg_data->flag.camp_diff_flag = 0;
        zst_event_send_exe_now(reg_data, DATA_PACK_ENENT_ELEMENT_DIFF_AFTERMATH, NULL);
    }
}

#endif
