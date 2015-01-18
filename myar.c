// Name: Nikolay Goncharenko
// Email: goncharn@onid.oregonstate.edu
// Class: CS344-400
// Assignment: homework#3 Problem3

// myar -t junk.a file1 file2 file3 file4
// myar -v junk.a file1 file2 file3 file4
// myar -q arc.a 1-s.txt 2-s.txt 3-s.txt 4-s.txt 5-s.txt
// myar -d arc.a 3-s.txt

#include <ctype.h> // for isprint()
#include <string.h>
#include <getopt.h>
#include <ar.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <utime.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>

#include <sys/utsname.h>
#include <time.h>

#define BUF_SIZE 1024
#define HEADER_SIZE 60

// functions' headers
char *filePermStr(mode_t, int);
char *get_perm(struct ar_hdr *, int);
char *get_name(struct ar_hdr *);
void print_name(struct ar_hdr *);
void print_perm(struct ar_hdr *file_head);
void print_uid_gid(struct ar_hdr *file_head);
void print_size(struct ar_hdr *file_head);
void print_date(struct ar_hdr *file_head);
void print_file_header(struct ar_hdr *file_head);
int format_check(int fd);
ssize_t read_archive(int fd, struct ar_hdr *file_head);
void t_archive(int fd, struct ar_hdr *file_head);
void v_archive(int fd, struct ar_hdr *file_head);
int open_archive(char *arch_name);
int append_archive(char *arch_name);
void write_archive_header(int fd, char *arch_name);
void write_file_header(int archive_fd, struct ar_hdr *file_head);
void write_file_content(int archive_fd, int cur_file_fd, struct ar_hdr *file_head);
void write_file(int archive_fd, int cur_file_fd, char* file_to_add);
int is_file_exist(int fd, char* tested_file);
int create_file(char *arch_name);
void copy_file_content();
void restore_date(struct ar_hdr *file_head, char *tested_file);
void restore_perm(struct ar_hdr *file_head, char *tested_file);
int is_file_exist(int fd, char* tested_file);
void x_file(int fd, struct ar_hdr *file_head, off_t curr, char* x_file);




char *get_perm(struct ar_hdr *file_header, int size)
{
    char *out_string = malloc(size * sizeof(char));
    
    memcpy(out_string, &(file_header->ar_mode), size);
    out_string[size] = '\0'; 
   
    return out_string;
}

char *get_name(struct ar_hdr *file_header){
    // size of ar_name
    int ar_name_size = sizeof(file_header->ar_name);
    
    char file_name[ar_name_size];
    
    // name without '/' symbol
    char *name = malloc((ar_name_size-1) * sizeof(char));
    
    memcpy(file_name, file_header, ar_name_size);
    file_name[ar_name_size-1] = '\0'; 
    
    int i = 0;
    for (i = 0; file_name[i] != '/'; i++)
        name[i] = file_name[i];
    
    name[i] = '\0';
    
    return name;
    
}

void print_name(struct ar_hdr *file_header){
    
    printf("%s", get_name(file_header));
       
}

void print_perm(struct ar_hdr *file_head){
    
    // translate into octal and print
    long int perm = strtol(file_head->ar_mode, NULL, 8);  
    printf("%s ", filePermStr(perm, 0));
}

void print_uid_gid(struct ar_hdr *file_head){
    
    printf("%d", atoi(file_head->ar_uid));   
    printf("%c", '/');
    printf("%d ", atoi(file_head->ar_gid));
}

void print_size(struct ar_hdr *file_head){
    
     //file_head->ar_size[10] = '\0';
     printf("%6d ", atoi(file_head->ar_size));
}

void print_date(struct ar_hdr *file_head){
    
    int buf_size = 10;
    // print date data
    char time_string[buf_size];
    time_t t = atoll(file_head->ar_date);       
    struct tm *local = localtime(&t);
    
    // print abbreviated month name
    strftime(time_string, 100, "%b", local);
    printf("%s ", time_string);
    
    // print day of month
    strftime(time_string, 100, "%e", local);
    printf("%s ", time_string);
    
    // print time hh:mm 
    strftime(time_string, 100, "%R", local);
    printf("%s ", time_string);
    
    // print year 
    strftime(time_string, 100, "%Y", local);
    printf("%s ", time_string);
}

