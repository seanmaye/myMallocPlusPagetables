//Andrew San and Sean Maye ars400 sam710
//Ilab: less


#include "my_vm.h"
#include <math.h>
#include <stdint.h>

int firstTime = 0;
char* phys_bitmap[PHYS_BITMAP_SIZE]; 
char* virt_bitmap[VIRT_BITMAP_SIZE]; 
int tlb_counter = 0;

char* RAM;
int nextPage;

//struct tlb* tlb_store[TLB_ENTRIES];
struct tlb* tlb[TLB_ENTRIES];


//Main pages directory: its a two-dimensional array pageDirectory[i][j] where i is a pde_t and where j is a pte_t
pte_t **pageDirectory;


/*
Function responsible for allocating and setting your physical memory 
*/
void set_physical_mem() {

    RAM = (char*)malloc(MEMSIZE);
    memset(RAM, 0, MEMSIZE);
    memset(phys_bitmap, 0, PHYS_BITMAP_SIZE);
    memset(virt_bitmap, 0, VIRT_BITMAP_SIZE); 
    //memset(tlb_store,0,TLB_ENTRIES * sizeof(struct tlb));
    int entries = pow(2,10);
    
    pageDirectory = (pte_t **)malloc(entries * sizeof(pte_t *)); 

    for (int i = 0; i < entries; i++) {
        pageDirectory[i] = (pte_t *)malloc(sizeof(pte_t));
    }


    //Allocate physical memory using mmap or malloc; this is the total size of
    //your memory you are simulating
    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps and initialize them
}


/*
 * Part 2: Add a virtual to physical page translation to the TLB.
 * Feel free to extend the function arguments or return type.
 */
int
add_TLB(void *va, void *pa)
{
    if(tlb_counter>=TLB_ENTRIES){
        tlb_counter = 0;
    }
        tlb[tlb_counter]->pa = pa;
        tlb[tlb_counter]->va = va;
        tlb_counter++;   
    
    /*Part 2 HINT: Add a virtual to physical page translation to the TLB */
    return 1;
}


/*
 * Part 2: Check TLB for a valid translation.
 * Returns the physical page address.
 * Feel free to extend this function and change the return type.
 */
pte_t *
check_TLB(void *va) {
    /* Part 2: TLB lookup code here */
    for(int i = 0; i<TLB_ENTRIES; i++){
        if(tlb[i]->va == va) return translate(NULL, va); 
    }


   /*This function should return a pte_t pointer*/
}


/*
 * Part 2: Print TLB miss rate.
 * Feel free to extend the function arguments or return type.
 */
void
print_TLB_missrate()
{
    double miss_rate = 0;	

    /*Part 2 Code here to calculate and print the TLB miss rate*/




    fprintf(stderr, "TLB miss rate %lf \n", miss_rate);
}



