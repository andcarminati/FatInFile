/*
Simple fat32 implementation: test example 
Copyright (C) 2016  Andreu Carminati
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fat.h"
#include "fs.h"

const char *msg = "Teste de gravacao no sistema de arquivos!!";

int main (int argc, char *argv[]){

	printf("User-level File System\n");

	Mkdir("/meudir");
	Mkdir("/meudir3");
	Mkdir("/meudir3");
	Mkdir("/meudir4");

	Mkdir("/meudir/teste");
	Mkdir("/meudir4/teste");
	Mkdir("/meudir4/teste/dir5");

	int fd = Open("/meudir/arquivo", O_RDWR);
	
	if(fd == -1){
		fd = Creat("/meudir/arquivo");
	}	

	Write(fd, msg, strlen(msg));

	char * buf = calloc(1, 100);
	Read(fd, buf, 100);
		
	puts(buf);

	int fd2 = Open("/nenhumdir/nenhumarq", O_RDWR);
	Write(fd2, msg, strlen(msg));

} 
