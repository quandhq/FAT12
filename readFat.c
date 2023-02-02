#include "readFat.h"
#include "linkedList.h"

/*
 * brief: read all parts of bootblock into corresponding field of a structure  
 * params: struct Bootblock* -> a pointer to a struct Bootblock
 *         FILE* -> file pointer to a currently reading file
 */
void readBootblock(struct Bootblock* Bootblock_ptr, uint8_t* fileName)
{
    FILE* filePtr = fopen(fileName, "rb");
    fseek(filePtr, 0, SEEK_SET);
    fread(Bootblock_ptr, 1, BOOTBLOCK_SIZE, filePtr);
    fclose(filePtr);
}

/*
 * brief: calculate the block where each of four sectors starts 
 * params: struct StartPosition* -> pointer to a struct StartPosition
 *         const struct Bootblock* -> pointer to a struct Bootblock
 */
void calculateStartPosition(struct StartPosition* SP_ptr, const struct Bootblock* BB_ptr)
{
    /* bootblock starts at position 0 and always occupys 1 block */
    SP_ptr->bootblock = 0;
    /* File Allocation Table */
    SP_ptr->FAT = 1;
    /* Disk root directory */
    /* number of block before FAT = (size of FAT)*(number of FATs) + 1 */
    SP_ptr->disk_rootdirectory = (BB_ptr->blocks_occupied_by_one_FAT)*(BB_ptr->number_of_FATs) + 1;
    /* File data area */
    /* block occupied by disk_rootdirectory = bytes_per_entry * number_of_entry / bytes_per_block */
    SP_ptr->file_data_area = BYTE_PER_ENTRY*BB_ptr->number_of_rootdirectory_entries/BB_ptr->bytes_per_block + SP_ptr->disk_rootdirectory;
}

/*
 * brief: read an entry into a structure containing all fields of a entry 
 * params: const uint32_t cluster -> byte position at which the entry starts
 */
void readEntry(struct Entry* E_ptr, FILE* filePtr, const struct Bootblock* BB_ptr, const struct StartPosition* SP_ptr, const uint32_t cluster)
{
    printf("seeking ....... %d\n", BB_ptr->bytes_per_block*SP_ptr->disk_rootdirectory);
    /* TESTING READING ENTRY SAMPLE.TXT */
    // fseek(filePtr, (BB_ptr->bytes_per_block*SP_ptr->disk_rootdirectory)+BYTE_PER_ENTRY*3, SEEK_SET);
    fseek(filePtr, cluster, SEEK_SET);
    fread(E_ptr, 1, BYTE_PER_ENTRY, filePtr);
    printf("File name: ............... %s\n", E_ptr->filename);
    if((E_ptr->filename)[0] == 0 && (E_ptr->filename)[1] == 0)
    {
        printf("NOFILE>>>>>>>\n");
    }
}

/*
 * brief: check if a entry is a a dir or a file 
 * params: const struct Entry* 
 * ouput: enum EntryState -> FileAttributeState_Dir if entry is DIR
 *                        -> FileAttributeState_File if entry is FILE
 *                        -> FileAttributeState_ExtraDir if entry is Extra Dir
 */
enum FileAttributeState checkFileAttribute(const struct Entry* E_ptr)
{
    /* get the file_attribute_subdirectory_bit to check if it is set or not */
    if((E_ptr->file_attribute & FileAttributeState_ExtraDir) != 0)
    {
        return FileAttributeState_ExtraDir;
    }
    else if((E_ptr->file_attribute & FileAttributeState_Dir) != 0)
    {
        return FileAttributeState_Dir;
    }
    else
    {
        return FileAttributeState_File;
    }
}

/*
 * brief: print all bytes data of cluster (512 bytes) that cluster starter pointing to
 * params: const uint32_t block -> block where the data starts in data area starting from 0 
 *         uint8_t* fileName -> name of the file we are reading 
 */
