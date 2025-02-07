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

/** @addtogroup 423_I2S_fullduplex_dma I2S_fullduplex_dma
  * @{
  */

#define TXBUF_SIZE                       32
#define RXBUF_SIZE                       TXBUF_SIZE

#define I2S_MASTER_BOARD ///<master board choose

uint16_t i2s1_buffer_tx[TXBUF_SIZE];
uint16_t i2s2_buffer_rx[RXBUF_SIZE];
uint16_t i2s1_buffer_rx[TXBUF_SIZE];
uint16_t i2s2_buffer_tx[RXBUF_SIZE];
volatile error_status transfer_status1 = ERROR, transfer_status2 = ERROR;

/**
  * @brief  buffer compare function.
  * @param  none
  * @retval the result of compare
  */
error_status buffer_compare(uint16_t* pbuffer1, uint16_t* pbuffer2, uint16_t buffer_length)
{
  while(buffer_length--)
  {
    if(*pbuffer1 != *pbuffer2)
    {
      return ERROR;
    }

    pbuffer1++;
    pbuffer2++;
  }
  return SUCCESS;
}

/**
  * @brief  transfer data fill.
  * @param  none
  * @retval none
  */
void tx_data_fill(void)
{
  uint32_t data_index = 0;
  for(data_index = 0; data_index < TXBUF_SIZE; data_index++)
  {
    i2s1_buffer_tx[data_index] = data_index;
    i2s2_buffer_tx[data_index] = data_index;
  }
}

/**
  * @brief  i2s dma configuration.
  * @param  none
  * @retval none
  */
static void i2s_dma_config(void)
{
  dma_init_type dma_init_struct;

  crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
  dmamux_enable(DMA1, TRUE);
  
  /* use dma1_channel2 as i2s1 dma channel */
  dma_reset(DMA1_CHANNEL2);
  dma_default_para_init(&dma_init_struct);
  dma_init_struct.buffer_size = TXBUF_SIZE;
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_HALFWORD;
  dma_init_struct.memory_inc_enable = TRUE;
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_HALFWORD;
  dma_init_struct.peripheral_inc_enable = FALSE;
  dma_init_struct.priority = DMA_PRIORITY_HIGH;
  dma_init_struct.loop_mode_enable = FALSE;
  dma_init_struct.peripheral_base_addr = (uint32_t)&(SPI1->dt);
#if defined(I2S_MASTER_BOARD)
  /* when master board,use i2s1 transmit */
  dma_init_struct.memory_base_addr = (uint32_t)i2s1_buffer_tx;
  dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
#else
  /* when slave board,use i2s1 receive */
  dma_init_struct.memory_base_addr = (uint32_t)i2s1_buffer_rx;
  dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
#endif
  dma_init(DMA1_CHANNEL2, &dma_init_struct);
  
#if defined(I2S_MASTER_BOARD)
  /* when master board,use i2s1 transmit */
  dmamux_init(DMA1MUX_CHANNEL2, DMAMUX_DMAREQ_ID_SPI1_TX);
#else
  /* when slave board,use i2s1 receive */
  dmamux_init(DMA1MUX_CHANNEL2, DMAMUX_DMAREQ_ID_SPI1_RX);
#endif
  
  
  /* use dma1_channel3 as i2s2 dma channel */
  dma_reset(DMA1_CHANNEL3);
  dma_default_para_init(&dma_init_struct);
  dma_init_struct.buffer_size = RXBUF_SIZE;
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_HALFWORD;
  dma_init_struct.memory_inc_enable = TRUE;
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_HALFWORD;
  dma_init_struct.peripheral_inc_enable = FALSE;
  dma_init_struct.priority = DMA_PRIORITY_HIGH;
  dma_init_struct.loop_mode_enable = FALSE;
  dma_init_struct.peripheral_base_addr = (uint32_t)&(SPI2->dt);
#if defined(I2S_MASTER_BOARD)
  /* when master board,use i2s2 receive */
  dma_init_struct.memory_base_addr = (uint32_t)i2s2_buffer_rx;
  dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
#else
  /* when slave board,use i2s2 transmit */
  dma_init_struct.memory_base_addr = (uint32_t)i2s2_buffer_tx;
  dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
#endif
  dma_init(DMA1_CHANNEL3, &dma_init_struct);
  
#if defined(I2S_MASTER_BOARD)
  /* when master board,use i2s2 receive */
  dmamux_init(DMA1MUX_CHANNEL3, DMAMUX_DMAREQ_ID_SPI2_RX);
#else
  /* when slave board,use i2s2 transmit */
  dmamux_init(DMA1MUX_CHANNEL3, DMAMUX_DMAREQ_ID_SPI2_TX);
#endif
}

/**
  * @brief  i2s configuration.
  * @param  none
  * @retval none
  */
