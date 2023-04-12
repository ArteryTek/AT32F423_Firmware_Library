/**
  **************************************************************************
  * @file     readme.txt
  * @brief    readme
  **************************************************************************
  */

  this demo is based on the at-start board, in this demo, shows how to constitute
  a fullduplex i2s module by two halfduplex i2s module.
  the pins distribution as follow:
  - fullduplex i2s
  - pa4(ws)
  - pa5(ck)
  - pa6(mck)
  - pa7(sd)
  - pb14(ext_sd)
  
  if you have only one at-start board for test, you can connect the pins as follow:
  - pa4(ws)  <--->  NC
  - pa5(ck)  <--->  NC
  - pa6(mck) <--->  NC
  - pa7(sd)  <--->  pb14(ext_sd)

  for more detailed information. please refer to the application note document AN0102.
