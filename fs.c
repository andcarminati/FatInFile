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
#include "fs.h"
#include "fat.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

static initialized = 0;
static int fs_file;
static struct Fat *my_fat;
const char *fs_file_name = "fs.dat";
LIST *open_files;
int open_count = 0;


static int fill_file(int fd, int blocks);
static int write_fat(void);
static int read_fat(void);
static int initialize(void);
static int verify_if_init(void);
static int format_volume(void);
static int is_valid_volume(int fd);
static int fs_read(int n, void * buf);
static int get_next_block_index(int blk);
static int get_free_block_index(void);
static int get_pthname_size(const char * pathname);
static int get_sub_pthname(const char * pathname, char * subpath);
static int get_file_name(const char * pathname, char * name);
static void set_next_block_index(int blk, int nxt_block);
static int read_block_rel_to_fat(int n, void * buf);
static int write_block_rel_to_fat(int n, void * buf);
static struct Dir_entry* get_file(const char * pathname);
/*Reads the block of order n (first block = 0) from the virtual disk into the buffer pointed by buf. The function returns "0" on success and "-1" on error.*/

static int fs_write(int n, void * buf);
/*Writes the data pointed by buf into the n-th block (first block = 0) of the virtual disk. The function returns "0" on success and "-1" on error.*/

static int fs_read(int n, void * buf){
	n = n*1024;
	char *buff = buf; 
	lseek(fs_file, n, SEEK_SET);
	int ret = read(fs_file, buff, 1024);
	if(ret == 1024){
		
		return 0;
	} else {
		perror("Erro");
		return -1;
	}
}

static int fs_write(int n, void * buf){
	n = n*1024;
	char *buff = buf; 
	if(lseek(fs_file, n, SEEK_SET) == -1){
		perror("FS_WRITE: ");
		
	}
	int ret = write(fs_file, buff, 1024);
	if(ret == -1){
		perror("FS_WRITE: ");
	}
}

static int get_free_block_index(void){
	int fr_blk = my_fat->sectors[1];
	my_fat->sectors[1] = my_fat->sectors[fr_blk];
	my_fat->sectors[fr_blk] = LAST_BLOCK;
	write_fat();
	return fr_blk;
}

static int get_next_block_index(int blk){
	int *fat_table = my_fat->sectors;
	return fat_table[blk];
}

static void set_next_block_index(int blk, int nxt_block){
	int *fat_table = my_fat->sectors;
	fat_table[blk] = nxt_block;
}

static int read_block_rel_to_fat(int n, void * buf){
	return fs_read(n+OFFSET, buf);
}

static int write_block_rel_to_fat(int n, void * buf){
	return fs_write(n+OFFSET, buf);
}

static int get_pthname_size(const char * pathname){
	char *token = NULL;
	char *pthname_cpy = malloc(strlen(pathname));
	int len = 0;

	strcpy(pthname_cpy, pathname);
	token = strtok(pthname_cpy, "/");

	while(token != NULL){
		len++;
		token = strtok(NULL, "/");
	}
	free(pthname_cpy);
	
	return len;
}

static int get_sub_pthname(const char * pathname, char * subpath){
	char *token = NULL;
	char *pthname_cpy = malloc(strlen(pathname));
	int size = get_pthname_size(pathname);
	char * subpath_aux = subpath;
	int i = 0;
	

	strcpy(pthname_cpy, pathname);
	token = strtok(pthname_cpy, "/");

	while(i++ < size -1){
		
		subpath_aux = memcpy(subpath_aux , "/", sizeof("/"));
		subpath_aux++;
		
		int token_size = strlen(token);
		memcpy(subpath_aux , token, token_size);
		subpath_aux += token_size;

		token = strtok(NULL, "/");
	}
}

static int get_file_name(const char * pathname, char * name){
	int size = get_pthname_size(pathname);
	char *token = NULL;
	char *pthname_cpy = malloc(strlen(pathname));
	int i = 0;
	

	strcpy(pthname_cpy, pathname);
	token = strtok(pthname_cpy, "/");

	while(i++ < size -1){
		token = strtok(NULL, "/");
		
	}
	strcpy(name, token);
}

static struct Dir_entry* get_file(const char * pathname){
	int my_dir = 0;
	void *buffer = malloc(sizeof(struct Dir));

	if(read_block_rel_to_fat(my_dir, buffer) == -1){
		puts("GET_FILE:");
		return NULL;
	}
	struct Dir *dir = (struct Dir*) buffer;
	struct Dir_entry *file;

