/**
 * @file delay_port.c
 * @brief WIT传感器STM32F407延时端口层实现
 * @details 本文件基于STM32 HAL库和DWT实现延时端口层功能，支持毫秒和微秒级精确延时
 * @author Augment Agent
 * @date 2025-07-25
 */

#include "wit_port.h"
#include "stm32f407_port_config.h"

/* ========================================================================== */
/*                              私有宏定义                                    */
/* ========================================================================== */

#define SYSTICK_FREQ_HZ         SYSTICK_FREQ        /* SysTick频率(Hz) */
#define CPU_FREQ_HZ             SYSTEM_CLOCK_FREQ   /* CPU频率(Hz) */
#define DELAY_US_FACTOR         WIT_DELAY_US_FACTOR /* 微秒延时因子 */

/* ========================================================================== */
/*                              私有变量                                      */
/* ========================================================================== */

static uint8_t s_delay_initialized = 0;            /* 延时初始化标志 */

/* ========================================================================== */
/*                              私有函数声明                                  */
/* ========================================================================== */

#if WIT_USE_DWT_DELAY
static void delay_us_dwt(uint32_t us);
#else
static void delay_us_loop(uint32_t us);
#endif

/* ========================================================================== */
/*                              公共函数实现                                  */
/* ========================================================================== */

/**
 * @brief 延时端口层初始化
 * @return 0: 成功, 其他: 失败
 * @note SysTick由HAL库自动初始化，此函数主要初始化DWT(如果使用)
 */
int32_t wit_port_delay_init(void)
{
    if (s_delay_initialized) {
        return 0;  /* 已经初始化 */
    }

#if WIT_USE_DWT_DELAY
    /* 初始化DWT用于微秒延时 */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
#endif

    s_delay_initialized = 1;
    return 0;
}

/**
 * @brief 毫秒级延时
 * @param ucMs 延时时间(毫秒)
 */
void wit_port_delay_ms(uint16_t ucMs)
{
    if (ucMs == 0) {
        return;
    }

    /* 检查初始化状态 */
    if (!s_delay_initialized) {
        wit_port_delay_init();
    }

    /* 使用HAL库的毫秒延时 */
    HAL_Delay(ucMs);
}

/**
 * @brief 微秒级延时
 * @param ucUs 延时时间(微秒)
 */
void wit_port_delay_us(uint16_t ucUs)
{
    if (ucUs == 0) {
        return;
    }

    /* 检查初始化状态 */
    if (!s_delay_initialized) {
        wit_port_delay_init();
    }

#if WIT_USE_DWT_DELAY
    /* 使用DWT计数器实现高精度微秒延时 */
    delay_us_dwt(ucUs);
#else
    /* 使用循环计数实现微秒延时 */
    delay_us_loop(ucUs);
#endif
}

/* ========================================================================== */
/*                              私有函数实现                                  */
/* ========================================================================== */

#if WIT_USE_DWT_DELAY
/**
 * @brief 基于DWT的微秒延时
 * @param us 延时时间(微秒)
 */
static void delay_us_dwt(uint32_t us)
{
    uint32_t start_cycle = DWT->CYCCNT;
    uint32_t delay_cycles = us * (CPU_FREQ_HZ / 1000000UL);

    while ((DWT->CYCCNT - start_cycle) < delay_cycles) {
        /* 等待延时完成 */
    }
}
#else
/**
 * @brief 基于循环的微秒延时
 * @param us 延时时间(微秒)
 */
static void delay_us_loop(uint32_t us)
{
    volatile uint32_t delay_count = us * DELAY_US_FACTOR;

    while (delay_count--) {
        __NOP();  /* 空操作，防止编译器优化 */
    }
}
#endif


