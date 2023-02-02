#include "linkedList.h"

struct Node* HEAD = NULL;

void addNode(uint8_t state , uint32_t cluster_start, struct Entry entry_buffer[32], uint32_t number_of_entry, struct Entry main_entry_buffer[32], uint32_t number_of_main_entry)
{
	NodeType *NodeX = HEAD;
	//Allocate for Node
	NodeType * NewNode = (NodeType *)malloc(sizeof(NodeType));
	NewNode->state = state;
	NewNode->cluster_start=cluster_start;
	NewNode->number_of_entry = number_of_entry;
	NewNode->number_of_main_entry = number_of_main_entry; 
	int i;
    for(i=0;i<number_of_entry;i++)
	{
		NewNode->entry_buffer[i]=entry_buffer[i];
	}
    for(i=0;i<number_of_main_entry;i++)
	{
		NewNode->main_entry_buffer[i]=main_entry_buffer[i];
	}
	NewNode->Next=NULL;
	if(HEAD==NULL)
	{
		HEAD = NewNode;
	}
	else
	{
		while(NodeX->Next != NULL)
		{
			NodeX = NodeX->Next;
		}
		NodeX->Next	= NewNode;
	}
}
void deleteLastNode()
{	
	NodeType *NodeX= HEAD;
	if(NodeX->Next==NULL)
	{
		free(NodeX);
		return; 
	}
	while(NodeX->Next->Next != NULL)
	{
		NodeX = NodeX->Next;
	}
	free(NodeX->Next);
	NodeX->Next = NULL;
}
struct Node* getLastNode()
{	
	NodeType *NodeX = HEAD;
	while(NodeX->Next!=NULL)
	{
		NodeX = NodeX->Next;
	}
	return NodeX;
}
