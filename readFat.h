#ifndef _READ_FAT_H_
#define _READ_FAT_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "linkedList.h"
#pragma pack(1)
#define BYTE_PER_ENTRY 32U
#define BOOTBLOCK_SIZE 512
#define FILE_ATTRIBUTE_SUBDIRECTORY_MASK (uint8_t)(0x10)
/* x is block that data sector starts at, y is the block you want to goto */
#define gotoDataSectorByte(x,y) (512*(x+y-2))
/* x is block that FAT sector starts at, y is the block you want to goto */
#define gotoFAT(x,y) (512*x + ((y%2==0) ? (3*y/2) : (3*y/2 - 1))) /* go to the first of 3 bytes that contains FAT entry */
#define MAX_ENTRY_BUFFER_SIZE 32

enum FileAttributeState
{   
    FileAttributeState_File = (uint8_t)0x00,
    FileAttributeState_Dir = (uint8_t)0x10,
    FileAttributeState_ExtraDir = (uint8_t)0x0F,
};

struct StartPosition  /* in block */
{
    uint8_t bootblock;
    uint8_t FAT;
    uint8_t disk_rootdirectory;
    uint8_t file_data_area;
};

struct Bootblock            /* structure containing all informations of bootblock */
{
    uint8_t boostrap_program[3];
    uint8_t optional_manufacturer[8];
    uint16_t bytes_per_block;
    uint8_t blocks_per_cluster;     /* cluster = allocation unit */
    uint16_t reserved_blocks;
    uint8_t number_of_FATs;
    uint16_t number_of_rootdirectory_entries;
    uint16_t number_of_blocks_entired_disk_1;
    uint8_t media_desrciptor;
    uint16_t blocks_occupied_by_one_FAT;
    uint16_t blocks_per_track;
    uint16_t number_of_heads;
    uint32_t number_of_hidden_blocks;
    uint32_t number_of_blocks_entired_disk_2;
    uint16_t physical_driver_number;
    uint8_t boot_record_signature;
    uint32_t volume_serial_number;
    uint8_t volume_label[11];
    uint8_t file_system_identifier[8];
    uint8_t remainder[0x1c0];
    uint8_t bootblock_signature[2];
};

// struct Entry    /* struct containing all fields of a entry */
// {
//     uint8_t filename[8];
//     uint8_t filename_extention[3];
//     uint8_t file_attribute;
//     uint8_t reserved[10];
//     uint16_t timecreated_or_lastupdated;
//     uint16_t datecreated_or_lastupdated;
//     uint16_t starting_cluster_number;
//     uint32_t file_size_inbytes;
// };

/*
 * brief: read all parts of bootblock into corresponding field of a structure  
 * params: struct Bootblock* -> a pointer to a struct Bootblock
 *         FILE* -> file pointer to a currently reading file
 */
void readBootblock(struct Bootblock* Bootblock_ptr, uint8_t* fileName);

/*
 * brief: calculate the block where each of four sectors starts 
 * params: struct StartPosition* -> pointer to a struct StartPosition
 *         const struct Bootblock* -> pointer to a struct Bootblock
 */
void calculateStartPosition(struct StartPosition* SP_ptr, const struct Bootblock* BB_ptr);

/*
 * brief: read an entry into a structure containing all fields of a entry 
 * params: const uint32_t cluster -> byte position at which the entry starts
 */
void readEntry(struct Entry* E_ptr, FILE* filePtr, const struct Bootblock* BB_ptr, const struct StartPosition* SP_ptr, const uint32_t cluster);

/*
 * brief: check if a entry is a a dir or a file 
 * params: const struct Entry* 
 * ouput: enum EntryState -> FileAttributeState_Dir if entry is DIR
 *                        -> FileAttributeState_File if entry is FILE
 *                        -> FileAttributeState_ExtraDir if entry is Extra Dir
 */
enum FileAttributeState checkFileAttribute(const struct Entry* E_ptr);

/*
 * brief: print all bytes data of cluster (512 bytes) that cluster starter pointing to
 * params: const uint32_t block -> block where the data starts in data area starting from 0 
 *         uint8_t* fileName -> name of the file we are reading 
 */
void printClusterData(const struct Bootblock* BB_ptr, const struct StartPosition* SP_ptr,const uint32_t block, uint8_t* fileName);

/*
 * brief: print all data of a file
 * params: const struct Entry* E_ptr -> a pointer to an entry that we are printing all data from
 */
void printAllFileData(const struct Bootblock* BB_ptr, const struct StartPosition* SP_ptr, const struct Entry* E_ptr, FILE* filePtr, uint8_t* fileName);

void printLastNode(const NodeType* currentNode);

#endif  /* _READ_FAT_H_ */
