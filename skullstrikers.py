import pygame
from flask import Flask, request
import threading
from spritesheet import Spritesheet

# Initialize the game
pygame.init()

# Set up display
WIDTH, HEIGHT = 800, 400
canvas = pygame.Surface((WIDTH, HEIGHT))
window = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Sticky Strikers!")

# Set up the game clock
clock = pygame.time.Clock()

spritesheetR = Spritesheet('spritesheetR.png')
red = [spritesheetR.parse_sprite('red_idle.png'), spritesheetR.parse_sprite('red_punch.png')]

spritesheetB = Spritesheet('spritesheetB.png')
blue = [spritesheetB.parse_sprite('blue_idle.png'), spritesheetB.parse_sprite('blue_punch.png')]

# Frame widths for blue character
blue_frame_widths = {
    'blue_idle.png': 125,
    'blue_punch.png': 177
}

# Joystick control variables
joystick_command = None

# Player properties
player_width, player_height = 177, 69
player1_x, player1_y = 50, 0
player2_x, player2_y = WIDTH - 175, 0
player_speed = 5
health1, health2 = 10, 10  # Health points
attack_cooldown1, attack_cooldown2 = 0, 0  # Cooldown timers
cooldown_time = 20  # Frames before another attack
punching1, punching2 = False, False
punch_time = 5  # Punch animation duration

def joystick_server():
    global joystick_command
    app = Flask(__name__)

    @app.route('/joystick', methods=['POST'])
    def joystick_data():
        global joystick_command
        data = request.json
        joystick_command = data['command']

        # pygame events for BLUE player
        if joystick_command == "RIGHT_B":
            custom_event = pygame.event.Event(pygame.USEREVENT, command="RIGHT_B")
        elif joystick_command == "LEFT_B":
            custom_event = pygame.event.Event(pygame.USEREVENT, command="LEFT_B")
        elif joystick_command == "PUNCH_B":
            custom_event = pygame.event.Event(pygame.USEREVENT, command="PUNCH_B")
        # Add events for the RED player
        elif joystick_command == "RIGHT_R":
            custom_event = pygame.event.Event(pygame.USEREVENT, command="RIGHT_R")
        elif joystick_command == "LEFT_R":
            custom_event = pygame.event.Event(pygame.USEREVENT, command="LEFT_R")
        elif joystick_command == "PUNCH_R":
            custom_event = pygame.event.Event(pygame.USEREVENT, command="PUNCH_R")
        else:
            custom_event = None

        if custom_event:
            pygame.event.post(custom_event)

        return "OK", 200

    app.run(host='0.0.0.0', port=5000, debug=False, use_reloader=False)

# Run Flask server in a separate thread
server_thread = threading.Thread(target=joystick_server, daemon=True)
server_thread.start()

def handle_punch_r():
    global indexR, attack_cooldown1, punching1, health2
    indexR = 1
    if abs(player1_x - player2_x) < player_width:
        health2 -= 1
    attack_cooldown1 = cooldown_time
    punching1 = True

def handle_punch_b():
    global indexB, attack_cooldown2, punching2, health1
    indexB = 1
    if abs(player2_x - player1_x) < player_width:
        health1 -= 1
    attack_cooldown2 = cooldown_time
    punching2 = True

# Main game loop
running = True
indexR = 0
indexB = 0
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        elif event.type == pygame.USEREVENT:
            # Handle custom events
            if event.command == "RIGHT_B":
                player2_x += player_speed
            elif event.command == "LEFT_B":
                player2_x -= player_speed
            elif event.command == "PUNCH_B" and attack_cooldown2 == 0:
                handle_punch_b()
            elif event.command == "RIGHT_R":
                player1_x += player_speed
            elif event.command == "LEFT_R":
                player1_x -= player_speed
            elif event.command == "PUNCH_R" and attack_cooldown1 == 0:
                handle_punch_r()
        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_d:
                player1_x += player_speed
            elif event.key == pygame.K_a:
                player1_x -= player_speed
            elif event.key == pygame.K_g and attack_cooldown1 == 0:
                handle_punch_r()
            elif event.key == pygame.K_RIGHT:
                player2_x += player_speed
            elif event.key == pygame.K_LEFT:
                player2_x -= player_speed
            elif event.key == pygame.K_l and attack_cooldown2 == 0:
                handle_punch_b()

    # Reduce cooldown timers
    if attack_cooldown1 > 0:
        attack_cooldown1 -= 1
    if attack_cooldown2 > 0:
        attack_cooldown2 -= 1

    # Punch animation duration
    if punching1 and attack_cooldown1 <= cooldown_time - punch_time:
        punching1 = False
        indexR = 0  # Reset to idle state for Player 1
    if punching2 and attack_cooldown2 <= cooldown_time - punch_time:
        punching2 = False
        indexB = 0  # Reset to idle state for Player 2

    # Game logic goes here

    # Drawing code goes here
    # 1. Draw the background
    canvas.fill((255, 255, 255))  # Fill the canvas with white

    # 2. Draw the player sprites
    canvas.blit(red[indexR], (player1_x, player1_y))
    blue_frame_width = blue_frame_widths['blue_punch.png'] if indexB == 1 else blue_frame_widths['blue_idle.png']
    canvas.blit(blue[indexB], (player2_x - (blue_frame_widths['blue_punch.png'] - blue_frame_widths['blue_idle.png']) if indexB == 1 else player2_x, player2_y))

    # 3. Draw the UI elements
    # Health bars
    pygame.draw.rect(canvas, (255, 0, 0), (50, 20, health1 * 20, 20))
    pygame.draw.rect(canvas, (0, 0, 255), (WIDTH - (health2 * 20) - 50, 20, health2 * 20, 20))

    # Blit the canvas onto the window
    window.blit(canvas, (0, 0))
    # Update the display
    pygame.display.update()

    # Cap the frame rate
    clock.tick(60)

# Quit the game
pygame.quit()
