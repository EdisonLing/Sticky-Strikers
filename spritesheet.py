import pygame
import json

class Spritesheet:
    def __init__(self, filename):
        self.filename = filename
        self.spritesheet = pygame.image.load(filename).convert()
        self.meta_data = self.filename.replace('.png', '.json')
        with open(self.meta_data) as f:
            self.data = json.load(f)
        f.close()

    def get_sprite(self, x, y, w, h):
        sprite = pygame.Surface((w, h))
        sprite.blit(self.spritesheet, (0, 0), (x, y, w, h))
        return sprite
    
    def parse_sprite(self, name):
        sprite = self.data['frames'][name]['frame']
        x,y,w,h = sprite['x'], sprite['y'], sprite['w'], sprite['h']
        image = self.get_sprite(x, y, w, h)
        return image