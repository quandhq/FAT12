#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#pragma pack	(1)

struct Entry    /* struct containing all fields of an entry */
{
    uint8_t filename[8];
    uint8_t filename_extention[3];
    uint8_t file_attribute;
    uint8_t reserved[10];
    uint8_t timecreated_or_lastupdated[2];
    uint8_t datecreated_or_lastupdated[2];
    uint16_t starting_cluster_number;
    uint32_t file_size_inbytes;
};

struct Node
{
    uint8_t state;                              /* state = 1 meaning that these are dirs in directory sector, 0 meaning that these are dirs in data sector  */
	uint32_t cluster_start;                     /* the cluster that the all directories start at */
    struct Entry entry_buffer[32];              /* store all entries */
    struct Entry main_entry_buffer[32];         /* store main entries only */
    uint32_t number_of_main_entry;              /* number of entries */
    uint32_t number_of_entry;                   /* number of main entry */
    struct Node* Next;                          /* Next node */
};
typedef struct Node NodeType;

void addNode(uint8_t state , uint32_t cluster_start, struct Entry entry_buffer[32], uint32_t number_of_entry, struct Entry main_entry_buffer[32], uint32_t number_of_main_entry);
void deleteLastNode();

struct Node* getLastNode();

#endif /* _LINKED_LIST_H_ */
