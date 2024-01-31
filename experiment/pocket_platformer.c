#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lib/SMSlib.h"
#include "lib/PSGlib.h"
#include "data.h"
#include "actor.h"

#define SCREEN_W (256)
#define SCREEN_H (192)
#define SCROLL_H (224)

#define MAX_MAP_COLUMNS (64)
#define TILE_PLAYER (15)
#define TILE_LEVEL_START (19)

#define TILE_SIZE (8)

// TODO: Read from config
#define PLAYER_MAX_SPEED ((int) (3.2 * 0x100))
#define PLAYER_GROUND_ACCELLERATION ((int) (0.8 * 0x100))
#define PLAYER_GROUND_FRICTION ((int) (0.65 * 0x100))

actor player;

typedef struct map_header {
	char w, h;
	char objectCount;
} map_header;

typedef struct map_object {
	char x, y, tile;
} map_object;

map_header *pmap_header = (void *) map3_bin;
char *map_colunms[MAX_MAP_COLUMNS];
unsigned int tilemap_buffer[SCREEN_CHAR_H];

void interrupt_handler() {
	PSGFrame();
	PSGSFXFrame();	
}

void load_standard_palettes() {
	SMS_loadBGPalette(background_palette_bin);
	SMS_loadSpritePalette(sprites_palette_bin);
	SMS_setSpritePaletteColor(0, 0);
}

int getTileValueForPosition(int pos) {
	return pos >> 3;
}

int getTileLayerValueByIndex(int y, int x) {
	return map_colunms[x][y];
	// return this.tileMap[y]?.[x];
}

int isPassableTile(char tile) {
	return !tile;
}

void  checkTileCollisions(actor *obj, char cornerCorrection) {
	obj->displacement_x.displacement.w += obj->displacement_x.speed.w;
	obj->x += obj->displacement_x.displacement.b.h;
	obj->displacement_x.displacement.b.h = 0;	

	// collision to the left
	if (obj->displacement_x.speed.w < 0) {
        int leftX = obj->x;
        int bottomY = obj->y + TILE_SIZE - 1;
        int topY = obj->y;

        int left = getTileValueForPosition(leftX);
        int bottom = getTileValueForPosition(bottomY);
        int top = getTileValueForPosition(topY);

        char top_left = getTileLayerValueByIndex(top, left);
        char bottom_left = getTileLayerValueByIndex(bottom, left);

		if (!isPassableTile(top_left) || !isPassableTile(bottom_left)) {
			obj->x = (left + 1) << 3;
		}

		/*
		if (!this.passableTiles.includes(obj.top_left)
			|| !this.passableTiles.includes(obj.bottom_left)) {
			obj.x = (obj.left + 1) * tileMapHandler.tileSize;
			obj.hitWall(AnimationHelper.facingDirections.left);
		}
		*/
	}

	// collision to the right
	else if (obj->displacement_x.speed.w > 0) {
        int rightX = obj->x + TILE_SIZE - 1;
        int bottomY = obj->y + TILE_SIZE - 1;
        int topY = obj->y;
		
        int right = getTileValueForPosition(rightX);
        int bottom = getTileValueForPosition(bottomY);
        int top = getTileValueForPosition(topY);
		
		char top_right = getTileLayerValueByIndex(top, right);
		char bottom_right = getTileLayerValueByIndex(bottom, right);

		if (!isPassableTile(top_right) || !isPassableTile(bottom_right)) {
			obj->x = (right << 3) - (TILE_SIZE + 1);
		}
		/*
		if (!this.passableTiles.includes(obj.top_right)
			|| !this.passableTiles.includes(obj.bottom_right)) {
			obj.x = obj.right * tileMapHandler.tileSize - (obj.width + 1);
			obj.hitWall(AnimationHelper.facingDirections.right);
		}
		*/
	}
}

void  checkCollisionsWithWorld(actor *obj, char cornerCorrection) {
	checkTileCollisions(obj, cornerCorrection);
	/*
    static checkCollisionsWithWorld(obj, cornerCorrection = false) {
        this.checkHazardsCollision(obj);
        this.groundUnderFeet(obj);
        obj.xspeed += obj.bonusSpeed;
        this.checkTileCollisions(obj, cornerCorrection);
        obj.xspeed -= obj.bonusSpeed;
    }
	*/
}

