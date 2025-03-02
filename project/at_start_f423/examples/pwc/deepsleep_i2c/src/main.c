/**
  **************************************************************************
  * @file     main.c
  * @brief    main program
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

#include "at32f423_board.h"
#include "at32f423_clock.h"
#include "i2c_application.h"


/** @addtogroup AT32F423_periph_examples
  * @{
  */

/** @addtogroup 423_PWC_deepsleep_i2c PWC_deepsleep_i2c
  * @{
  */

#define I2C_TIMEOUT                      0xFFFFFFF

/* setting i2c clock frequency */
//#define I2Cx_CLKCTRL                   0x7170F7F7   //10K
//#define I2Cx_CLKCTRL                   0xA0F06767   //50K
#define I2Cx_CLKCTRL                     0xA0F03131   //100K
//#define I2Cx_CLKCTRL                   0x30D03259   //200K

#define I2Cx_ADDRESS                     0xA0

#define I2Cx_PORT                        I2C1
#define I2Cx_CLK                         CRM_I2C1_PERIPH_CLOCK

#define I2Cx_SCL_GPIO_CLK                CRM_GPIOB_PERIPH_CLOCK
#define I2Cx_SCL_GPIO_PIN                GPIO_PINS_6
#define I2Cx_SCL_GPIO_PinsSource         GPIO_PINS_SOURCE6
#define I2Cx_SCL_GPIO_PORT               GPIOB
#define I2Cx_SCL_GPIO_MUX                GPIO_MUX_4

#define I2Cx_SDA_GPIO_CLK                CRM_GPIOB_PERIPH_CLOCK
#define I2Cx_SDA_GPIO_PIN                GPIO_PINS_7
#define I2Cx_SDA_GPIO_PinsSource         GPIO_PINS_SOURCE7
#define I2Cx_SDA_GPIO_PORT               GPIOB
#define I2Cx_SDA_GPIO_MUX                GPIO_MUX_4

#define I2Cx_EVT_IRQn                    I2C1_EVT_IRQn
#define I2Cx_ERR_IRQn                    I2C1_ERR_IRQn

#define BUF_SIZE                         8
//#define MASTER_BOARD

uint8_t tx_buf[BUF_SIZE] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
uint8_t rx_buf[BUF_SIZE] = {0};

i2c_handle_type hi2cx;

/**
  * @brief  error handler program
  * @param  i2c_status
  * @retval none
  */
void error_handler(uint32_t error_code)
{
  while(1)
  {
    at32_led_toggle(LED2);
    delay_ms(500);
  }
}

/**
  * @brief  compare whether the valus of buffer 1 and buffer 2 are equal.
  * @param  buffer1: buffer 1 address.
            buffer2: buffer 2 address.
  * @retval 0: equal.
  *         1: unequal.
  */
uint32_t buffer_compare(uint8_t* buffer1, uint8_t* buffer2, uint32_t len)
{
  uint32_t i;

  for(i = 0; i < len; i++)
  {
    if(buffer1[i] != buffer2[i])
    {
      return 1;
    }
  }

  return 0;
}

/**
  * @brief  systemclock recover.
  * @param  none
  * @retval none
  */