void print_file_header(struct ar_hdr *file_head){
    
    print_perm(file_head);

    print_uid_gid(file_head);

    print_size(file_head);

    // print date data
    print_date(file_head);

    // file name
    print_name(file_head);

    printf("\n");
}

int format_check(int fd)
{
    ssize_t num_read;
    char comp_str[SARMAG];
    
    //reset to start of the archive
    lseek(fd, 0, SEEK_SET);
    
    // check if the file is a proper archive
    // read an archive header
    if ((num_read = read(fd, &comp_str, SARMAG)) == -1)
        perror("read error");
    // add null term char
    comp_str[SARMAG] = '\0';
    
    // check if the archive file of a proper format
    if(strcmp(comp_str, ARMAG) != 0){ 
        //printf("buf_read = %s  strlen = %ld\n", comp_str, strlen(comp_str));
        //printf("buf_must_be = %s strlen = %ld\n", ARMAG, strlen(ARMAG));
        
        return 0;
        //printf("The archive file is not of a proper format\n");
        //exit(EXIT_FAILURE);
    }
    
    else
        return 1;
}


ssize_t read_archive(int fd, struct ar_hdr *file_head){
        
        ssize_t num_read;
        off_t curr;
                
        // find file_size
        long int file_size = atol(file_head->ar_size);
        
        // create buffer to read content of file
        char file_buffer[file_size];
        
        if ((num_read = read(fd, file_buffer, file_size)) == -1)
            perror("read error");
        
        file_buffer[file_size] = '\0';
        
        // make one step foreard if file size is odd
        if (file_size % 2 != 0)
            curr = lseek(fd, 1, SEEK_CUR);
               
        //printf("current = %ld   num_read = %ld\n", curr, num_read);  
        
        // print file_name
        //print_name(fd, file_head);
        
        // read next header
        if ((num_read = read(fd, file_head, sizeof(*file_head))) == -1)
            perror("read error");
        
        return num_read;
}




void t_archive(int fd, struct ar_hdr *file_head){
      
    ssize_t num_read;
    // check that archive has correct header
    if (!format_check(fd)){
        printf("Error: The archive file is not of a proper format\n");
        exit(EXIT_FAILURE);
    }
    
    // read a 1st header
    if ((num_read = read(fd, file_head, sizeof(*file_head))) == -1)
        perror("read error");
    // print 1st header
    print_name(file_head);
   //printf(" %d", file_head->ar_name[10]);
    printf("\n");
       
    while(read_archive(fd, file_head) == sizeof(*file_head)){   
        print_name(file_head);
        printf("\n");
    }
    
}


void v_archive(int fd, struct ar_hdr *file_head){
      
    ssize_t num_read;
    
    // check archive format
    if (!format_check(fd)){
        printf("Error: The archive file is not of a proper format\n");
        exit(EXIT_FAILURE);
    }
       
    // read a 1st header
    if ((num_read = read(fd, file_head, sizeof(*file_head))) == -1)
        perror("read error");
    
    // print 1st file header
    print_file_header(file_head);
    
    // read the rest file headers and print data
    while(read_archive(fd, file_head) == sizeof(*file_head)){   
        
        print_file_header(file_head);
    }
    
    //free(file_perm);
}


int open_archive(char *arch_name){
    
    // for error string concatenation
    char error_str[50];
    
    // open archive file for reading
    int read_fd = open(arch_name, O_RDONLY);    
    
    if (read_fd == -1){
        strcpy(error_str, "\nCan't Open File ");
        strcat(error_str, arch_name);
        perror(error_str);
        exit(EXIT_FAILURE);
    }
    
    return read_fd;
}

int append_archive(char *arch_name){
    // for error string concatenation
    char error_str[50];
    
    // flags arguments
    int openFlags = O_CREAT | O_RDWR | O_APPEND;

    // perm mode 666 rw-rw-rw-
    // from Listing 4-1
    mode_t filePerms = S_IRUSR | S_IWUSR | S_IRGRP | 
                        S_IWGRP | S_IROTH | S_IWOTH;

    // open archive file for reading
    int append_fd = open(arch_name, openFlags, filePerms);    

    if (append_fd == -1){
        strcpy(error_str, "\nCan't Open File ");
        strcat(error_str, arch_name);
        perror(error_str);
        exit(EXIT_FAILURE);
    }

    return append_fd;
}

