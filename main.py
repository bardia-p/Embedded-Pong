import pygame
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

while True:
    pong.play()

    for event in pygame.event.get():
        if event.type == KEYDOWN:
            pong.keydown(event)
        elif event.type == KEYUP:
            pong.keyup(event)
        elif event.type == pygame.QUIT:
            pygame.quit();

    pygame.display.update()
    fps.tick(60)