	char *token = NULL;
	char *pthname_cpy = malloc(13);
	strcpy(pthname_cpy, pathname);
	token = strtok(pthname_cpy, "/");

	int i;
	/*tokenizar e achar*/
	for(i = 0; i < 32 && token != NULL; i++){
		file = &dir->entries[i];
		/*end of directory*/
		if(file->DIR_Attr == 0){
			return NULL;
			break;
		}	
		/*file was found or path was found*/
		if(strcmp(token, file->DIR_Name) == 0){
			token = strtok(NULL, "/");
			/*file was found*/
			if(token == NULL){

				return file;
			/*else, path was found, continue....*/
			} else {
				if(file->DIR_Attr & Directory){
					my_dir = file->DIR_first_block;
					if(read_block_rel_to_fat(my_dir, buffer) == -1){
						puts("Error block loading.");
						free(pthname_cpy);
						free(buffer);
						return NULL;
					} else {
						i = -1;
						dir = (struct Dir*) buffer;
						continue;
					}	
				}
			}
		} else {
			int next = get_next_block_index(my_dir);
			if(i == 31 && next != LAST_BLOCK){
				if(read_block_rel_to_fat(my_dir, buffer) == -1){
					free(pthname_cpy);
					free(buffer);
					return NULL;
				}
				i = -1;
				dir = (struct Dir*) buffer;
			} else {
				continue;
			}	
		}
		token = strtok(NULL, "/");	
	}
	return file;  
}

static int read_fat(void){
	my_fat = malloc(sizeof(struct Fat));
	
	if(my_fat == NULL){
		return -1;
	}
	void *ch_fat = my_fat;
	int i;
	for(i = 0; i < 101; i++){
		fs_read(i, ch_fat);
		ch_fat += 1024;
		
	}	

#ifdef DEBUG_FS
	int *fat_table = my_fat->sectors;	
#endif
}

static int write_fat(void){
	char *ch_fat = (char *) my_fat;
	int i;
	for(i = 0; i < 101; i++){
		if(fs_write(i, ch_fat) == -1){
			perror("WRITE_FAT:");
		}
		ch_fat += 1024;
	}

}

static int format_volume(){
	my_fat = malloc(sizeof(struct Fat));
	if(my_fat == NULL){
		return -1;
	}
	my_fat->FAT_ident = FAT_ID;
	int *fat_table = my_fat->sectors;
	
	fat_table[0] = LAST_BLOCK; /*root*/
	int i;
	for(i = 1; i < FS_BLOCKS-1; i++){
		fat_table[i] = i+1;
#ifdef DEBUG_FS
		//printf("bloco: %d \n", i);
#endif
	}	
	fat_table[FAT_SIZE-1] = LAST_BLOCK;
	write_fat();
	return 0;
}

static int is_valid_volume(int fd){
	int code;
	char block[1024]; 
	int ret = fs_read(0, &block);
		if(ret == -1){
			puts("IS_VALID_VOLUME: identification error. \n");
			return 0;
		}
	code = block[0];
	if(code == FAT_ID){
		return 1;
	}
	return 0;
}

static int fill_file(int fd, int blocks){
	unsigned char zeros[BLK_SIZE/2];
	int i;
	for(i = 0; i < BLK_SIZE/2; i++){
		zeros[i] = 0;
	}
	for(i = 0; i < blocks*2; i++){
		int ret = write(fd, zeros, BLK_SIZE/2);
		if(ret < BLK_SIZE/2){
			perror("FILL_FILE:");
		}
	}
}


static int initialize(void){
	struct stat *file_stat;
	fs_file = open(fs_file_name, O_RDWR | O_CREAT, S_IRWXU);
	if(fs_file == -1){
		perror("INITIALIZE: fatal error, aborting.");
		return -1;
	}
	file_stat = malloc(sizeof(struct stat));
	stat(fs_file_name, file_stat);
	if(file_stat->st_size == 0){
		fill_file(fs_file, PARTITION_SIZE);
		printf("INITIALIZE: Volume size : %d blocks\n", PARTITION_SIZE);
		printf("INITIALIZE: Formatting...");
		
		if(format_volume() == 0){
			printf("OK\n");
		} else {
			printf("FAIL\n");
			return -1;
		}
	} else {
		printf("INITIALIZE: fs.dat exists, validating file system...");
		if(is_valid_volume(fs_file)){
			printf("OK\n");
		} else {
			printf("FAIL\n");
			return -1;
		}
		read_fat();
		
	}
	open_files = alloc_list();
	return 0;
}

