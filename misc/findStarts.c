/* Donated to the public domain 2020-2021 Sam Trenholme */

/* This tool reads a C-Evo map file on the standard input.
 * On the standard output, it lists the size of the map and the location
 * of any human player starts on the map */

#include <stdio.h>
#include <stdint.h>

int main(int argc, char **argv) {
	int a;
	int32_t lx = 0;
	int32_t ly = 0;
	int32_t x = 0;
	int32_t y = 0;
	int32_t place = 0;
	int64_t tile;
	float lat;
	FILE *f;
	if(argc == 2) {
		f = fopen(argv[1],"rb");
		if(f == NULL) {	
			printf("Error opening file %s\n",argv[1]);
			return 1;
		}
	} else {
		f = stdin;
	}
	if(getc(f) != 'c') { puts("Not a C-Evo map"); return 1; }
	if(getc(f) != 'E') { puts("Not a C-Evo map"); return 1; }
	if(getc(f) != 'v') { puts("Not a C-Evo map"); return 1; }
	if(getc(f) != 'o') { puts("Not a C-Evo map"); return 1; }
	if(getc(f) != 'M') { puts("Not a C-Evo map"); return 1; }
	if(getc(f) != 'a') { puts("Not a C-Evo map"); return 1; }
	if(getc(f) != 'p') { puts("Not a C-Evo map"); return 1; }
	for(place = 0; place < 5; place++) {
		if(getc(f) != 0) { puts("Not a C-Evo map"); return 1; }
	}
	for(place = 0; place < 4; place++) { getc(f); } // MaxTurn
	for(place = 0; place < 4; place++) { lx |= (getc(f) << 8*place); }
	for(place = 0; place < 4; place++) { ly |= (getc(f) << 8*place); }
	for(y = 0; y < ly; y++) {
		for(x = 0; x < lx; x++) {
			tile = 0;
			for(place = 0; place < 4; place++) {
				tile |= (getc(f) << 8*place); 
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
			
	
