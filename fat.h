/*
Simple fat32 implementation 
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

#define DIR_Name_size 11
#define FAT_ID 21

#define LAST_BLOCK -2

/*File Attributes*/
#define Normal_File 0x00000001
#define Hidden 0x00000002
#define System 0x00000004
#define Volume_ID 0x00000008
#define Directory 0x0000010
#define FS_SIZE (100*1024*1024)
#define FS_BLOCKS (FS_SIZE/1024)
#define FAT_SIZE (FS_BLOCKS*4/1024)
#define COMPLETE_FAT FAT_SIZE+1 
#define PARTITION_SIZE FS_BLOCKS+COMPLETE_FAT
// separacao logica fat-dados 
#define OFFSET 101 
// 100mb - 400

struct File{
	struct Dir_entry *dir_entry;
	int read_pos;
	int write_pos;
	int read_block;
	int write_block;
	int flags;
	int parent_dir_block;
	int my_index_on_parent_dir;
};

struct Date{
	short year;
	unsigned char month;
	unsigned char date;
	unsigned char hrs; 
	unsigned char min;
	
};

struct Dir_entry{
	char DIR_Name[DIR_Name_size]; /*11 bytes file name*/
	unsigned char DIR_Attr; /*attributes*/
	int DIR_first_block;
	int DIR_last_block;
	int DIR_FileSize;
	struct Date DIR_creation_date;
};


struct Dir{
	struct Dir_entry entries[32];
};


struct Fat{
	int FAT_ident;
	int sectors[FS_BLOCKS];
};