void handle_player_input() {
	unsigned int joy = SMS_getKeysStatus();
	char walking = 0;
	
	if (joy & PORT_A_KEY_LEFT) {
		if (player.displacement_x.speed.w - PLAYER_GROUND_ACCELLERATION > -PLAYER_MAX_SPEED) {
			player.displacement_x.speed.w -= PLAYER_GROUND_ACCELLERATION;
		}
		/*
		if (player.xspeed - player.speed > newMaxSpeed * -1) {
			player.xspeed -= player.speed;
		}
		else if (player.xspeed > newMaxSpeed * -1 && player.wallJumping) {
			player.xspeed -= player.speed;
		}
		else {
			if (player.swimming) {
				player.xspeed = newMaxSpeed * -1;
			}
			else {
				const restSpeed = player.currentMaxSpeed + player.xspeed;
				if (restSpeed > 0) {
					player.xspeed -= restSpeed;
				}
			}
		}
		*/
		walking = 1;
	} else if (joy & PORT_A_KEY_RIGHT) {
		if (player.displacement_x.speed.w + PLAYER_GROUND_ACCELLERATION < PLAYER_MAX_SPEED) {
			player.displacement_x.speed.w += PLAYER_GROUND_ACCELLERATION;
		}
		/*
                if (player.xspeed + player.speed < newMaxSpeed) {
                    player.xspeed += player.speed;
                }
                else if (player.xspeed < newMaxSpeed && player.wallJumping) {
                    player.xspeed += player.speed;
                }
                else {
                    if (player.swimming) {
                        player.xspeed = newMaxSpeed;
                    }
                    else {
                        const restSpeed = player.currentMaxSpeed - player.xspeed;
                        if (restSpeed > 0) {
                            player.xspeed += restSpeed;
                        }
                    }
                }
		*/
		walking = 1;
	}

	if (!walking) {
		player.displacement_x.speed.w = (player.displacement_x.speed.w >> 4) * (PLAYER_GROUND_FRICTION >> 4);
		if (abs(player.displacement_x.speed.w) < 0x80) {
			player.displacement_x.speed.w = 0;
		}
		/*
        if (!player.fixedSpeed) {
            player.xspeed *= player.friction;
            if (Math.abs(player.xspeed) < 0.5) {
                player.xspeed = 0;
            }
        }
		*/
	}	
}

void move_player() {
	checkCollisionsWithWorld(&player, 1);	
}

void draw_background_map(void *map) {
	unsigned int *ch = map;
	
	SMS_setNextTileatXY(0, 0);
	for (char y = 0; y != 24; y++) {
		for (char x = 0; x != 32; x++) {
			unsigned int tile_number = *ch + 256;
			SMS_setTile(tile_number);
			ch++;
		}
	}
}

void draw_background() {
	draw_background_map(background_tilemap_bin);
}

map_object *getObjectList() {
	return (void *) (((char *) pmap_header) + sizeof(map_header) + (pmap_header->w * pmap_header->h));
}


char gameplay_loop() {
	unsigned int joy = 0;

	SMS_waitForVBlank();
	SMS_displayOff();
	SMS_disableLineInterrupt();	

	SMS_VRAMmemset (0, 0, 0xFFFF); 
	clear_sprites();
	
	SMS_loadPSGaidencompressedTiles(title_tiles_psgcompr, 1);	
	SMS_loadBGPalette(title_palette_bin);
	
	SMS_useFirstHalfTilesforSprites(1);
	SMS_loadSpritePalette(title_palette_bin);
	
	SMS_load1bppTiles(font_1bpp, 352, font_1bpp_size, 0, 12);
	SMS_configureTextRenderer(352 - 32);

	// Prepare column pointer table
	char *map_cell = ((char *) pmap_header) + sizeof(map_header);
	for (char colNum = 0; colNum < SCREEN_CHAR_W; colNum++) {
		map_colunms[colNum] = map_cell;
		map_cell += pmap_header->h;
	}

	// Draw background
	for (char colNum = 0; colNum < SCREEN_CHAR_W; colNum++) {
		char *o = map_colunms[colNum];
		unsigned int *d = tilemap_buffer;
		for (char rowNum = 0; rowNum < pmap_header->h; rowNum++) {
			*d = *o;
			d++;
			o++;			
		}
		SMS_loadTileMapArea(colNum, 0, tilemap_buffer, 1, pmap_header->h);
	}
		
	//SMS_loadTileMap(0, 0, title_tilemap_bin, title_tilemap_bin_size);
	
	SMS_displayOn();
	
	// Spawn player
	map_object *objectList = getObjectList();
	for (char objectNum = 0; objectNum < pmap_header->objectCount; objectNum++) {		
		map_object *obj = objectList + objectNum;
		
		if (obj->tile == TILE_LEVEL_START) {
			init_actor(&player, obj->x, obj->y - 8, 1, 1, TILE_PLAYER, 1);
			player.displacement_x.displacement.w = 0;
			player.displacement_x.speed.w = 0;
			player.displacement_y.displacement.w = 0;
			player.displacement_y.speed.w = 0;
		}
	}
	
	while (1) {
		handle_player_input();
		move_player();
		
		SMS_initSprites();	

		// Draw sprites
		map_object *objectList = getObjectList();
		for (char objectNum = 0; objectNum < pmap_header->objectCount; objectNum++) {		
			map_object *obj = objectList + objectNum;
			
			/*
			if (objectNum < 4) {
				SMS_setNextTileatXY(1, objectNum + 1);
				printf("%d: %d, %d, %d", objectNum, obj->x, obj->y, obj->tile);
			}
			*/
			
			SMS_addSprite(obj->x, obj->y - 8, obj->tile);
		}
		
		draw_actor(&player);
		
		SMS_finalizeSprites();	

		SMS_waitForVBlank();
		SMS_copySpritestoSAT();	
	}
}

void main() {
	while (1) {
		gameplay_loop();
	}
}

SMS_EMBED_SEGA_ROM_HEADER(9999,0); // code 9999 hopefully free, here this means 'homebrew'
SMS_EMBED_SDSC_HEADER(0,6, 2024,1,30, "Haroldo-OK\\2024", "Pocket Platformer Converter",
  "Convert Pocket Platformer Projects to SMS.\n"
  "Built using devkitSMS & SMSlib - https://github.com/sverx/devkitSMS");