void printClusterData(const struct Bootblock* BB_ptr, const struct StartPosition* SP_ptr,const uint32_t block, uint8_t* fileName)
{
    FILE* filePtr = fopen(fileName, "rb");
    /* get cluster size in byte */
    uint32_t cluster_size = BB_ptr->bytes_per_block*BB_ptr->blocks_per_cluster;
    /* move cursor to byte at DataSector that cluster starter pointing to */
    fseek(filePtr, gotoDataSectorByte(SP_ptr->file_data_area, block), SEEK_SET);
    /* read one cluster */
    uint8_t buffer[cluster_size+1];
    fgets(buffer, cluster_size+1, filePtr);
    printf("%s", buffer);
    fclose(filePtr);
}

/*
 * brief: print all data of a file
 * params: const struct Entry* E_ptr -> a pointer to an entry that we are printing all data from
 */
void printAllFileData(const struct Bootblock* BB_ptr, const struct StartPosition* SP_ptr, const struct Entry* E_ptr, FILE* filePtr, uint8_t* fileName)
{
    uint8_t buffer[4];  /* 3 bytes that contain 2 entries */
    uint16_t first_entry; /* contain first entry */
    uint16_t second_entry;  /* contain second entry */
    /* print first cluster of data */
    printClusterData(BB_ptr, SP_ptr, E_ptr->starting_cluster_number, fileName);
    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<calculate entry in FAT area>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    /* go to FAT area with the cluster starter*/
    fseek(filePtr, gotoFAT(SP_ptr->FAT, E_ptr->starting_cluster_number), SEEK_SET);
    /* get the first two clusters */
    fgets(buffer, 4, filePtr);
    first_entry = (uint8_t)(buffer[1]&0x0f)*16*16+buffer[0]; /* get the first entry */
    second_entry = (buffer[2]*16+(buffer[1]>>4)); /* get the seccond entry */

    /* if the starting_cluster_number is odd then we have to start from the second entry in the 3 bytes */
    if(E_ptr->starting_cluster_number %2 == 1)
    {
        if(second_entry == 0xFFF)
        {
            /* 0xFFF means that there is no more data */
            return;
        }
        else
        {
            printf("else---------------------------------------\n");
            printClusterData(BB_ptr, SP_ptr, second_entry, fileName);
            buffer[0] = 0;
            buffer[1] = 0;
            buffer[2] = 0;
            fgets(buffer, 4, filePtr);
            first_entry = (uint8_t)(buffer[1]&0x0f)*16*16+buffer[0]; /* get the first entry */
            second_entry = (buffer[2]*16+(buffer[1]>>4)); /* get the seccond entry */
        }
    }
    
    while(1)    /* read data until facing 0xFFF */
    {
        if(first_entry == 0xFFF)
        {
            /* 0xFFF means that there is no more data */
            break;
        }
        else
        {
            printClusterData(BB_ptr, SP_ptr, first_entry, fileName);
        }

        if(second_entry == 0xFFF)
        {
            /* 0xFFF means that there is no more data */
            break;
        }
        else
        {
            printClusterData(BB_ptr, SP_ptr, second_entry, fileName);
        }

        fgets(buffer, 4, filePtr);
        first_entry = (uint8_t)(buffer[1]&0x0f)*16*16+buffer[0]; /* get the first entry */
        second_entry = (buffer[2]*16+(buffer[1]>>4)); /* get the seccond entry */
    }
}