int create_archive(char *arch_name, mode_t old_mode){
    
    
    // for error string concatenation
    char error_str[50];
    
    // flags arguments
    int openFlags = O_CREAT | O_RDWR;

    // perm mode 
    // from Listing 4-1
    mode_t filePerms = old_mode;

    // open archive file for reading
    int fd = open(arch_name, openFlags, filePerms);    

    if (fd == -1){
        strcpy(error_str, "\nCan't Open File ");
        strcat(error_str, arch_name);
        perror(error_str);
        exit(EXIT_FAILURE);
    }
    
    // write ARMAG header in the archive if it's not written
    write_archive_header(fd, arch_name);


    return fd;
}

void write_archive_header(int fd, char *arch_name){
   
    int num_written = write(fd, ARMAG, SARMAG);
    
    if(num_written != SARMAG || num_written == -1){
			perror("Error writing file");
			unlink(arch_name);
			exit(EXIT_FAILURE);
	}
}


 void write_file_header(int archive_fd, struct ar_hdr *file_head){
    
    int num_written = write(archive_fd, file_head, sizeof(*file_head));
    
    if(num_written != sizeof(*file_head) || num_written == -1){
			perror("Error writing in arhgive file");
			close(archive_fd);
			exit(EXIT_FAILURE);
	}

 }
 
 void write_file_content(int archive_fd, int cur_file_fd, struct ar_hdr *file_head){
    
    int num_read = -1;
    int num_written = -1;
    
    // find file_size
    long int file_size = atol(file_head->ar_size);

    // create buffer to read content of the file
    char file_buffer[file_size];
    
    // read to buffer
    if ((num_read = read(cur_file_fd, file_buffer, file_size)) == -1)
        perror("read error");
    
    // write from buffer to archive
    num_written = write(archive_fd, file_buffer, sizeof(file_buffer));    
    if(num_written != sizeof(file_buffer) || num_written == -1){
			perror("Error writing in archive file");
			close(archive_fd);
            close(cur_file_fd);
			exit(EXIT_FAILURE);
	}
    
    // add '\n' char after content if file_size is odd 
    char LF = '\n';
    if (file_size % 2 != 0)
    {
        num_written = write(archive_fd, &LF, sizeof(LF));
        if(num_written != sizeof(LF) || num_written == -1){
			perror("Error writing in archive file");
			close(archive_fd);
			exit(EXIT_FAILURE);
        }   
    }
   
 }


void write_file(int archive_fd, int cur_file_fd, char* file_to_add){
    // structs
    struct ar_hdr file_head;
    struct stat sb;   
    
    // create ar_name of the format name/
    char file_name[sizeof(file_head.ar_name)];   
    // copy without null char
    memcpy(file_name, file_to_add, strlen(file_to_add));    
    
    //printf("\nfile_name: %s", file_name);
    
    //strncpy(file_name, file_to_add, sizeof(file_name));    
    file_name[strlen(file_to_add)] = '/';
    
    
    // initialize the rest to 'space'
    for (int i = strlen(file_to_add)+1; i < sizeof(file_name); i++)
        file_name[i] = ' ';
        
    //printf("\nfile_name: %s\n", file_name);
    
    // create buffer and initialize to spaces
    char buffer[sizeof(file_head)];    
    for (int i = 0; i < sizeof(buffer); i++)
        buffer[i] = ' ';
    
   
    //printf("file_name: %s   size:%ld    ", file_name, sizeof(file_to_add));
    
    // read stat info of the file
    if(stat(file_to_add, &sb) == -1)
    {
        perror("stat");
        exit(EXIT_FAILURE);
    }
   
    // !!!--------------------- !!! is regular file !!! ---------------------!!!
    //Being the well known entity files are, files are also called "regular files" 
    //to distinguish them from "special files". They show up in ls -l without 
    //a specific character in the mode field.
    // !!!Source: http://en.wikipedia.org/wiki/Unix_file_types#Regular_file
    
    //File tests each argument in an attempt to classify it.  There are three
    //sets  of tests, performed in this order: filesystem tests, magic number
    //tests, and language tests.
    //The filesystem tests are based on examining the return from  a  stat(2)
    //system  call.   The  program  checks to see if the file is empty, or if
    //it's some sort of special file.	Any known file	types  appropriate  to
    //the  system you are running on (sockets, symbolic links, or named pipes
    //(FIFOs) on those systems that implement them) are intuited if they  are
    //defined in the system header file <sys/stat.h>.
    // Source: http://unixhelp.ed.ac.uk/CGI/man-cgi?file
    
   // source tlcp, page 281
   if(!S_ISREG(sb.st_mode)){
        printf("File: ");
        printf(file_to_add);
        printf(" file is not regular and not archived!\n");
        return;
    }
       
    // src: http://stackoverflow.com/questions/19695798/
    // c-unix-write-function-inserting-strange-charachters-instead-of-spaces
    // https://piazza.com/class/hwqqs6ygf434lx?cid=149
    snprintf((char *)&(file_head), sizeof(file_head), "%-*.*s%-*ld%-*ld%-*ld%-*o%-*ld", 
            (int)sizeof(file_name), (int)sizeof(file_name), file_name,
            (int)sizeof(file_head.ar_date), (long)sb.st_mtime,
            (int)sizeof(file_head.ar_uid), (long)sb.st_uid,
            (int)sizeof(file_head.ar_gid), (long)sb.st_gid,
            (int)sizeof(file_head.ar_mode), sb.st_mode,
            (int)sizeof(file_head.ar_size), (long)sb.st_size); 
    // copy the last 2 bytes         
    memcpy(file_head.ar_fmag, ARFMAG, 2);
    //printf("%s", (char *)&(file_head));
    
    // write file_header in the archive
    write_file_header(archive_fd, &file_head);
    
    // write file content in the archive
    write_file_content(archive_fd, cur_file_fd, &file_head);
}