/*
The function takes a virtual address and page directories starting address and
performs translation to return the physical address
*/
pte_t *translate(pde_t *pgdir, void *va) {
    /* Part 1 HINT: Get the Page directory index (1st level) Then get the
    * 2nd-level-page table index using the virtual address.  Using the page
    * directory index and page table index get the physical address.
    *
    * Part 2 HINT: Check the TLB before performing the translation. If
    * translation exists, then you can return physical address from the TLB.
    */


    //If translation not successful, then return NULL
    
    unsigned int mask1stlevel = 0xFFC00000;
    unsigned int mask2ndlevel = 0x007FE000;
    unsigned int first10 =((unsigned int)va & mask1stlevel) >> 22;
    unsigned int next10 = ((unsigned int)va & mask2ndlevel) >> 13;
    unsigned int offset = (unsigned int)va&0xFFF;
    //shift bits to get offset (12)
    //use first 10 bits and create pte then use the next 10 bits to get physical address from there we combine the address with the offset 
    pte_t secondLevel = *pgdir +(first10 *sizeof(pde_t));
    unsigned int physicalAddress = secondLevel&mask2ndlevel; //whatever next 10 represents 


    return secondLevel; //anyshift here
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
int
page_map(pde_t *pgdir, void *va, void *pa)
{
    /* HINT: Similar to translate(), find the page directory (1st level)
     * and page table (2nd-level) indices. If no mapping exists, set the
     * virtual to physical mapping.
     */

    //straight from translate()
    unsigned int mask1stlevel = 0xFFC00000;
    unsigned int mask2ndlevel = 0x003FF000;
    unsigned int first10 =((unsigned int)va & mask1stlevel) >> 22;
    unsigned int next10 = ((unsigned int)va & mask2ndlevel) >> 12;
 
    if ((virt_bitmap[first10]) == 1) {
        return 1; //return 1 to tmalloc to show that this VA is already mapped
    }
    else {
        virt_bitmap[first10] = 1;
        pageDirectory[first10][next10] = pa;
        return 0; 
    }
}



/*Function that gets the next available page
*/
void *get_next_avail(int num_pages) {
 
    //Use virtual address bitmap to find the next free pag
    int i=0;
    for(int i=0; i<VIRT_BITMAP_SIZE; i++){
        if(virt_bitmap[i]==0){
        for(int j=i; j<num_pages+j; j++){
            if(virt_bitmap[j]==1){
                break;
            }else if(((num_pages+j)-1 ==j)&& virt_bitmap[j]==0){
                return &virt_bitmap+i;
            }
        }
        }
    }
}


/* Function responsible for allocating pages
and used by the benchmark
*/
void *t_malloc(unsigned int num_bytes) {

    /* 
     * HINT: If the physical memory is not yet initialized, then allocate and initialize.
     */

   /* 
    * HINT: If the page directory is not initialized, then initialize the
    * page directory. Next, using get_next_avail(), check if there are free pages. If
    * free pages are available, set the bitmaps and map a new page. Note, you will 
    * have to mark which physical pages are used. 
    */

    if(firstTime==0){
        set_physical_mem();
        firstTime++;
    }
    int num_pages = num_bytes/PGSIZE + 1;
    unsigned int * page_addr = get_next_avail(num_pages);
    unsigned long virtual_Address = (unsigned int) page_addr*PGSIZE;
    unsigned long physical_Address;
    
    
    for(int i =page_addr; i<page_addr+num_pages; i++){
        virt_bitmap[i]=1;
    }
    
    for (int i = 0; i < num_pages; i++) {
        for (int j = 0; j < PHYS_BITMAP_SIZE; j++) {
            if (phys_bitmap[j]!= 1) {
                phys_bitmap[j]= 1;
                physical_Address = RAM+PGSIZE * j;
                page_map(pageDirectory,virtual_Address,physical_Address);
                memset(physical_Address, 1, PGSIZE);
                break;
            }
        }
        virtual_Address+=sizeof(pte_t);
    }
    //VA was incramented so we have to get the original starting point
    return virtual_Address -(num_pages*sizeof(pte_t));
}


/* Responsible for releasing one or more memory pages using virtual address (va)
*/
void t_free(void *va, int size) {

    /* Part 1: Free the page table entries starting from this virtual address
     * (va). Also mark the pages free in the bitmap. Perform free only if the 
     * memory from "va" to va+size is valid.
     *
     * Part 2: Also, remove the translation from the TLB
     */
    unsigned int mask1stlevel = 0xFFC00000;
    unsigned int mask2ndlevel = 0x003FF000;
    unsigned int first10 =((unsigned int)va & mask1stlevel) >> 22;
    unsigned int next10 = ((unsigned int)va & mask2ndlevel) >> 12;
    pageDirectory[first10][next10]=NULL;
    
    int num_pages = size/PGSIZE + 1;
    for(int i =0; i<num_pages; i++){
         pte_t pa = translate(NULL, va);
         unsigned int offset = (unsigned int)va&0xFFF;
          unsigned int mask2ndlevel = 0x007FE000;
          unsigned int physicalAddress = pa&mask2ndlevel;
          pa=(physicalAddress>>12)||offset;
         int distance =RAM[pa]-RAM[0];
         int phys_index = distance/PGSIZE;
         phys_bitmap[phys_index] = 0;
          memset(pa, 0, PGSIZE);
         //remove from physical and virtual remeber return page table
         va+=sizeof(pte_t);
    }    
}


/* The function copies data pointed by "val" to physical
 * memory pages using virtual address (va)
 * The function returns 0 if the put is successfull and -1 otherwise.
*/
int put_value(void *va, void *val, int size) {

    /* HINT: Using the virtual address and translate(), find the physical page. Copy
     * the contents of "val" to a physical page. NOTE: The "size" value can be larger 
     * than one page. Therefore, you may have to find multiple pages using translate()
     * function.
     */


    /*return -1 if put_value failed and 0 if put is successfull*/
    pte_t * pa = translate(NULL, va);
    memcpy(pa,val,size);
}


/*Given a virtual address, this function copies the contents of the page to val*/
void get_value(void *va, void *val, int size) {

    /* HINT: put the values pointed to by "va" inside the physical memory at given
    * "val" address. Assume you can access "val" directly by derefencing them.
    */

   pte_t * pa = translate(NULL, va);
    memcpy(val,pa,size);

}

/*
This function receives two matrices mat1 and mat2 as an argument with size
argument representing the number of rows and columns. After performing matrix
multiplication, copy the result to answer.
*/
void mat_mult(void *mat1, void *mat2, int size, void *answer) {

    /* Hint: You will index as [i * size + j] where  "i, j" are the indices of the
     * matrix accessed. Similar to the code in test.c, you will use get_value() to
     * load each element and perform multiplication. Take a look at test.c! In addition to 
     * getting the values from two matrices, you will perform multiplication and 
     * store the result to the "answer array"
     */
    int x, y, val_size = sizeof(int);
    int i, j, k;
    for (i = 0; i < size; i++) {
        for(j = 0; j < size; j++) {
            unsigned int a, b, c = 0;
            for (k = 0; k < size; k++) {
                int address_a = (unsigned int)mat1 + ((i * size * sizeof(int))) + (k * sizeof(int));
                int address_b = (unsigned int)mat2 + ((k * size * sizeof(int))) + (j * sizeof(int));
                get_value( (void *)address_a, &a, sizeof(int));
                get_value( (void *)address_b, &b, sizeof(int));
                // printf("Values at the index: %d, %d, %d, %d, %d\n", 
                //     a, b, size, (i * size + k), (k * size + j));
                c += (a * b);
            }
            int address_c = (unsigned int)answer + ((i * size * sizeof(int))) + (j * sizeof(int));
            // printf("This is the c: %d, address: %x!\n", c, address_c);
            put_value((void *)address_c, (void *)&c, sizeof(int));
        }
    }
}



