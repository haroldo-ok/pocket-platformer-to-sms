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

void handle_player_input() {
	unsigned int joy = SMS_getKeysStatus();
	
	if (joy & PORT_A_KEY_LEFT) {
		player.x--;
	} else if (joy & PORT_A_KEY_RIGHT) {
		player.x++;
	}

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
		}
	}
	
	while (1) {
		handle_player_input();
		
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

	return 0;
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