static void i2s_config(void)
{
  i2s_init_type i2s_init_struct;
  
  /* spi i2s initialization */
  crm_periph_clock_enable(CRM_SPI1_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_SPI2_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_SCFG_PERIPH_CLOCK, TRUE);
  
  /* spi i2s full duplex config */
  scfg_i2s_full_duplex_config(SCFG_FULL_DUPLEX_I2S1_I2S2);
  
  /* i2s1 config */
  i2s_default_para_init(&i2s_init_struct);
  i2s_init_struct.audio_protocol = I2S_AUDIO_PROTOCOL_PHILLIPS;
  i2s_init_struct.data_channel_format = I2S_DATA_16BIT_CHANNEL_32BIT;
  i2s_init_struct.mclk_output_enable = TRUE;
  i2s_init_struct.audio_sampling_freq = I2S_AUDIO_FREQUENCY_48K;
  i2s_init_struct.clock_polarity = I2S_CLOCK_POLARITY_LOW;
#if defined(I2S_MASTER_BOARD)
  /* when master board,use i2s1 transmit */
  i2s_init_struct.operation_mode = I2S_MODE_MASTER_TX;
#else
  /* when slave board,use i2s1 receive */
  i2s_init_struct.operation_mode = I2S_MODE_SLAVE_RX;
#endif
  i2s_init(SPI1, &i2s_init_struct);
  
#if defined(I2S_MASTER_BOARD)
  /* when master board,use dma transmit */
  spi_i2s_dma_transmitter_enable(SPI1, TRUE);
#else
  /* when slave board,use dma receive */
  spi_i2s_dma_receiver_enable(SPI1, TRUE);
#endif
  
  i2s_enable(SPI1, TRUE);
  
  /* i2s2 config */
  i2s_init_struct.audio_protocol = I2S_AUDIO_PROTOCOL_PHILLIPS;
  i2s_init_struct.data_channel_format = I2S_DATA_16BIT_CHANNEL_32BIT;
  i2s_init_struct.mclk_output_enable = TRUE;
  i2s_init_struct.audio_sampling_freq = I2S_AUDIO_FREQUENCY_48K;
  i2s_init_struct.clock_polarity = I2S_CLOCK_POLARITY_LOW;
#if defined(I2S_MASTER_BOARD)
  /* when master board,use i2s2 receive */
  i2s_init_struct.operation_mode = I2S_MODE_SLAVE_RX;
#else
  /* when slave board,use i2s2 transmit */
  i2s_init_struct.operation_mode = I2S_MODE_SLAVE_TX;
#endif
  i2s_init(SPI2, &i2s_init_struct);
  
#if defined(I2S_MASTER_BOARD)
  /* when master board,use dma receive */
  spi_i2s_dma_receiver_enable(SPI2, TRUE);
#else
  /* when master board,use dma transmit */
  spi_i2s_dma_transmitter_enable(SPI2, TRUE);
#endif

  i2s_enable(SPI2, TRUE);
}

/**
  * @brief  gpio configuration.
  * @param  none
  * @retval none
  */
static void gpio_config(void)
{
  gpio_init_type gpio_initstructure;
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);

  gpio_default_para_init(&gpio_initstructure);

  /* i2s1 ws pin */
  gpio_initstructure.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
  gpio_initstructure.gpio_pull = GPIO_PULL_UP;
  gpio_initstructure.gpio_mode = GPIO_MODE_MUX;
  gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_initstructure.gpio_pins = GPIO_PINS_4;
  gpio_init(GPIOA, &gpio_initstructure);
  gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE4, GPIO_MUX_5);

  /* i2s1 ck pin */
  gpio_initstructure.gpio_pull = GPIO_PULL_DOWN;
  gpio_initstructure.gpio_pins = GPIO_PINS_5;
  gpio_init(GPIOA, &gpio_initstructure);
  gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE5, GPIO_MUX_5);
  
#if defined(I2S_MASTER_BOARD)
  /* i2s1 mck pin */
  gpio_initstructure.gpio_pull = GPIO_PULL_DOWN;
  gpio_initstructure.gpio_pins = GPIO_PINS_6;
  gpio_init(GPIOA, &gpio_initstructure);
  gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE6, GPIO_MUX_5);
#endif

  /* i2s1 sd pin */
  gpio_initstructure.gpio_pull = GPIO_PULL_DOWN;
  gpio_initstructure.gpio_pins = GPIO_PINS_7;
  gpio_init(GPIOA, &gpio_initstructure);
  gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE7, GPIO_MUX_5);

  /* i2s2 ext_sd pin */
  gpio_initstructure.gpio_pull = GPIO_PULL_DOWN;
  gpio_initstructure.gpio_pins = GPIO_PINS_14;
  gpio_init(GPIOB, &gpio_initstructure);
  gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE14, GPIO_MUX_6);
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
  at32_led_on(LED4);

  tx_data_fill();
  
  /* i2s gpio config */
  gpio_config();
  
  /* i2s dma config */
  i2s_dma_config();
  
  /* i2s cofig */
  i2s_config();
  
#if defined(I2S_MASTER_BOARD)
  /* wait for key USER_BUTTON press before starting the communication */
  while(at32_button_press() != USER_BUTTON)
  {
  }
#endif
  
  dma_channel_enable(DMA1_CHANNEL3, TRUE);
  dma_channel_enable(DMA1_CHANNEL2, TRUE);
  
  /* wait data transmit/receive end */
  while(dma_flag_get(DMA1_FDT2_FLAG) == RESET)
  {
  }
  while(dma_flag_get(DMA1_FDT3_FLAG) == RESET)
  {
  }
  
  /* wait i2s1 and i2s2 idle when communication end */
  while(spi_i2s_flag_get(SPI1, SPI_I2S_BF_FLAG) != RESET);
  while(spi_i2s_flag_get(SPI2, SPI_I2S_BF_FLAG) != RESET);

  /* test result:the data check */
  
#if defined(I2S_MASTER_BOARD)
  transfer_status1 = buffer_compare(i2s2_buffer_rx, i2s1_buffer_tx, TXBUF_SIZE);
#else
  transfer_status1 = buffer_compare(i2s1_buffer_rx, i2s2_buffer_tx, TXBUF_SIZE);
#endif

  /* test result indicate:if success ,led2 lights */
  if(transfer_status1 == SUCCESS)
  {
    at32_led_on(LED2);
  }
  else
  {
    at32_led_on(LED3);
  }
  while(1)
  {
  }
}

/**
  * @}
  */

/**
  * @}
  */
