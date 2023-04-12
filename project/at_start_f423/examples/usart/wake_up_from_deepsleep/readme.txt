/**
  **************************************************************************
  * @file     readme.txt
  * @brief    readme
  **************************************************************************
  */

  this demo is based on the at-start board, in this demo, configuration of gpio and 
  usart peripherals to allow the characters(match id) received on usart rx pin to 
  wake up mcu from low power mode(deep sleep). if wake up mcu, led4 will light.

  set-up
  - connect usart1 tx pin (pa9) to usart1 rx pin (pa10)
  - connect PC Com port (ex: using HyperTerminal)

  for more detailed information. please refer to the application note document AN0099.