void x_file(int fd, struct ar_hdr *file_head, off_t curr, char* copied_file){
        
        ssize_t num_read, num_written;
        
        //printf("!!!yay!!!\n");
        
        // create file for write and read
        int cur_file_fd = create_file(copied_file);
        
        // find file_size
        long int file_size = atol(file_head->ar_size);
        
        // create buffer to read content of file
        char file_buffer[file_size];
        
        //set at the last the file content in archive
        lseek(fd, curr, SEEK_SET);
        
        // read in to buffer
        if ((num_read = read(fd, file_buffer, file_size)) == -1)
            perror("read error");
        
        // write to the file
        num_written = write(cur_file_fd, file_buffer, file_size);
        
        if(num_written != file_size || num_written == -1){
			unlink(copied_file);
            perror(copied_file);
			exit(EXIT_FAILURE);
        }
        
        // reset back pointer
        lseek(fd, curr, SEEK_SET);
        printf("%s is extracted\n", copied_file); 
    // copy file content
   
}


void restore_date(struct ar_hdr *file_head, char *tested_file){

    struct utimbuf utb;
    //int buf_size = 10;
    // print date data
    //char time_string[buf_size];
    time_t old_time = atoll(file_head->ar_date); 
    
    // from the class book, p. 288
    utb.actime = old_time; 
    utb.modtime = old_time;
    if (utime(tested_file, &utb) == -1)
        perror("utime");
     
}

void restore_perm(struct ar_hdr *file_head, char *tested_file){
   
   mode_t old_mode = strtol(file_head->ar_mode, NULL, 8); 

    if (chmod(tested_file, old_mode) == -1)
        perror("chmod");
     
}

int is_file_exist(int fd, char* tested_file){
    
    struct ar_hdr file_head;
    int isFound = 0;
    ssize_t num_read;
    off_t curr;
    
    
    // check that archive has correct header
    if (!format_check(fd)){
        printf("Error: The archive file is not of a proper format\n");
        exit(EXIT_FAILURE);
    }
    
    // read a 1st header
    if ((num_read = read(fd, &file_head, sizeof(file_head))) == -1)
        perror("read error");
    
    // compare 1st header filename
    if (strcmp(get_name(&file_head), tested_file) == 0){       
        // get current position
        curr = lseek(fd, 0, SEEK_CUR);
        x_file(fd, &file_head, curr, tested_file);
        restore_perm(&file_head, tested_file);
        restore_date(&file_head, tested_file);
        isFound = 1;
        return isFound;
    }

       
    while(read_archive(fd, &file_head) == sizeof(file_head)){   
        //printf("%s\n",get_name(&file_head));
        // compare 1st header filename
        if (strcmp(get_name(&file_head), tested_file) == 0){
            //print_name(&file_head);
            // get current position
            curr = lseek(fd, 0, SEEK_CUR);
            x_file(fd, &file_head, curr, tested_file);
            restore_perm(&file_head, tested_file);
            restore_date(&file_head, tested_file);
            isFound = 1;
            return isFound;
            
        }
    }
    
    if (isFound == 0)
    {
        printf("File %s is not found\n", tested_file);
    }
    
    return isFound;
}