static int verify_if_init(void){
	int ret = 0;
	if(!initialized){
		int ret = initialize();
		initialized = 1;
	}
	return ret;
}

int Open(const char * pathname, int flags){
	if(verify_if_init() == -1){
		return -1;
	}
	struct Dir_entry *file;
	file = get_file(pathname);
	if(file == NULL){
#ifdef DEBUG_FS
		puts("OPEN: File don't exists.");
#endif
		return -1;
	} 
	if(file->DIR_Attr & Directory){
#ifdef DEBUG_FS
		puts("OPEN: Can't open directories.");
#endif
		return -1;
	}
	struct File *open_file = malloc(sizeof(struct File));

	open_file->dir_entry = file;
	open_file->write_pos = 0;
	open_file->read_pos = 0;
	open_file->read_block = file->DIR_first_block;
	open_file->write_block = file->DIR_first_block;
	open_file->flags = flags;
	//open_file->parent_dir_block = root_index;
	//open_file->my_index_on_parent_dir = i;
	
	int fd = open_count++;
	
	insert(open_files, fd, open_file);
#ifdef DEBUG_FS
		puts("OPEN: Success.");
#endif
	return fd;
}

int Close(int fd){
	if(verify_if_init() == -1){
		return -1;
	}
	struct File *open_file = (struct File*)get(open_files, fd);
	if(open_file == NULL){
		/*wrong file descriptor*/
#ifdef DEBUG_FS
		puts("CLOSE: wrong file descriptor.");
#endif
		return -1;
	}
	rem(open_files, fd);
	free(open_file);
#ifdef DEBUG_FS
		puts("CLOSE: Success.");
#endif	
	return 0;
}

int Read(int fd, void * buf, int count){
	if(verify_if_init() == -1){
		return -1;
	}
	if(!contains(open_files, fd)){
		/*no file associated with this file descripor*/
#ifdef DEBUG_FS
		puts("READ: Invalid file descriptor.");
#endif
		return -1;
	}
	struct File *file = (struct File*)get(open_files, fd);
	struct Dir_entry *entry = file->dir_entry;

	//printf("%d\n", file->flags);

	if(file->flags == 2){
		/*illegal opperation*/
#ifdef DEBUG_FS
		puts("This file is O_WRONLY, you can't read.");
#endif
		return -1;
	}

	int readed = 0;
	int position = file->read_pos % 1024;
	int in_block = 1024 - position;
	char *block = malloc(1024);
	if(read_block_rel_to_fat(file->read_block, block) == -1){
		puts("READ: error");
		return -1;
	}
	
	if(count > in_block){
		readed = in_block;

	} else {
		readed = count;
	}
	file->read_pos += readed;
	int i;
	char *buff = (char *) buf;
	char *block_aux = block + position;
	for(i = 0; i < readed; i++){
		 buff[i]  = block_aux[i];
	}
	free(block);


#ifdef DEBUG_FS
		puts("READ: Success.");
#endif
	return readed;
}

int Write(int fd, const void * buf, int count){
	if(verify_if_init() == -1){
		return -1;
	}
	if(!contains(open_files, fd)){
		/*no file associated with this file descripor*/
		puts("WRITE: Invalid file descriptor.");
		return -1;
	}
	struct File *file = (struct File*)get(open_files, fd);
	struct Dir_entry *entry = file->dir_entry;

	if(file->flags == 1){
		/*illegal opperation*/
#ifdef DEBUG_FS
		puts("WRITE: This file is O_RDONLY, you can't write.");
#endif
		return -1;
	}

	int writed = 0;
	int position = file->write_pos % 1024;
	int in_block = 1024 - position;
	char *block = malloc(1024);
	if(read_block_rel_to_fat(file->write_block, block) == -1){
		puts("WRITE: error");
		return -1;
	}

	//char *dir_block = malloc(1024);
	//struct Dir *dir = read_block_rel_to_fat(file->parent_dir_block, dir_block);
	//struct Dir_entry *entry = &dir->entries[file->my_index_on_parent_dir];
	
	if(count > in_block){
		writed = in_block;
		int new_block = get_free_block_index();
		set_next_block_index(file->write_block, new_block);
		//file->write_block = new_block;
		//file->dir_entry->DIR_FileSize += writed;
		//file->dir_entry->DIR_last_block = new_block;

		/*changing dir_entry on dir*/

		
		write_fat();
	} else {
		writed = count;
	}
	file->write_pos += writed;
	
	int i;
	char *buff = (char *) buf;
	char *block_aux = block + position;
	for(i = 0; i < writed; i++){
		block_aux[i] = buff[i];
	}
	write_block_rel_to_fat(file->write_block, block);
	free(block);


#ifdef DEBUG_FS
		puts("WRITE: Success");
#endif
	return writed;
}

