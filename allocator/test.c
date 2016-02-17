/* Include the sbrk function */ 
#include <unistd.h> 
#include <stdio.h> 

void* heap_start_address = 0;
int   heap_len = 0;

#define EXTEND_MEM_BLOCK (2*1024*1024)


typedef struct memchunk_t{
	int len;
	struct memchunk_t *next;
} memchunk;

#define CHUNK_INFO_SIZE (sizeof(memchunk))

memchunk* alloc_head = 0;
memchunk* alloc_tail = 0;

memchunk* free_head = 0;
memchunk* free_tail = 0;


void simple_allocator_init(){
	heap_start_address = sbrk(0);
	heap_len = 0;
}

//extend 2M
void* extend(size_t n){
	printf("extend %ld \n",n);
	sbrk(n);
	memchunk* p = (memchunk*)(heap_start_address + heap_len);	
	p->len = n;
	p->next = 0;
	heap_len += n;

	if(free_tail){
		free_tail->next = p;
		free_tail = p;
	}else{
		//free_head is also NULL
		free_head = free_tail = p;
	}

}

void* simple_alloc(size_t n){
	int bTried = 0;
	memchunk* memnode = 0;
	memchunk* prevnode = 0;
	if(heap_len == 0){
		if(n > (EXTEND_MEM_BLOCK-8)){
			extend(n);
		}else
			extend(EXTEND_MEM_BLOCK);	
	}

	
START_ALLOC: 
	memnode = free_head;
	prevnode = memnode;
	while(memnode!=0){
		if(memnode->len >= (n+CHUNK_INFO_SIZE))	{	
			break;
		}else{
			prevnode = memnode;
			memnode = memnode->next;
		}
	}

	if(memnode == 0){
		printf("allocate fail,need extend memory\n");
		if( (n+CHUNK_INFO_SIZE) >= (EXTEND_MEM_BLOCK)){
			extend(n+CHUNK_INFO_SIZE);
		}else
			extend(EXTEND_MEM_BLOCK);
		if(bTried == 0){
			bTried = 1;
			goto START_ALLOC;	
		}else
			return 0;
	}
	else{
		printf("get node %p,len %d \n",memnode,memnode->len);
		//remove from free list
		if(prevnode)
			prevnode->next = memnode->next;
		if(memnode == free_head){
			free_head = memnode->next;
			if(free_tail == memnode)
				free_tail = 0;		
		}
		
		
		//add to alloc list
		if(alloc_tail){
			alloc_tail->next = memnode;
			alloc_tail = memnode;		
		}else{
			//no alloc_tail, also means no alloc_head
			alloc_head = alloc_tail = memnode;		
		}
	}
		
	
	return memnode+CHUNK_INFO_SIZE;
}

int simple_free(void* p){
	
	return 1;
}


int main(){
	printf("test simple allocator \n");

	simple_allocator_init();
	printf("test simple allocator 1 \n");

	char * test = (char*)simple_alloc(512);
	test[0] = 1;

	printf("test simple allocator 2 \n");
	char * test2 = (char*)simple_alloc(2*1024*1024);
	test2[0] = 1;


	printf("test simple allocator end \n");
	return 1;
}