void system_clock_recover(void)
{
  /* enable external high-speed crystal oscillator - hext */
  crm_clock_source_enable(CRM_CLOCK_SOURCE_HEXT, TRUE);
  
  /* wait till hext is ready */
  while(crm_hext_stable_wait() == ERROR);
  
  /* enable pll */
  crm_clock_source_enable(CRM_CLOCK_SOURCE_PLL, TRUE);
  
  /* wait till pll is ready */
  while(crm_flag_get(CRM_PLL_STABLE_FLAG) == RESET);
  
  /* enable auto step mode */
  crm_auto_step_mode_enable(TRUE);
  
  /* select pll as system clock source */
  crm_sysclk_switch(CRM_SCLK_PLL);
  
  /* wait till pll is used as system clock source */
  while(crm_sysclk_switch_status_get() != CRM_SCLK_PLL);
}

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  crm_clocks_freq_type crm_clocks_freq_struct = {0};
  i2c_status_type i2c_status;

  /* initial system clock */
  system_clock_config();
  
  /* get system clock */
  crm_clocks_freq_get(&crm_clocks_freq_struct);

  /* at board initial */
  at32_board_init();

  hi2cx.i2cx = I2Cx_PORT;

  /* i2c config */
  i2c_config(&hi2cx);

  while(1)
  {

  #if defined (MASTER_BOARD)

    /* wait for key USER_BUTTON press before starting the communication */
    while(at32_button_press() != USER_BUTTON)
    {
    }

    crm_i2c_clock_select(CRM_I2C1, CRM_I2C_CLOCK_SOURCE_PCLK);
    
    /* start the request reception process */
    if((i2c_status = i2c_master_transmit_int(&hi2cx, I2Cx_ADDRESS, tx_buf, BUF_SIZE, I2C_TIMEOUT)) != I2C_OK)
    {
      error_handler(i2c_status);
    }

    /* wait for the communication to end */
    if(i2c_wait_end(&hi2cx, I2C_TIMEOUT) != I2C_OK)
    {
      error_handler(i2c_status);
    }

    delay_ms(10);

    /* start the request reception process */
    if((i2c_status = i2c_master_receive_int(&hi2cx, I2Cx_ADDRESS, rx_buf, BUF_SIZE, I2C_TIMEOUT)) != I2C_OK)
    {
      error_handler(i2c_status);
    }

    /* wait for the communication to end */
    if(i2c_wait_end(&hi2cx, I2C_TIMEOUT) != I2C_OK)
    {
      error_handler(i2c_status);
    }

    if(buffer_compare(tx_buf, rx_buf, BUF_SIZE) == 0)
    {
      at32_led_on(LED3);
    }
    else
    {
      error_handler(i2c_status);
    }

  #else

    /* wait for key USER_BUTTON press before starting the communication */
    while(at32_button_press() != USER_BUTTON)
    {
    }
    
    crm_hick_sclk_frequency_select(CRM_HICK_SCLK_48MHZ);
    crm_i2c_clock_select(CRM_I2C1, CRM_I2C_CLOCK_SOURCE_HICK);
    
    /* start the transmission process */
    if((i2c_status = i2c_slave_receive_int(&hi2cx, rx_buf, BUF_SIZE, I2C_TIMEOUT)) != I2C_OK)
    {
      error_handler(i2c_status);
    }

    /* select system clock source as hick before ldo set */
    crm_sysclk_switch(CRM_SCLK_HICK);

    /* wait till hick is used as system clock source */
    while(crm_sysclk_switch_status_get() != CRM_SCLK_HICK)
    {
    }
    
    /* reduce ldo before enter deepsleep mode */
    pwc_ldo_output_voltage_set(PWC_LDO_OUTPUT_1V1);
	
	/* congfig the voltage regulator mode.only used with deep sleep mode */
    pwc_voltage_regulate_set(PWC_REGULATOR_EXTRA_LOW_POWER);

    /* enter deep sleep mode */
    pwc_deep_sleep_mode_enter(PWC_DEEP_SLEEP_ENTER_WFI);
    
    /* determine if the debugging function is enabled */
    if((DEBUGMCU->ctrl & 0x00000007) != 0x00000000)
    {
      /* wait 3 LICK(maximum 120us) cycles to ensure clock stable */
      /* when wakeup from deepsleep,system clock source changes to HICK */
      if((CRM->misc1_bit.hick_to_sclk == TRUE) && (CRM->misc1_bit.hickdiv == TRUE))
      {
        /* HICK is 48MHz */
        delay_us(((120 * 6 * HICK_VALUE) /crm_clocks_freq_struct.sclk_freq) + 1);
      }
      else
      {
        /* HICK is 8MHz */
        delay_us(((120 * HICK_VALUE) /crm_clocks_freq_struct.sclk_freq) + 1);
      }
    }

    /* resume ldo before system clock source enhance */
    pwc_ldo_output_voltage_set(PWC_LDO_OUTPUT_1V3);

    /* wake up from deep sleep mode, congfig the system clock */
    system_clock_recover();
    
    /* wait for the communication to end */
    if(i2c_wait_end(&hi2cx, I2C_TIMEOUT) != I2C_OK)
    {
      error_handler(i2c_status);
    }

    if((i2c_status = i2c_slave_transmit_int(&hi2cx, tx_buf, BUF_SIZE, I2C_TIMEOUT)) != I2C_OK)
    {
      error_handler(i2c_status);
    }

    /* select system clock source as hick before ldo set */
    crm_sysclk_switch(CRM_SCLK_HICK);

    /* wait till hick is used as system clock source */
    while(crm_sysclk_switch_status_get() != CRM_SCLK_HICK)
    {
    }
    
    /* reduce ldo before enter deepsleep mode */
    pwc_ldo_output_voltage_set(PWC_LDO_OUTPUT_1V1);
	
	/* congfig the voltage regulator mode.only used with deep sleep mode */
    pwc_voltage_regulate_set(PWC_REGULATOR_EXTRA_LOW_POWER);

    /* enter deep sleep mode */
    pwc_deep_sleep_mode_enter(PWC_DEEP_SLEEP_ENTER_WFI);
    
    /* determine if the debugging function is enabled */
    if((DEBUGMCU->ctrl & 0x00000007) != 0x00000000)
    {
      /* wait 3 LICK(maximum 120us) cycles to ensure clock stable */
      /* when wakeup from deepsleep,system clock source changes to HICK */
      if((CRM->misc1_bit.hick_to_sclk == TRUE) && (CRM->misc1_bit.hickdiv == TRUE))
      {
        /* HICK is 48MHz */
        delay_us(((120 * 6 * HICK_VALUE) /crm_clocks_freq_struct.sclk_freq) + 1);
      }
      else
      {
        /* HICK is 8MHz */
        delay_us(((120 * HICK_VALUE) /crm_clocks_freq_struct.sclk_freq) + 1);
      }
    }

    /* resume ldo before system clock source enhance */
    pwc_ldo_output_voltage_set(PWC_LDO_OUTPUT_1V3);

    /* wake up from deep sleep mode, congfig the system clock */
    system_clock_recover();

    /* wait for the communication to end */
    if(i2c_wait_end(&hi2cx, I2C_TIMEOUT) != I2C_OK)
    {
      error_handler(i2c_status);
    }

    if(buffer_compare(tx_buf, rx_buf, BUF_SIZE) == 0)
    {
      at32_led_on(LED3);
    }
    else
    {
      error_handler(i2c_status);
    }
  #endif
  }
}

