/* Donated to the public domain 2020 Sam Trenholme */

/* This tool reads a C-Evo map file on the standard input.
 * On the standard output, it lists the size of the map and the location
 * of any human player starts on the map */

#include <stdio.h>
#include <stdint.h>

int main() {
	int a;
	int64_t lx = 0;
	int64_t ly = 0;
	int64_t x = 0;
	int64_t y = 0;
	int64_t tile, place;
	float lat;
	if(getc(stdin) != 'c') { puts("Not a C-Evo map"); return 1; }
	if(getc(stdin) != 'E') { puts("Not a C-Evo map"); return 1; }
	if(getc(stdin) != 'v') { puts("Not a C-Evo map"); return 1; }
	if(getc(stdin) != 'o') { puts("Not a C-Evo map"); return 1; }
	if(getc(stdin) != 'M') { puts("Not a C-Evo map"); return 1; }
	if(getc(stdin) != 'a') { puts("Not a C-Evo map"); return 1; }
	if(getc(stdin) != 'p') { puts("Not a C-Evo map"); return 1; }
	for(place = 0; place < 5; place++) {
		if(getc(stdin) != 0) { puts("Not a C-Evo map"); return 1; }
	}
	for(place = 0; place < 4; place++) { getc(stdin); } // MaxTurn
	for(place = 0; place < 4; place++) { lx |= (getc(stdin) << 8*place); }
	for(place = 0; place < 4; place++) { ly |= (getc(stdin) << 8*place); }
	for(y = 0; y < ly; y++) {
		for(x = 0; x < lx; x++) {
			tile = 0;
			for(place = 0; place < 4; place++) {
				tile |= (getc(stdin) << 8*place); 
			}
			// We only care about player start
			tile &= 0x00200000;
			if(tile != 0) {
				lat = (ly / 2) - y;
				if(lat < 0) { lat = 0 - lat; }
				lat /= (ly / 2);
				lat *= 90;
				printf("Start at %d %d Latitude %f\n",x,y,lat); 
			}
		}
	}
}
			
	
