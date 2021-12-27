import pygame
from pygame.locals import *
import random
from enum import Enum

WIDTH = 600
HEIGHT = 400
BALL_RADIUS = 10
PADDLE_HEIGHT = 80
PADDLE_WIDTH = 8


class Direction(Enum):
    LEFT = 1
    DOWN = 2
    RIGHT = 3
    UP = 4

class Color(Enum):
    WHITE = (255, 255, 255)
    RED = (255, 0, 0)
    GREEN = (0, 255, 0)
    BLACK = (0, 0, 0)

class Ball:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.vx = random.choice([-1,1])
        self.vy = random.choice([-1,1])
        self.r = BALL_RADIUS

    def move(self):
        self.x += self.vx
        self.y += self.vy

    def reflect(self, dir):
        if dir == Direction.LEFT or dir == Direction.RIGHT:
            self.vx *= -1
            self.vx *= 1.1
            self.vy *= 1.1
        elif dir == Direction.UP or dir == Direction.DOWN:
            self.vy *= -1

    def draw(self, canvas):
        pygame.draw.circle(canvas, Color.RED.value, [self.x, self.y], self.r, 0)


class Paddle:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.l = PADDLE_HEIGHT
        self.w = PADDLE_WIDTH
        self.v = 0

    def move(self):
        if self.y > self.l//2 and self.y < HEIGHT - self.l//2:
            self.y += self.v
        elif self.y == self.l//2 and self.v > 0:
            self.y += self.v
        elif self.y == HEIGHT - self.l//2 and self.v < 0:
            self.y += self.v

    def draw(self, canvas):
        pygame.draw.polygon(canvas, Color.GREEN.value, [
            [self.x - self.w //2 , self.y - self.l // 2],
            [self.x - self.w // 2, self.y + self.l // 2],
            [self.x + self.w // 2, self.y + self.l // 2],
            [self.x + self.w // 2, self.y - self.l // 2]])

class Pong:
    def __init__(self, canvas, AIPlayer):
        self.canvas = canvas
        self.b = Ball(WIDTH//2, HEIGHT//2)
        self.l_p = Paddle(PADDLE_WIDTH // 2 - 1, HEIGHT//2)
        self.r_p = Paddle(WIDTH - PADDLE_WIDTH // 2, HEIGHT//2)
        self.l_score = 0
        self.r_score = 0
        self.AIPlayer = AIPlayer

    def draw(self):
        self.canvas.fill(Color.BLACK.value)
        self.b.draw(self.canvas)
        self.l_p.draw(self.canvas)
        self.r_p.draw(self.canvas)

        # updating scores
        font = pygame.font.SysFont("Comic Sans MS", 20)
        label = font.render("Score " + str(self.l_score), 1, (255, 255, 0))
        self.canvas.blit(label, (50, 20))

        font = pygame.font.SysFont("Comic Sans MS", 20)
        label = font.render("Score " + str(self.r_score), 1, (255, 255, 0))
        self.canvas.blit(label, (470, 20))

    def play(self):
        if self.AIPlayer:
            if abs(self.b.y - self.l_p.y) <= 10:
                self.l_p.v = 8 if self.b.y > self.l_p.y else -8

        self.b.move()
        self.r_p.move()
        self.l_p.move()
        self.handleCollision()
        self.draw()

    def handleCollision(self):
        # ball collision on top and bottom walls
        if int(self.b.y) <= BALL_RADIUS:
            self.b.reflect(Direction.UP)
        if int(self.b.y) >= HEIGHT + 1 - BALL_RADIUS:
            self.b.reflect(Direction.DOWN)

        # ball collision on the left paddle
        if int(self.b.x) <= BALL_RADIUS + PADDLE_WIDTH and \
                int(self.b.y) in range(int(self.l_p.y - PADDLE_HEIGHT // 2),
                                      int(self.l_p.y + PADDLE_HEIGHT // 2), 1):
            self.b.reflect(Direction.LEFT)

        # ball collision on the left wall
        elif int(self.b.x) <= BALL_RADIUS + PADDLE_WIDTH:
            self.r_score += 1
            self.b = Ball(WIDTH // 2, HEIGHT // 2)

        # ball collision on the right paddle
        if int(self.b.x) >= WIDTH + 1 - BALL_RADIUS - PADDLE_WIDTH and \
                int(self.b.y) in range(int(self.r_p.y - PADDLE_HEIGHT // 2),
                                      int(self.r_p.y + PADDLE_HEIGHT // 2), 1):
            self.b.reflect(Direction.RIGHT)

        # ball collision on the right wall
        elif int(self.b.x) >= WIDTH + 1 - BALL_RADIUS - PADDLE_WIDTH:
            self.l_score += 1
            self.b = Ball(WIDTH // 2, HEIGHT // 2)

    def keydown(self, event):
        if event.key == K_UP:
            self.move(-8, Direction.RIGHT)
        elif event.key == K_DOWN:
            self.move(8, Direction.RIGHT)
        elif event.key == K_w:
            self.move(-8, Direction.LEFT)
        elif event.key == K_s:
            self.move(8, Direction.LEFT)

    def keyup(self, event):
        if event.key in (K_w, K_s):
            self.move(0, Direction.LEFT)
        elif event.key in (K_UP, K_DOWN):
            self.move(0, Direction.RIGHT)

    def move(self, velocity, panel):
        if panel == Direction.LEFT:
            self.l_p.v = velocity
        elif panel == Direction.RIGHT:
            self.r_p.v = velocity