/**
  * @brief  initializes peripherals used by the i2c.
  * @param  none
  * @retval none
  */
void i2c_lowlevel_init(i2c_handle_type* hi2c)
{
  gpio_init_type gpio_init_structure;
  exint_init_type exint_init_struct;
  
  if(hi2c->i2cx == I2Cx_PORT)
  {
    exint_init_struct.line_select        = EXINT_LINE_23;
    exint_init_struct.line_enable        = TRUE;
    exint_init_struct.line_polarity      = EXINT_TRIGGER_RISING_EDGE;
    exint_init_struct.line_mode          = EXINT_LINE_INTERRUPT;
    exint_init(&exint_init_struct);
    
    /* pwc periph clock enable */
    crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);
    
    /* i2c periph clock enable */
    crm_periph_clock_enable(I2Cx_CLK, TRUE);
    crm_periph_clock_enable(I2Cx_SCL_GPIO_CLK, TRUE);
    crm_periph_clock_enable(I2Cx_SDA_GPIO_CLK, TRUE);

    /* gpio configuration */
    gpio_pin_mux_config(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_GPIO_PinsSource, I2Cx_SCL_GPIO_MUX);

    gpio_pin_mux_config(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_GPIO_PinsSource, I2Cx_SDA_GPIO_MUX);

    /* configure i2c pins: scl */
    gpio_init_structure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
    gpio_init_structure.gpio_mode           = GPIO_MODE_MUX;
    gpio_init_structure.gpio_out_type       = GPIO_OUTPUT_OPEN_DRAIN;
    gpio_init_structure.gpio_pull           = GPIO_PULL_NONE;

    gpio_init_structure.gpio_pins           = I2Cx_SCL_GPIO_PIN;
    gpio_init(I2Cx_SCL_GPIO_PORT, &gpio_init_structure);

    /* configure i2c pins: sda */
    gpio_init_structure.gpio_pins           = I2Cx_SDA_GPIO_PIN;
    gpio_init(I2Cx_SDA_GPIO_PORT, &gpio_init_structure);

    /* configure and enable i2c interrupt */
    nvic_irq_enable(I2Cx_EVT_IRQn, 0, 0);
    nvic_irq_enable(I2Cx_ERR_IRQn, 0, 0);

    /* config i2c, the digital filter must be 0 in deepsleep mode */
    i2c_init(hi2c->i2cx, 0x00, I2Cx_CLKCTRL);

    i2c_own_address1_set(hi2c->i2cx, I2C_ADDRESS_MODE_7BIT, I2Cx_ADDRESS);
    
    i2c_wakeup_enable(hi2c->i2cx, TRUE);
  }
}


/**
  * @}
  */

/**
  * @}
  */