void extract_all(int fd){
    struct ar_hdr file_head;
   
    ssize_t num_read;
    off_t curr;
    char *file_name = malloc(sizeof(file_head.ar_name));
    
    // check that archive has correct header
    if (!format_check(fd)){
        printf("Error: The archive file is not of a proper format\n");
        exit(EXIT_FAILURE);
    }
    
    // read a 1st header
    if ((num_read = read(fd, &file_head, sizeof(file_head))) == -1)
        perror("read error");
    
    // extract 1st header filename
    file_name = get_name(&file_head);
      
    // get current position
    curr = lseek(fd, 0, SEEK_CUR);
    x_file(fd, &file_head, curr, file_name);
    restore_perm(&file_head, file_name);
    restore_date(&file_head, file_name);
    
  
    while(read_archive(fd, &file_head) == sizeof(file_head)){   
        // extract the rest
        file_name = get_name(&file_head);      
        // get current position
        curr = lseek(fd, 0, SEEK_CUR);
        x_file(fd, &file_head, curr, file_name);
        restore_perm(&file_head, file_name);
        restore_date(&file_head, file_name);   
    }
    
}



int create_file(char *arch_name){
    // for error string concatenation
    char error_str[50];
    
    // flags arguments
    int openFlags = O_CREAT | O_RDWR;

    // perm mode 666 rw-rw-rw-
    // from Listing 4-1
    mode_t filePerms = S_IRUSR | S_IWUSR | S_IRGRP | 
                        S_IWGRP | S_IROTH | S_IWOTH;

    // open archive file for reading
    int fd = open(arch_name, openFlags, filePerms);    

    if (fd == -1){
        strcpy(error_str, "\nCan't Open File ");
        strcat(error_str, arch_name);
        perror(error_str);
        exit(EXIT_FAILURE);
    }

    return fd;
}


void copy_file(int old_fd, int new_archive_fd, struct ar_hdr *file_head, off_t curr){
        
        ssize_t num_read, num_written;
		char LF = '\n';
		
		// find file_size
        long int file_size = atol(file_head->ar_size);
        
		// create buffer to read content of file
        char file_buffer[file_size];
     
        //printf("file_head:%s ", (char*)file_head);	
        //printf("LastChar:%d\n", file_head->ar_fmag[1]);

        // write file_header in the archive
        write_file_header(new_archive_fd, file_head);

        //set at the point of the file content in archive
        //lseek(old_fd, curr, SEEK_SET);

        // read in to buffer
        if ((num_read = read(old_fd, file_buffer, file_size)) == -1)
            perror("read error");
        //printf("%s", file_buffer);
         
        
        // write to the file
        num_written = write(new_archive_fd, file_buffer, file_size);       
        if(num_written != file_size || num_written == -1){
			//unlink(file_to_copy);
            perror("write content error");
			exit(EXIT_FAILURE);
        }
       

		// add '\n' char after content if file_size is odd 
		if (file_size % 2 != 0)
		{
			num_written = write(new_archive_fd, &LF, sizeof(LF));
			if(num_written != sizeof(LF) || num_written == -1){
				perror("Error writing in a new archive file");
				close(new_archive_fd);
				exit(EXIT_FAILURE);
			}

			lseek(old_fd, 1, SEEK_CUR);
		}    
			
		//write_file(archive_fd, cur_file_fd, argv[index]);
        // write file content in the archive
        //write_file_content(new_archive_fd, old_fd, file_head);
        
        // reset back pointer
        //lseek(fd, curr, SEEK_SET);
        //printf("%s is extracted\n", file_to_copy); 
    // copy file content
}


