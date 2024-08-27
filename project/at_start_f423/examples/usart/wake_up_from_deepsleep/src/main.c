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

/** @addtogroup 423_USART_wake_up_from_deepsleep USART_wake_up_from_deepsleep
  * @{
  */

#define USART1_BUFFER_SIZE            (128)
#define USART1_ID_MATCH               (0x00)

uint8_t usart1_rx_buffer[USART1_BUFFER_SIZE];
uint8_t usart1_rx_counter = 0x00;

uint8_t text_info[] = "enter deep sleep, '0x80' character for waking up mcu..\r\n";

/**
  * @brief  config exint
  * @param  none
  * @retval none
  */
void exint_configuration(void)
{
  exint_init_type exint_init_struct;

  exint_default_para_init(&exint_init_struct);

  exint_init_struct.line_select = EXINT_LINE_25;
  exint_init_struct.line_mode = EXINT_LINE_INTERRUPT;
  exint_init_struct.line_polarity = EXINT_TRIGGER_RISING_EDGE;
  exint_init_struct.line_enable = TRUE;
  exint_init(&exint_init_struct);
}

/**
  * @brief  config usart1
  * @param  none
  * @retval none
  */
void usart_configuration(void)
{
  gpio_init_type gpio_init_struct;

  /* pwc periph clock enable */
  crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);
  /* enable the usart1 and gpio clock */
  crm_periph_clock_enable(CRM_USART1_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);

  gpio_default_para_init(&gpio_init_struct);

  /* configure the usart1 tx/rx pin */
  gpio_init_struct.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_init_struct.gpio_out_type  = GPIO_OUTPUT_PUSH_PULL;
  gpio_init_struct.gpio_mode = GPIO_MODE_MUX;
  gpio_init_struct.gpio_pins = GPIO_PINS_9 | GPIO_PINS_10;
  gpio_init_struct.gpio_pull = GPIO_PULL_NONE;
  gpio_init(GPIOA, &gpio_init_struct);

  /* config usart1 iomux */
  gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE9, GPIO_MUX_7);
  gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE10, GPIO_MUX_7);

  crm_usart_clock_select(CRM_USART1, CRM_USART_CLOCK_SOURCE_HICK);
  
  /* configure usart1 param */
  usart_init(USART1, 9600, USART_DATA_8BITS, USART_STOP_1_BIT);
  usart_parity_selection_config(USART1, USART_PARITY_NONE);
  usart_transmitter_enable(USART1, TRUE);
  usart_receiver_enable(USART1, TRUE);
  
  usart_wakeup_id_set(USART1, USART1_ID_MATCH);
  usart_id_bit_num_set(USART1, USART_ID_RELATED_DATA_BIT);
  
  usart_low_power_wakeup_set(USART1, USART_WAKEUP_METHOD_ID);
  
  nvic_irq_enable(USART1_IRQn, 0, 0);
  
  usart_enable(USART1, TRUE);
  
  /* enable wake up from deep sleep */
  usart_deep_sleep_mode_enable(USART1, TRUE);
  
  /* polling usart initialisation */
  while((!(usart_flag_get(USART1, USART_TXON_FLAG))) || (!(usart_flag_get(USART1, USART_RXON_FLAG))))
  { 
  }
}

/**
  * @brief  usart before enter deep sleep.
  * @param  none
  * @retval none
  */
void usart_before_enter_deep_sleep(void)
{
  /* make sure that no usart receiver is ongoing */ 
  while(usart_flag_get(USART1, USART_OCCUPY_FLAG) == SET)
  {
  }
  
  /* clear rdbf\roerr flag */
  usart_data_receive(USART1);
  
  /* make sure that usart is ready to receive */   
  while(usart_flag_get(USART1, USART_RXON_FLAG) == RESET)
  {
  }

  /* clear lpwuf flag and enable the uart wake up from deep sleep mode interrupt */
  usart_flag_clear(USART1, USART_LPWUF_FLAG);
  usart_interrupt_enable(USART1, USART_LPWUF_INT, TRUE);
}

/**
  * @brief  print info function.
  * @param  none
  * @retval none
  */
void print_info(void)
{
  uint32_t index = 0;
  
  /* send characters one per one, until last char to be sent */
  for (index = 0; index < sizeof(text_info); index++)
  {
    while(usart_flag_get(USART1, USART_TDBE_FLAG) == RESET);
    usart_data_transmit(USART1, text_info[index]);
  }

  /* wait for tdc flag to be raised for last char */
  while(usart_flag_get(USART1, USART_TDC_FLAG) == RESET);
}

/**
  * @brief  print echo function.
  * @param  none
  * @retval none
  */
void print_echo(void)
{
  uint32_t index = 0;
  
  /* send characters one per one, until last char to be sent */
  for (index = 0; index < usart1_rx_counter; index++)
  {
    while(usart_flag_get(USART1, USART_TDBE_FLAG) == RESET);
    usart_data_transmit(USART1, usart1_rx_buffer[index]);
  }
  usart1_rx_counter = 0;

  /* wait for tdc flag to be raised for last char */
  while(usart_flag_get(USART1, USART_TDC_FLAG) == RESET);
}

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  system_clock_config();
  at32_board_init();
  exint_configuration();
  usart_configuration();

  while(1)
  {
    print_info();
    
    usart_before_enter_deep_sleep();
    at32_led_off(LED4);
    /* enter deep sleep mode */
    pwc_deep_sleep_mode_enter(PWC_DEEP_SLEEP_ENTER_WFI);
    at32_led_on(LED4);
    system_clock_config();
    
    delay_ms(20);
    usart_interrupt_enable(USART1, USART_RDBF_INT, FALSE);
    print_echo();
  }
}

/**
  * @}
  */

/**
  * @}
  */
