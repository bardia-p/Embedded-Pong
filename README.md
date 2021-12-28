# Embedded Pong!

<p align="center">
<img src="images/Pong_Demo.gif" />
</p>

## Description
- This repository simulates a simple pong game that can be controlled via the MSP432R board.
- The game can be run in both single player and multiplayer. 
- While playing the game, the users have the option to use the MSP432R board as a controller to move the right paddle. 
- Alternatively, arrow keys (UP and DOWN) can be used to control the right paddle.
- The left paddle can be moved using the WASD keys (W for up and S for down).

## Compiling the Program
1. You need to have a MSP432R board.
    - You can buy the board from [here](https://www.python.org/downloads/release/python-390/) 
2. To properly run 'main.py' and 'pong.py' you need to first have both python and pygame installed.
    - The latest version of python and pygame can be downloaded from here: \
    [Download Python](https://www.python.org/downloads/release/python-390/)
    - To install the latest version of pygame use the following command:
        ```
        pip install pygame
        ```
3. To install the 'controller.c' file on the board you need to install the KEIL IDE.
    - Download KEIL from [here](https://www2.keil.com/mdk5/install/)
4. Then you need to install the TEXaS drivers:
    - Download the [TEXaS Drivers](http://edx-org-utaustinx.s3.amazonaws.com/UT601x/RTBN_Install.exe)
5. After that you need to install the proper drivers for the board
    - You can download them from [here](https://login.ti.com/as/authorization.oauth2?response_type=code&scope=openid%20email%20profile&client_id=DCIT_ALL_WWW-PROD&state=vfKLAzffqiweH4pSEzlk3KTlMuU&redirect_uri=https%3A%2F%2Fwww.ti.com%2Foidc%2Fredirect_uri%2F&nonce=HfziSJPGmdrN4MpA7cdsdQdnVM3SoGUQ6KPIDykfa6Y&response_mode=form_post).
  
Once you have all the proper drivers installed simply load KEIL and create a new project:
1. Click on ```project -> new uVision Project..```
2. Select the folder for the project.
3. A new dialog should pop open called the ```Device tab```.
    - Navigate through the Texas Instruments devices until you find ```MSP432P401R```.
4. Another new dialog should pop up called ```Manage Run-Time Environment```:
    - Select ```CMSIS-CORE``` for ```Cortex-M, SC000, SC300, ARMv8-M```
    - Select ```System Startup``` for ```MSP432P401R```
5. Right click on ```Target 1``` and select ```Options for Target 'Target 1'```:
    - Under the ```Debug``` tab select the ```CMSIS DAP Debugger```.
    - Under the ```C/C++``` tab select ```C99 Mode```.
    - Under the ```Target``` tab select ```Use default compiler version 5```.
6. Under the ```project``` tab select ```compile project```.
7. Under the ```debug``` tab select ```start/stop debug session``` to load the program to the board.
   
## Running the Program
1. First load the controller.c file into the MSP432R board.
2. Once that program is up and running on the board, start main.py.
3. From there select the number of players (either 1 or 2).
4. After that you can use the buttons on the board (P1.1 for UP and P1.4 for DOWN) or the arrow keys (UP and DOWN) to control the right paddle.
5. Similarly if you are running the game with 2 players, you can used the WASD keys (W for UP and S for DOWN) to control the left paddle.

**Note:** The RGB LED on the controller changes its state every 0.5s. This feature was added to verify that the controller is up and running.

**Note:** Whenever a player scores a goal, the red LED turns on and stays on for 1.5s (aka three state changes on the RGB LED). This is an indicator for the player to show that one side has scored a goal.

## Design Process
This program consists of three parts:
1. pong.py
   - This file includes the logic of the game. It sets up the main environment for the game along with the required graphics using Pygame.
  
2. controller.c
   - This file is in charge of setting up the hardware such as configuring the switches, the LEDs, the timer, and the serial port. 
   - The controller also handles sending the UP and DOWN commands and receiving the G command from the serial port.
  
3. main.py
   - This file is in charge of connecting the controller to the game. It is made up of 2 threads: the reader thread and the main thread.
   - The reader thread is constantly monitoring the serial port for any new commands from the controller.
   - The main thread handles running the game and applying the commands.
   - The main thread also handle sending the "G" command to the controller whenever one of the players scores a goal. This command is later used to turn on the red LED. 


## CREDITS
Author: Bardia Parmoun,

Copyright © 2021. All rights reserved
