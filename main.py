import pygame
import serial
import time
import sys
import threading
from pygame.locals import *
from pong import *

pygame.init()
fps = pygame.time.Clock()
window = pygame.display.set_mode((WIDTH, HEIGHT), 0, 32)
pygame.display.set_caption('PONG!')


p_count = int(input("How many players? 1 or 2: "))

if p_count == 1:
    pong = Pong(window, True)
else:
    pong = Pong(window, False)

serialPort = serial.Serial(port="COM4", baudrate=115200,
                           bytesize=8, timeout=2)

if not serialPort.isOpen():
    serialPort.open()

serialString = ""  # Used to hold data coming over UART


class Reader(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.isBusy = False
        self._stop_event = threading.Event()

    def run(self):
        print("Starting the reading thread...")
        self.read_serial()
        print("Exiting the reading thread...")

    def read_serial(self):
        while not self.stopped():
            # Wait until there is data waiting in the serial buffer
            if (serialPort.in_waiting > 0):
                self.isBusy = True
                # Read data out of the buffer until a carraige return / new
                # line is found
                serialString = serialPort.readline()

                # Print the contents of the serial data
                command = serialString.decode('Ascii')

                print("Controller command: " + command)

                if "UP" in command:
                    pong.move(-4, Direction.RIGHT)
                elif "DOWN" in command:
                    pong.move(4, Direction.RIGHT)
                elif "DONE" in command:
                    pong.move(0, Direction.RIGHT)

                self.isBusy = False

    def stop(self):
        self._stop_event.set()

    def stopped(self):
        return self._stop_event.is_set()


# Create new threads
reader = Reader()

# Start new Threads
reader.start()

while True:
    pong.play()

    for event in pygame.event.get():
        if event.type == KEYDOWN:
            pong.keydown(event)
        elif event.type == KEYUP:
            pong.keyup(event)
        elif event.type == pygame.QUIT:
            pygame.quit()
            reader.stop()
            sys.exit()

    pygame.display.update()
    fps.tick(60)