int del_search(int fd, int new_archive_fd, char** argv, int argc, int optind){
    
    struct ar_hdr file_head;
    // flag is set 0 is the file in the deletion list
	int is_copy = 1;
	long int file_size;
    ssize_t num_read;
    off_t curr;
    
    // check that archive has correct header
    if (!format_check(fd)){
        printf("Error: The archive file is not of a proper format\n");
        exit(EXIT_FAILURE);
    }
    
    // read a 1st header
    if ((num_read = read(fd, &file_head, sizeof(file_head))) == -1)
        perror("read error");
    
    // compare 1st header filename
    for (int index = optind; index < argc; index++){
		//printf("argv[index]: %s\n", argv[index]);
		is_copy = strcmp(get_name(&file_head), argv[index]);
		//printf("is_copy = %d\n", is_copy);
		if (is_copy == 0)
		{
			printf("%s is deleted\n", get_name(&file_head));
			break;
		}
			
	}
	
	if (is_copy != 0){       
		//printf("%s is copied\n", get_name(&file_head));
		// get current position
		curr = lseek(fd, 0, SEEK_CUR);
		copy_file(fd, new_archive_fd, &file_head, curr);
		//write_file(new_archive_fd, fd, tested_file);
		
		//return isFound;
	}

	else{
		file_size = atol(file_head.ar_size);
		lseek(fd, file_size, SEEK_CUR);
		
		if (file_size % 2 != 0)
			curr = lseek(fd, 1, SEEK_CUR);
	}

	
    //printf("curr:%ld\n", curr);
	//num_read = read(fd, &file_head, sizeof(file_head));
	//printf("file_head:%s", (char*)&file_head);
	//printf("LastChar:%d\n", file_head.ar_fmag[1]);
	
	//copy_file(fd, new_archive_fd, &file_head, curr);
	
    // compare next headers  
    while(read(fd, &file_head, sizeof(file_head)) == sizeof(file_head)){   
        //printf("%s\n", get_name(&file_head));       
		is_copy = 1;
		// compare other headers filename
		for (int index = optind; index < argc; index++){
			//printf("argv[index]: %s\n", argv[index]);
			is_copy = strcmp(get_name(&file_head), argv[index]);
			//printf("is_copy = %d\n", is_copy);
			if (is_copy == 0)
			{
				printf("%s is deleted\n", get_name(&file_head));
				break;
			}
		}
		
		if (is_copy != 0){       
			//printf("%s is copied\n", get_name(&file_head));
			// get current position
			curr = lseek(fd, 0, SEEK_CUR);
			copy_file(fd, new_archive_fd, &file_head, curr);
			//write_file(new_archive_fd, fd, tested_file);
			
			//return isFound;
		}

		else{
			file_size = atol(file_head.ar_size);
			lseek(fd, file_size, SEEK_CUR);
			
			if (file_size % 2 != 0)
				curr = lseek(fd, 1, SEEK_CUR);
		}
		
		//printf("file_head:%s\n", (char*)&file_head);
    }
    
    if (is_copy == 0)
    {
        //printf("File %s is not found\n", tested_file);
    }
    
    return is_copy;
}


