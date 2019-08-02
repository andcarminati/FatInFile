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
#define O_RDONLY 0x00000001
#define O_WRONLY 0x00000002
#define O_RDWR 0x00000004

struct Stat{
	//struct Date creation_date;
	int size;
	unsigned char attributes;
	
};

#define DEBUG_FS

#define BLK_SIZE 1024

int Creat(const char * pathname);

int Open(const char * pathname, int flags);
/*Opens the file designated by pathname or create it case it does not exist. Files can be open in tree modes according to *flags:
*
*   1. O_RDONLY: read-only;
*   2. O_WRONLY: write-only;
*   3. O_RDWR: read/write.
*
*The function returns a file descriptor on success or "-1" on failure.*/

int Close(int fd);
/*Closes the file designated by fd. In case of success, this function returns "0", otherwise, "-1".*/

int Read(int fd, void * buf, int count);

/*Attempts to read up to count bytes from the file designated by fd into the buffer starting at buf. On success, the number *of bytes read is returned (zero indicates end of file), and the file position is advanced by this number. On error, "-1" is *returned.*/

int Write(int fd, const void * buf, int count);
/*Writes up to count bytes to the file referenced by the file descriptor fd from the buffer starting at buf. On success, the *number of bytes written are returned (zero indicates nothing was written). On error, "-1" is returned.*/

int Stat(const char * pathname, struct Stat * buf);
/*Gets information about the file designated by pathname and returns it as a stat structure in buf. In case of success, this *function returns "0", otherwise, "-1".*/

int Mkdir(const char * pathname);
/*Attempts to create a directory named pathname. In case of success, this function returns "0", otherwise, "-1".*/

int Rmdir(const char * pathname);
/*Deletes an empty directory designated by pathname. In case of success, this function returns "0", otherwise, "-1".*/
  
