#include "app.h"
#include "linkedList.h"

/*
 * brief: main program 
 */
void run(uint8_t* fileName)
{
    struct Entry main_entry_buffer[32]; /* contain all main entry in root directory sector */
    struct Entry entry_buffer[32]; /* contain all entry in root directory sector */
    uint32_t number_of_entry = 0;
    uint32_t number_of_main_entry = 0;
    uint32_t current_cluster;    /* currentCluster that all entries of current node start */
    uint8_t current_state;       /* current state of node */
    const NodeType* current_last_node;
    FILE* filePtr = fopen(fileName, "rb");
    struct Bootblock BB;
    struct StartPosition SP;
    struct Entry E/*  , sub_E */;
    struct Entry sub_E;
    uint32_t position_in_data_area;
    uint8_t user;   /* contain user's choice */
    /* -----------------------READ BOOTBLOCK--------------------------------------- */
    readBootblock(&BB, fileName);
    /* -------------------------CALCULATE STARTING POSITION-------------------------------------- */
    calculateStartPosition(&SP, &BB);
    /* ----------------GET ALL DIRECTORIES IN ROOT DIRECTORY SECTOR AND STORE THEM IN A NODE----------------- */
    /* Go to root directory sector */
    fseek(filePtr, BB.bytes_per_block*SP.disk_rootdirectory, SEEK_SET);
    /* get all entries and classify the into entry-array or main-entry-array */
    fread(&E, 1, BYTE_PER_ENTRY, filePtr);
    while((E.filename)[0] != 0 && (E.filename)[1] != 0)  
    {
        if((checkFileAttribute(&E) == FileAttributeState_Dir) || (checkFileAttribute(&E) == FileAttributeState_File))
        {
            main_entry_buffer[number_of_main_entry++] = E;
        }
        entry_buffer[number_of_entry++] = E;
        fread(&E, 1, BYTE_PER_ENTRY, filePtr);
    }
    current_cluster = BB.bytes_per_block*SP.disk_rootdirectory;
    current_state = 1;    
    /* ------> add this First Node of root directory to list */
    addNode(current_state, current_cluster, entry_buffer, number_of_entry, main_entry_buffer, number_of_main_entry);
    while(1)
    {
        number_of_entry = 0;
        number_of_main_entry = 0;
        /* ------> get the current last node */
        current_last_node = getLastNode();
    /* ---------------PRINT THE LAST NODE OF THE LIST-------------------- */
        printLastNode(current_last_node);  
        printf("Enter 0 to go back\n"); 
        /* --------------------GET INPUT FROM USER------------------------------ */ 
        printf("USER INPUT:");
        scanf("%d", &user);
        /* --------- IF USER CHOOSES ANY NUMBER OTHER THAN ALLOWED ONE ---------- */
        if(user > current_last_node->number_of_main_entry){return;}
        /* -------- IF USER CHOOSE 0 TO GO BACK ----------- */
        else if(user == 0)
        {
            /* --------DELETE THE LAST NODE OF THE LIST--------- */
            deleteLastNode();
            /* -------CHECK IF THERE IS ANY OTHER NODE LEFT------- */
            /* ---->get the current last node */
            current_last_node = getLastNode();
            /* -----------IF THERE IS NO NODE LEFT------------ */
            if(current_last_node == NULL)
            {
                /* ------> end the program */
                return;
            }
            else
            {
                /* ------> continue printing the current node */
                continue;
            }
        }
        /* ---------IF USER CHOOSE AN EXISTING NUMBER----------------- */
        /* -----> get the entry that user choose */
        sub_E = (current_last_node->main_entry_buffer)[user-1];
        /* -------- IF USER CHOOSE A FILE ----------- */
        if(checkFileAttribute(&sub_E) == FileAttributeState_File)
        {
            printAllFileData(&BB, &SP, &sub_E, filePtr, fileName);
            printf("\nEnter 0 to go back\n"); 
            printf("USER INPUT:");
            scanf("%d", &user);
            if(user == 0)
            {
                continue;
            }
            else
            {
                return;
            }
        }
        /* -------- IF USER CHOOSE A DIR ----------- */
        else if(checkFileAttribute(&sub_E) == FileAttributeState_Dir)
        {
            /* --------- PUT ALL NEW DIRS IN THAT DIR IN A NODE, ADD THAT NODE TO THE LIST --------- */
            /* -----> if the current last node is root-directory node then deep dir start at cluster + 32bytes*2 because of '.' and '..'*/
            position_in_data_area = gotoDataSectorByte(SP.file_data_area, sub_E.starting_cluster_number) + 32*2;
            current_cluster = position_in_data_area;
            current_state = 0;
            /* ----> Go to the cluster starter in data area that this entry has */
            fseek(filePtr, position_in_data_area, SEEK_SET);
            /* get all entries and classify them into entry-array or main-entry-array */
            fread(&E, 1, BYTE_PER_ENTRY, filePtr);
            while((E.filename)[0] != 0 && (E.filename)[1] != 0)  
            {
                if((checkFileAttribute(&E) == FileAttributeState_Dir) || (checkFileAttribute(&E) == FileAttributeState_File))
                {
                    main_entry_buffer[number_of_main_entry++] = E;
                }
                entry_buffer[number_of_entry++] = E;
                fread(&E, 1, BYTE_PER_ENTRY, filePtr);
            } 
            /* --------ADD THIS NODE TO THE LIST--------- */
            addNode(current_state, current_cluster, entry_buffer, number_of_entry, main_entry_buffer, number_of_main_entry);
        }
    }
}