int main (int argc, char *argv[]) {
    
    
    //char c;
    //struct utsname uname_pointer; 
    //time_t time_raw_format;
    //struct stat s;
    
    // Parse arguments with getopt
    /*
    int q_flag = 0,
        x_flag = 0,
        t_flag = 0,
        v_flag = 0,
        d_flag = 0,
        A_flag = 0,
        w_flag = 0;
    */
    
    // define and assign vars
    char *q_value = NULL,
        *x_value = NULL,
        *t_value = NULL,
        *v_value = NULL,
        *d_value = NULL,
        *A_value = NULL,
        *w_value = NULL;       
    
    int option = 0;
    int loop_cnt = 0;
    
    // unset default standard err stream
    // and replace it with custom error
    // handling below(case: '?')
    opterr = 0;
    
    // traverse the command line string
    // until getopt returns -1
    while ((option = getopt(argc, argv, "q:x:t:v:d:A:w:")) != -1){
        loop_cnt++;
        if (loop_cnt > 1)
        {
             printf("\nOnly one key is permitted!\nUsage: myar -key archive-file [member[...]]\n");
             exit(EXIT_FAILURE);
        }
        
        switch (option){
            case 'q':
                q_value = optarg;
                break;
            case 'x':
                x_value = optarg;
                break;
            case 't':
                t_value = optarg;
                break;                
            case 'v':
                v_value = optarg;
                break;
            case 'd':
                d_value = optarg;
                break;
            case 'A':
                A_value = optarg;
                break;
            case 'w':
                w_value = optarg;
                break;
            // getopt returns '?' to indicate an error
            case '?':
                // optopt stores unknown option 
                // or option that has missing argument
                if (optopt == 'q' || optopt == 'x' || optopt == 't' ||
                        optopt == 'v' || optopt == 'd'|| optopt == 'A' ||
                        optopt == 'w')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint(optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr, "Unknown option character `\\x%x'.\n",
                        optopt);
                return 1;
            default:
                exit(EXIT_FAILURE);
        }//switch
    }//while
    
    // check if no keys were provided
    if (loop_cnt == 0)
    {
         printf("No keys provided!\nUsage: myar -key archive-file [member[...]]\n");
         exit(EXIT_FAILURE);
    }
    
    // print options and arguments
    //printf("q_value = %s\n x_value = %s\n t_value = %s\n v_value = %s\n d_value = %s\n A_value = %s\n w_value = %s\n\n", 
    //    q_value, x_value, t_value, v_value, d_value, A_value, w_value);
    
    // optind saves the position of next char to be processed
    // and after options and args with options has been processed, 
    // it points on the remainig non-option arguments in argv
    //for (int index = optind; index < argc; index++)
    //    printf ("file #%d is %s\n", index-optind+1, argv[index]);
    
    
    /*
        ar r myfile.a 'ls *.o'
        Replace myfile.a with object files in the current directory.
    */
    
    
    //************************** -t option **************************//    
    if (t_value != NULL){
        
       struct ar_hdr file_head;
       int read_fd = open_archive(t_value);

       // read archive
       t_archive(read_fd, &file_head);
              
        if(close(read_fd) != 0){
            perror("Close Failed");
        }       
   }//if
   
   
   //************************** -v option **************************//    
    else if (v_value != NULL){
        
        struct ar_hdr file_head;
        int read_fd = open_archive(v_value);

        // read archive
        v_archive(read_fd, &file_head);
              
        if(close(read_fd) != 0){
            perror("Close Failed");
        }       
   }//if
   
   
   
   //************************** -q option **************************//    
    else if (q_value != NULL){
        
        char *file_to_add;
        int cur_file_fd;
        
        // open or create archive file with a header
        int archive_fd = append_archive(q_value);
        
        //printf("\noptind = %d   argc = %d\n", optind, argc);
        
        // if no arguments provided to q-option:
        if(optind >= argc){
            write_archive_header(archive_fd, q_value);
        }
        
        for (int index = optind; index < argc; index++){
            // rename processed file
            file_to_add = argv[index];
            //printf ("file #%d is %s\n", index-optind+1, file_to_add);
            
            // write ARMAG header in the archive if it's not written
            if ((index == optind) && !(format_check(archive_fd))){
                 write_archive_header(archive_fd, q_value);
                 
            }
            
            // open file being archived for reading
            cur_file_fd = open_archive(argv[index]);
            
            write_file(archive_fd, cur_file_fd, argv[index]);
            
            // close file being archived
            if(close(cur_file_fd) != 0){
                perror("Close Failed");
            }             
        }
        
        if(close(archive_fd) != 0){
            perror("Close Failed");
        }
        
    }// else if
    
    
    
    //************************** -x option **************************//    
    else if (x_value != NULL){
       
        // open archive file
        int archive_fd = open_archive(x_value);
        
        // if no arguments provided to x-option:
        if(optind == argc){
            // extract all the files
            //write_archive_header(archive_fd, q_value);
            extract_all(archive_fd);
        }
        
        else
        {
            for (int index = optind; index < argc; index++){
                //printf("filefind: %s   archive_fd: %d\n", argv[index], archive_fd);
                is_file_exist(archive_fd, argv[index]);
            
            }//for
        }
		
		if(close(archive_fd) != 0){
			perror("Close Failed");
		}
       
    }//else if
    
    
    
    //************************** -d option **************************//    
    else if (d_value != NULL){
       
        // open archive file
        int archive_fd = open_archive(d_value);
        
        // get permissions
        // the book p. 303
        struct stat sb;
        mode_t old_mode;
        
        if (stat(d_value, &sb) == -1)
            perror("stat");
        
        old_mode = sb.st_mode;

        // unlink the file
        unlink(d_value);
        
        // open or create new archive file with the same mode
        int new_archive_fd = create_archive(d_value, old_mode);
              
        // if no arguments provided to d-option:
        if(optind == argc){
            // no action
            printf("No files specified for deletion. No action performed\n");
            exit(EXIT_SUCCESS);
        }
        
        else
        {
            
            //printf("filefind: %s   archive_fd: %d\n", argv[index], archive_fd);
                
            del_search(archive_fd, new_archive_fd, argv, argc, optind);

            if(close(archive_fd) != 0){
                perror("Close Failed");
            }
            
            if(close(new_archive_fd) != 0){
                perror("Close Failed");
            }
        }//else
        
        
    }//else if
	
	
	//************************** -A option **************************//    
	else if (A_value != NULL){
        
		//char *file_to_add;
        int cur_file_fd;
		
		// if no arguments provided to A-option:
        if(optind < argc){
            printf("-A option ignores all parameters if there is any!\n");
        }
		
		
		// open or create archive file with a header
        int archive_fd = append_archive(A_value);
 
		//file_to_add = argv[index];
		//printf ("file #%d is %s\n", index-optind+1, file_to_add);
		
		// write ARMAG header in the archive if it's not written
		if (!(format_check(archive_fd))){
			 write_archive_header(archive_fd, A_value);
			 
		}
	
		// get array of all the regular files, besides archive itself
		// source: http://stackoverflow.com/questions/612097/
		// how-can-i-get-a-list-of-files-in-a-directory-using-c-or-c
		DIR *dir;
		struct dirent *ent;
		if ((dir = opendir (".")) != NULL) {
			/* print all the files and directories within directory */
			while ((ent = readdir (dir)) != NULL) {
				
				if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..") &&
					strcmp(ent->d_name, A_value))
				{
					printf ("%s is archived\n", ent->d_name);
					// open file being archived for reading
					cur_file_fd = open_archive(ent->d_name);
					
					write_file(archive_fd, cur_file_fd, ent->d_name);
					
					// close file being archived
					if(close(cur_file_fd) != 0){
						perror("Close Failed");
					}      
				}//if

			}// while
			
			if(close(archive_fd) != 0){
				perror("Close Failed");
			}
			
			closedir (dir);
		} 
		
		else {
			/* could not open directory */
			perror ("");
			return EXIT_FAILURE;
		}
        
    }// else if
	
	
	
	//************************** -w option **************************//    
	else if (w_value != NULL){
        
		//char *file_to_add;
        int cur_file_fd;
		
		// if no arguments provided to A-option:
        if(optind < argc){
            printf("-A option ignores all parameters if there is any!\n");
        }
		
		
		// open or create archive file with a header
        int archive_fd = append_archive(w_value);
 
		//file_to_add = argv[index];
		//printf ("file #%d is %s\n", index-optind+1, file_to_add);
		
		// write ARMAG header in the archive if it's not written
		if (!(format_check(archive_fd))){
			 write_archive_header(archive_fd, w_value);
			 
		}
	
		// get array of all the regular files, besides archive itself
		// source: http://stackoverflow.com/questions/612097/
		// how-can-i-get-a-list-of-files-in-a-directory-using-c-or-c
		DIR *dir;
		struct dirent *ent;
		if ((dir = opendir (".")) != NULL) {
			/* print all the files and directories within directory */
			while ((ent = readdir (dir)) != NULL) {
				// omit . and .. and archive itself
				if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..") &&
					strcmp(ent->d_name, w_value))
				{
					printf ("%s\n", ent->d_name);
					// open file being archived for reading
					cur_file_fd = open_archive(ent->d_name);
					
					write_file(archive_fd, cur_file_fd, ent->d_name);
					
					// close file being archived
					if(close(cur_file_fd) != 0){
						perror("Close Failed");
					}      
				}//if

			}// while
			
			if(close(archive_fd) != 0){
				perror("Close Failed");
			}
			
			closedir (dir);
		} 
		
		else {
			/* could not open directory */
			perror ("");
			return EXIT_FAILURE;
		}
        
    }// else if
    

	return 0;
}