int Stat(const char * pathname, struct Stat * buf){
	if(verify_if_init() == -1){
		return -1;
	}
	struct Dir_entry *file;

	file = get_file(pathname);
	if(file == NULL){
		return -1;
	} else {
		buf->size = file->DIR_FileSize;
		buf->attributes = file->DIR_Attr;
		// incluir data
	}
	return 0;
}

int Mkdir(const char * pathname){
	if(verify_if_init() == -1){
		return -1;
	}
	int path_size = get_pthname_size(pathname);
	int root_index = 0;
	int i;
	void *buffer = malloc(sizeof(struct Dir));
	struct Dir *dir = (struct Dir*) buffer;
	struct Dir_entry *file;
	char *pthname_cpy = malloc(strlen(pathname));
	char *name, *subpathname;
	name = malloc(strlen(pathname));

	strcpy(pthname_cpy, pathname);
#ifdef DEBUG_FS
	//printf("Path size : %d \n", path_size);
#endif	
	//name = strtok(pthname_cpy, "/");
	get_file_name(pathname, name);
	if(path_size == 0 | strlen(name) > DIR_Name_size){
		free(buffer);
		free(pthname_cpy);
		puts("MKDIR: error, name too long");
		return -1;
	}

	if(get_file(pathname) != NULL){
		free(buffer);
		free(pthname_cpy);
		printf("MKDIR: Directory %s already exists!\n", name);
		return -1;
	}
	//name = strtok(pthname_cpy, "/");
	if(path_size == 1){
		root_index = 0;
		name = strtok(pthname_cpy, "/");
		
	} else {
		subpathname = malloc(strlen(pathname));
		get_sub_pthname(pathname, subpathname);
		struct Dir_entry *dir = get_file(subpathname);
		//puts(subpathname);
		if(dir == NULL){
			free(buffer);
			free(pthname_cpy);
			free(subpathname);
			puts("MKDIR: error, path does not exits");
			return -1;
		} 
		if(!(dir->DIR_Attr | Directory)){
			free(buffer);
			free(pthname_cpy);
			free(subpathname);
			puts("MKDIR: error, invalid path");
			return -1;
		} 
		root_index = dir->DIR_first_block;
		//puts("aqui");
		//get_file_name(pathname, name);
	}
#ifdef DEBUG_FS
	printf("MKDIR: Creating dir with name : %s \n", name);
#endif
	if(read_block_rel_to_fat(root_index, buffer) == -1){
		free(buffer);
		free(pthname_cpy);
		free(subpathname);
		puts("MKDIR: Error on block load.");
		return -1;
	}

	for(i = 0; i < 32; i++){
		file = &dir->entries[i];
		/*end of directory*/
		if(file->DIR_Attr == 0){
#ifdef DEBUG_FS
			//printf("Dir i : %d \n", i);
#endif
			file->DIR_Attr = Directory;
			file->DIR_FileSize = 0;
			strcpy(file->DIR_Name, name);
				// falta creation date;
			file->DIR_first_block = get_free_block_index();
			file->DIR_last_block = file->DIR_first_block;
#ifdef DEBUG_FS
//			printf("Dir first block : %d \n", file->DIR_first_block);
#endif
			if(i  == 31){
				set_next_block_index(root_index, get_free_block_index());
			}
			if(write_fat() == -1){
				free(dir);
				free(pthname_cpy);
				puts("MKDIR: Error on fat writing.");
				perror("Erro");
				return -1;
			}
			if(write_block_rel_to_fat(root_index, buffer) == -1){
				free(dir);
				free(pthname_cpy);
				puts("MKDIR: Error on block flush.");
				return -1;
			}			
			break;
		}
		if(i == 31){
			root_index = get_next_block_index(root_index);
			if(read_block_rel_to_fat(root_index, buffer) == -1){
				free(dir);
				free(pthname_cpy);
				puts("MKDIR: Error on block load. aborting..");
				return -1;
			}
			i = -1;
			continue;
		}
	}
	return 0;
}

