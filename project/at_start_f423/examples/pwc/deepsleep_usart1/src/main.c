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

/** @addtogroup AT32F423_periph_examples
  * @{
  */

/** @addtogroup 423_PWC_deepsleep_usart1 PWC_deepsleep_usart1
  * @{
  */

extern uint16_t usart1_index;

/**
  * @brief  usart1 configuration.
  * @param  none
  * @retval none
  */
void usart1_config(uint32_t baudrate)
{
  gpio_init_type gpio_init_struct;

  /* allow access to ertc */
  pwc_battery_powered_domain_access(TRUE);

  /* reset ertc domain */
  crm_battery_powered_domain_reset(TRUE);
  crm_battery_powered_domain_reset(FALSE);

  /* enable the lext osc */
  crm_clock_source_enable(CRM_CLOCK_SOURCE_LEXT, TRUE);

  /* wait till lext is ready */
  while(crm_flag_get(CRM_LEXT_STABLE_FLAG) == RESET);

  crm_usart_clock_select(CRM_USART1, CRM_USART_CLOCK_SOURCE_LEXT);

  /* enable the uart1 and gpio clock */
  crm_periph_clock_enable(CRM_USART1_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_SCFG_PERIPH_CLOCK, TRUE);

  gpio_default_para_init(&gpio_init_struct);

  /* configure the uart1 tx pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode           = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pull           = GPIO_PULL_NONE;
  gpio_init_struct.gpio_pins           = GPIO_PINS_9;
  gpio_init(GPIOA, &gpio_init_struct);

  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode           = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pull           = GPIO_PULL_NONE;
  gpio_init_struct.gpio_pins           = GPIO_PINS_10;
  gpio_init(GPIOA, &gpio_init_struct);

  gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE9, GPIO_MUX_7);
  gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE10, GPIO_MUX_7);

  /* configure uart param */
  nvic_irq_enable(USART1_IRQn, 0, 0);

  usart_init(USART1, baudrate, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_parity_selection_config(USART1, USART_PARITY_NONE);
  usart_transmitter_enable(USART1, TRUE);
  usart_receiver_enable(USART1, TRUE);
  usart_hardware_flow_control_set(USART1,USART_HARDWARE_FLOW_NONE);
  usart_interrupt_enable(USART1, USART_RDBF_INT, TRUE);
  usart_enable(USART1, TRUE);

  /* polling usart initialisation */
  while((!(usart_flag_get(USART1, USART_TXON_FLAG))) || (!(usart_flag_get(USART1, USART_RXON_FLAG))))
  { 
  }
}

/**
  * @brief  usart1 wakeup configuration.
  * @param  none
  * @retval none
  */
void usart1_wakeup_config(void)
{
  exint_init_type exint_init_struct;

  /* keep usart work when deepsleep */
  usart_deep_sleep_mode_enable(USART1, TRUE);

  /* low power wakeup method is receive data buffer full */
  usart_low_power_wakeup_set(USART1, USART_WAKEUP_METHOD_RDBF);

  usart_interrupt_enable(USART1, USART_LPWUF_INT, TRUE);

  /* config the exint line of the usart1 */
  exint_init_struct.line_select   = EXINT_LINE_25;
  exint_init_struct.line_enable   = TRUE;
  exint_init_struct.line_mode     = EXINT_LINE_INTERRUPUT;
  exint_init_struct.line_polarity = EXINT_TRIGGER_RISING_EDGE;
  exint_init(&exint_init_struct);
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
  __IO uint32_t index = 0;
  __IO uint32_t systick_index = 0;

  /* enable pwc clock */
  crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);

  /* congfig the voltage regulator mode.only used with deep sleep mode */
  pwc_voltage_regulate_set(PWC_REGULATOR_EXTRA_LOW_POWER);

  /* congfig the system clock */
  system_clock_config();

  /* init at start board */
  at32_board_init();

  /* turn on the led light */
  at32_led_on(LED2);
  at32_led_on(LED3);
  at32_led_on(LED4);

  /* add a necessary delay to ensure that Vdd is higher than the operating
     voltage of battery powered domain (2.57V) when the battery powered 
     domain is powered on for the first time and being operated. */
  delay_ms(60);
  
  /* config usart or other operations of battery powered domain */
  usart1_config(2400);
  usart1_wakeup_config();

  printf("exit deepsleep mode by usart1 rdbf interrupt \r\n");
  while(1)
  {
    at32_led_off(LED2);
    printf("now enter deepsleep mode \r\n");

    /* make sure that no usart receiver is ongoing */ 
    while(usart_flag_get(USART1, USART_OCCUPY_FLAG) == SET)
    {
    }

    /* select system clock source as hick before ldo set */
    crm_sysclk_switch(CRM_SCLK_HICK);

    /* wait till hick is used as system clock source */
    while(crm_sysclk_switch_status_get() != CRM_SCLK_HICK)
    {
    }

    /* reduce ldo before enter deepsleep mode */
    pwc_ldo_output_voltage_set(PWC_LDO_OUTPUT_1V0);

    while(usart_flag_get(USART1, USART_TDC_FLAG) == RESET)
    {
    }

    /* enter deep sleep mode */
    pwc_deep_sleep_mode_enter(PWC_DEEP_SLEEP_ENTER_WFI);

    /* turn on the led light */
    at32_led_on(LED2);
    
    /* wait clock stable */
    delay_us(120);

    /* resume ldo before system clock source enhance */
    pwc_ldo_output_voltage_set(PWC_LDO_OUTPUT_1V3);

    /* wake up from deep sleep mode, congfig the system clock */
    system_clock_recover();

    /* wake up from sleep mode */
    printf("\r\nnow exit deepsleep mode by usart1 rdbf interrupt \r\n");
    printf("usart1_rdne_data = 0x%x\r\n", usart1_index);
    delay_ms(300);
  }
}

/**
  * @}
  */

/**
  * @}
  */
