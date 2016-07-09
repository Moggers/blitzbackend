#include <stdlib.h>
#include <stdio.h>
#include "godreader.hpp"
namespace Gods {
	void getName( char * fname, char * res )
	{
		FILE  *f;
		f = fopen( fname, "r+" );
		if( f == 0x0 ) {
			fprintf( stdout, "Failed to open pretender file %s!\n", fname );
			return;
		}
		int ch;
		int i = 0;
		for( i  = 0; i < 38; i++ ) {
			fgetc(f);
		}
		while( (ch = fgetc(f)) != 0x4f) {
			if( ch == EOF ) {
				fclose(f);
				return;
			}
		}
		while( (ch = fgetc(f)) != 0x78) {
			if( ch == EOF ) {
				fclose(f);
				return;
			}
		}
		while( (ch = fgetc(f)) != 0x78) {
			if( ch == EOF ) {
				fclose(f);
				return;
			}
		}
		for( i  = 0; i < 80; i++ ) {
			if( fgetc(f) == EOF ) {
				fclose(f);
				return;
			}
		}
		i = 0;
		while( (ch = fgetc(f)) != 0x4f ) {
			if( ch == EOF ) {
				fclose(f);
				return;
			}
			res[i] = ch ^ 0x4f;
			res[i+1] = 0x0;
			i++;
		}
		fclose( f );
	}
};