int Rmdir(const char * pathname){
	if(verify_if_init() == -1){
		return -1;
	}
	struct Dir_entry *file;
	file = get_file(pathname);
	if(file == NULL){
		return -1;
	} 
	
}

int Creat(const char * pathname){
	if(verify_if_init() == -1){
		return -1;
	}
	int path_size = get_pthname_size(pathname);
	int root_index = 0;
	int i;
	void *buffer = malloc(sizeof(struct Dir));
	struct Dir *dir = (struct Dir*) buffer;
	struct Dir_entry *file;
	char *pthname_cpy = malloc(strlen(pathname));
	char *name, *subpathname;
	name = malloc(strlen(pathname));

	strcpy(pthname_cpy, pathname);
#ifdef DEBUG_FS
	//printf("Path size : %d \n", path_size);
#endif	
	//name = strtok(pthname_cpy, "/");
	get_file_name(pathname, name);
	if(path_size == 0 || strlen(name) > DIR_Name_size){
		free(buffer);
		free(pthname_cpy);
		puts("CREAT: error, name too long");
		return -1;
	}

	if(get_file(pathname) != NULL){
		free(buffer);
		free(pthname_cpy);
		printf("CREAT: error, file %s already exists!\n", name);
		return -1;
	}
	//name = strtok(pthname_cpy, "/");
	if(path_size == 1){
		root_index = 0;
		name = strtok(pthname_cpy, "/");
		
	} else {
		subpathname = malloc(strlen(pathname));
		get_sub_pthname(pathname, subpathname);
		struct Dir_entry *dir = get_file(subpathname);
		//puts(subpathname);
		if(dir == NULL){
			free(buffer);
			free(pthname_cpy);
			free(subpathname);
			puts("CREAT: error, path does not exits");
			return -1;
		} 
		if(!(dir->DIR_Attr | Directory)){
			free(buffer);
			free(pthname_cpy);
			free(subpathname);
			puts("CREAT: error, invalid path");
			return -1;
		} 
		root_index = dir->DIR_first_block;
		//puts("aqui");
		//get_file_name(pathname, name);
	}
#ifdef DEBUG_FS
	printf("CREAT: Creating file with name : %s \n", name);
#endif
	if(read_block_rel_to_fat(root_index, buffer) == -1){
		free(buffer);
		free(pthname_cpy);
		free(subpathname);
		puts("CREAT: error on block load.");
		return -1;
	}

	for(i = 0; i < 32; i++){
		file = &dir->entries[i];
		/*end of directory*/
		if(file->DIR_Attr == 0){
#ifdef DEBUG_FS
			//printf("Dir i : %d \n", i);
#endif
			file->DIR_Attr = Normal_File;
			file->DIR_FileSize = 0;
			strcpy(file->DIR_Name, name);
				// falta creation date;
			file->DIR_first_block = get_free_block_index();
			file->DIR_last_block = file->DIR_first_block;
#ifdef DEBUG_FS
			//printf("Dir first block : %d \n", file->DIR_first_block);
#endif
			if(i  == 31){
				set_next_block_index(root_index, get_free_block_index());
			}
			if(write_fat() == -1){
				free(dir);
				free(pthname_cpy);
				puts("CREAT: error on fat writing.");
				//perror("Erro");
				return -1;
			}
			if(write_block_rel_to_fat(root_index, buffer) == -1){
				free(dir);
				free(pthname_cpy);
				puts("CREAT: error on block flush.");
				return -1;
			}			
			break;
		}
		if(i == 31){
			root_index = get_next_block_index(root_index);
			if(read_block_rel_to_fat(root_index, buffer) == -1){
				free(dir);
				free(pthname_cpy);
				puts("CREAT: error on block load. aborting..");
				return -1;
			}
			i = -1;
			continue;
		}
	}
	struct File *open_file = malloc(sizeof(struct File));

	open_file->dir_entry = file;
	open_file->write_pos = 0;
	open_file->read_pos = 0;
	open_file->read_block = file->DIR_first_block;
	open_file->write_block = file->DIR_first_block;
	open_file->flags = 4;
	//open_file->parent_dir_block = root_index;
	//open_file->my_index_on_parent_dir = i;
	int fd = open_count++;
	
	insert(open_files, fd, open_file);

	return fd;

}
