/**
  **************************************************************************
  * @file     readme.txt
  * @brief    readme
  **************************************************************************
  */

  this demo is based on the at-start board, in this demo, shows how to use
  halfduplex mode transfer data by interrupt mode. use single line switch
  realize data send and receive.  
  the pins connection as follow:
  - spi2 slaver              spi3 master
    pb12(cs)        <--->     pa4(cs)
    pb13(sck)       <--->     pc10(sck)
    pb14(miso)      <--->     pc12(mosi)

  for more detailed information. please refer to the application note document AN0102.
