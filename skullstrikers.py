```import pygame

# Initialize pygame
pygame.init()

# Constants
WIDTH, HEIGHT = 800, 400
WHITE = (255, 255, 255)
RED = (255, 0, 0)
BLUE = (0, 0, 255)
BLACK = (0, 0, 0)

# Create window
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Stick Figure Boxing")
clock = pygame.time.Clock()

# Load background
bg = pygame.image.load("background.jpg")  # Ensure you have a suitable background image
bg = pygame.transform.scale(bg, (WIDTH, HEIGHT))
scroll_x = 0
scroll_speed = 2

# Player properties
player_width, player_height = 40, 100
player1_x, player1_y = 200, HEIGHT - player_height - 20
player2_x, player2_y = 600, HEIGHT - player_height - 20
player_speed = 5
health1, health2 = 10, 10  # Health points
attack_cooldown1, attack_cooldown2 = 0, 0  # Cooldown timers
cooldown_time = 20  # Frames before another attack
punching1, punching2 = False, False
punch_time = 5  # Punch animation duration

# Game loop
running = True
while running:
    clock.tick(30)  # 30 FPS
    
    # Scroll background
    scroll_x -= scroll_speed
    if scroll_x <= -WIDTH:
        scroll_x = 0
    screen.blit(bg, (scroll_x, 0))
    screen.blit(bg, (scroll_x + WIDTH, 0))
    
    # Event handling
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
    
    # Player movement
    keys = pygame.key.get_pressed()
    
    # Player 1
    if keys[pygame.K_a]:
        player1_x -= player_speed
    if keys[pygame.K_d]:
        player1_x += player_speed
    if keys[pygame.K_g] and attack_cooldown1 == 0:  # Attack with cooldown
        if abs(player1_x - player2_x) < player_width:  # If close enough
            health2 -= 1
        attack_cooldown1 = cooldown_time
        punching1 = True
    
    # Player 2
    if keys[pygame.K_LEFT]:
        player2_x -= player_speed
    if keys[pygame.K_RIGHT]:
        player2_x += player_speed
    if keys[pygame.K_l] and attack_cooldown2 == 0:  # Attack with cooldown
        if abs(player2_x - player1_x) < player_width:
            health1 -= 1
        attack_cooldown2 = cooldown_time
        punching2 = True
    
    # Reduce cooldown timers
    if attack_cooldown1 > 0:
        attack_cooldown1 -= 1
    if attack_cooldown2 > 0:
        attack_cooldown2 -= 1
    
    # Punch animation duration
    if punching1 and attack_cooldown1 <= cooldown_time - punch_time:
        punching1 = False
    if punching2 and attack_cooldown2 <= cooldown_time - punch_time:
        punching2 = False
    
    # Draw players with attack animation
    def draw_stick_figure(x, y, color, punching):
        pygame.draw.circle(screen, color, (x + 20, y - 15), 15)  # Head
        pygame.draw.line(screen, color, (x + 20, y), (x + 20, y + player_height), 5)  # Body
        if punching:
            pygame.draw.line(screen, color, (x + 20, y + 30), (x + 60, y + 30), 5)  # Punching Arm
        else:
            pygame.draw.line(screen, color, (x, y + 30), (x + 40, y + 30), 5)  # Arms
        pygame.draw.line(screen, color, (x + 20, y + player_height), (x, y + player_height + 20), 5)  # Left Leg
        pygame.draw.line(screen, color, (x + 20, y + player_height), (x + 40, y + player_height + 20), 5)  # Right Leg
    
    draw_stick_figure(player1_x, player1_y, RED, punching1)
    draw_stick_figure(player2_x, player2_y, BLUE, punching2)
    
    # Health bars
    pygame.draw.rect(screen, RED, (50, 20, health1 * 20, 20))
    pygame.draw.rect(screen, BLUE, (WIDTH - (health2 * 20) - 50, 20, health2 * 20, 20))
    
    # Check for game over
    if health1 <= 0 or health2 <= 0:
        running = False
    
    pygame.display.flip()

pygame.quit()
```