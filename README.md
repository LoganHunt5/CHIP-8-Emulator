# CHIP-8/SUPERCHIP Emulator
A CHIP-8 Emulator with limited support for SUPERCHIP games written in C++

![image](https://github.com/user-attachments/assets/150d4e6f-b289-43b6-a555-2a6ff52c6d8e)

## _RPS_

![image](https://github.com/user-attachments/assets/4f51afa5-0788-44c0-bbf1-7085a5a44987)

## _Down_

(games shown are from [The octojam game repository](https://johnearnest.github.io/chip8Archive/))

## Before running
  Install cmake and SDL2 if not installed 
  ```
  $ sudo apt-get install cmake libsdl2-dev
```

## Instructions:
  To run a game, move it into the same directory as the emulator files. Then change the GAME variable in the make file.   
  Then run ```$ make run```  

  In order to run a SUPERCHIP game, change the chipType variable in chip8.cpp to SUPERCHIPMODERN (located right above main function)
