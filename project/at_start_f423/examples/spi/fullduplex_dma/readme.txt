/**
  **************************************************************************
  * @file     readme.txt
  * @brief    readme
  **************************************************************************
  */

  this demo is based on the at-start board, in this demo, shows how to use
  dma transfer data. spi3 use jtag pin as spi pin. after download the code,
  please disconnect the pa13 and pa14 from downloader to make sure communication is ok.
  the pins connection as follow:
  - spi2                     spi3 
    pb12(cs)        <--->     pa15(cs)
    pb13(sck)       <--->     pb3 (sck)
    pb14(miso)      <--->     pa13(miso)
    pb15(mosi)      <--->     pa14(mosi)

  for more detailed information. please refer to the application note document AN0102.