static void getbit(char x,char s[8])
{	
	int i;
	for(i =0;i<8;i++)
	{
		s[i]=(x&(1<<i))?1:0;
	}
}
static int convert(char *s,int i, int j)
{
	int sum=0,pow=1;
	int k;
	for(k=i;k<=j;k++)
	{
		sum +=pow*s[k];
		pow*2;
	}
}
static void printAllDir(uint32_t address_start, uint8_t state, uint32_t number_of_main_entry, uint32_t number_of_entry)
{	
    FILE* filePtr = fopen("floppy.img", "rb");
   	if(filePtr == NULL)
   		{
   			printf("Can't not open this floppy disk!'");	
		}
	fseek(filePtr,address_start,SEEK_SET);
	struct Entry root;
	struct Node node;
    node.number_of_main_entry = number_of_main_entry;
    node.number_of_entry = number_of_entry;
	int i;
	int j=0;
	int loop_entry=0;
	int number_stt;
	char str[32];
	int entry_count=0; // dem so lan vong lap
	//for(i=0;i<BB.number_root_dicrectory/32;i++)
	int x=0;
	int y=0;
	printf("FileName\tTYPE\tDate\t\tTime\tSize\n");
	if(state==1)
	{
	for(i=0;i<node.number_of_main_entry+loop_entry;i++)	// 7 la 7 cai main entry loop_entry la 4 cai sub_main_entry
	{
		j=0;
		fread(str,1,32,filePtr);
		if(str[11] == 0x0F)
		{	
			
			while(str[j+1]!=0x0F)
			{
				printf("%c",str[j+1]);		
				j++;
			}
			j=0;
			while(j<12)
			{
				printf("%c",str[j+14]);
				j++;
			}
			loop_entry ++;
			printf("\t");
		}
		else if (str[11]!=0x0F)
		{	
			if(y-1<loop_entry)
			{
				printf("\n");
				y++;
			}
			fseek(filePtr,address_start+entry_count*32,SEEK_SET);
			fread(&root,1,32,filePtr);
			printf("%d.",++x);
			for(j=0;j<8 && root.filename[j] != ' ';j++)
			{	
				if(root.filename[0]==' ') continue;
				printf("%c",root.filename[j]);
			}
				if(y>loop_entry)
			{
				printf("\t");
			}
			printf("\t");
			for(j=0;j<3;j++)
			{
				printf("%c",root.filename_extention[j]);
			}
			char s[16],s1[8],s2[8];
			getbit(root.datecreated_or_lastupdated[0],s1);
			getbit(root.datecreated_or_lastupdated[1],s2);

			for(j=0;j<8;j++)
			{
				s[j]=s1[j];
				s[j+8]=s2[j];
			}
			// ngay, thang nam
			int ngay=convert(s,0,4);
			int thang=convert(s,5,8);
			int	nam=convert(s,9,15); nam+=1980;
			printf("\t%d/%d/%d",ngay,thang,nam);
			getbit(root.timecreated_or_lastupdated[0],s1);
			getbit(root.timecreated_or_lastupdated[1],s2);
			for(j=0;j<8;j++)
			{
				s[j]=s1[j];
				s[j+8]=s2[j];
			}
				int gio=convert(s,11,15);
				int phut=convert(s,5,10);
				int giay=convert(s,0,4);
			printf("\t%d:%d:%d",gio,phut,giay);
			printf("\t%d",root.file_size_inbytes);
			printf("\n");	
		}
		entry_count = entry_count + 1;
	}
	}
	else
	{	
		for(i=0;i<node.number_of_entry;i++)
		{
			j=0;
			fread(&root,1,32,filePtr);
			printf("%d.",i+1);
			for(j=0;j<8 && root.filename[j] != ' ';j++)
			{	
				if(root.filename[0]==' ') continue;
				printf("%c",root.filename[j]);
			}
            printf("\t");
			for(j=0;j<3;j++)
			{
				printf("%c",root.filename_extention[j]);
			}
			char s[16],s1[8],s2[8];
			getbit(root.datecreated_or_lastupdated[0],s1);
			getbit(root.datecreated_or_lastupdated[1],s2);
			for(j=0;j<8;j++)
			{
				s[j]=s1[j];
				s[j+8]=s2[j];
			}
			// ngay, thang nam
			int ngay=convert(s,0,4);
			int thang=convert(s,5,8);
			int	nam=convert(s,9,15); nam+=1980;
			printf("\t%d/%d/%d",ngay,thang,nam);
			getbit(root.timecreated_or_lastupdated[0],s1);
			getbit(root.timecreated_or_lastupdated[1],s2);
			for(j=0;j<8;j++)
			{
				s[j]=s1[j];
				s[j+8]=s2[j];
			}
				int gio=convert(s,11,15);
				int phut=convert(s,5,10);
                int giay=convert(s,0,4);
                printf("\t%d:%d:%d",gio,phut,giay);
                printf("\t%d",root.file_size_inbytes);
                printf("\n");	
		    }
	    }
        fclose(filePtr);
}     

/*
 * brief: print all entries that current node has 
 */
void printLastNode(const NodeType* currentNode)
{
    printAllDir(currentNode->cluster_start , currentNode->state, currentNode->number_of_main_entry, currentNode->number_of_entry);
}


