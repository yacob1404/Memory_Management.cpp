#include <iostream>
#include <iomanip>  // for setw()
using namespace std;


/*******************************************
  Author  : Jacob Hanafin
  Program : Memory Manager
  Due Date: tomorrow
  Course  : CSC 300 Data Structures - Spring 2016 - DSU
  Instr   : Krebsbach
********************************************
*/

// GLOBAL DATA STRUCTURES =======================
typedef struct FREE_NODE * FREEPTR;
typedef struct ALLOCNODE * ALLOCPTR;

struct FREE_NODE  // FREE LIST NODES
{
  int start_byte;
  int end_byte;
  int size;

  FREEPTR next;
};
struct ALLOCNODE // ALLOCTADED LIST NODES
{
  int start_byte;
  int end_byte;
  int size;
  int id;

  ALLOCPTR next;
};
// ======   GLOBAL DATA =========================
FREEPTR  freelist = NULL;  // the FREE link list
ALLOCPTR alloclist = NULL; // the ALLOCATED link list
int total_memory_managed = 0; // the amount of memory managed

//======   PROTOTYPES ===========================
//--- test only ---
void dump_freelist(void);
void dump_alloclist(void);
//--- utility ---
void remove_empty_freenodes(void);
void insert_freeblock(FREEPTR fptr);
//--- interface ---
void init_memory_manager(const int amount);
int allocate_memory(const int job, const int amount);
void release_memory(const int job);
int total_free(void);
int total_allocated(void);
int largest_free(void);
int job_allocated(const int job);
void report_memory(void);
void report_jobs(void);


//======= TESTING FUNCTIONS ====================
void dump_freelist(void)
{ FREEPTR tmp = freelist; // temp pointer to list
  cout << "==================" << endl;
  cout << "  FREE LIST DUMP  " << endl;
  cout << "==================" << endl;
  while (tmp !=NULL)
  {
    cout << tmp->start_byte << " : ";
	cout << tmp->end_byte << " : ";
	cout << tmp->size << endl;
	tmp = tmp->next; //move pointer to next node (or NULL)
  }
}
//----------------------
void dump_alloclist(void)
{ ALLOCPTR tmp = alloclist; // temp pointer to list
  cout << "========================" << endl;
  cout << "  ALLOCATED LIST DUMP  " << endl;
  cout << "========================" << endl;
  while (tmp !=NULL)
  {
    cout << tmp->start_byte << " : ";
	cout << tmp->end_byte << " : ";
	cout << tmp->size << " : ";
	cout << tmp->id << endl;
	tmp = tmp->next; //move pointer to next node (or NULL)
  }
}

//======= UTILITY FUNCTIONS ====================
void remove_empty_freenodes(void)
{
  FREEPTR t1 = NULL;
  FREEPTR t2 = NULL;

  if (freelist == NULL)  return; // empty list

  while ((freelist != NULL) && (freelist->size == 0)) // remove front nodes
  {
     t1 = freelist;
	 freelist = freelist->next;
	 delete t1;
  }

  if (freelist == NULL)  return; // empty list

  t1 = freelist;
  // t points to first node and it is not an empty (SIZE =0) node!!
  while (t1!= NULL) //
  {
    if ((t1->next != NULL) && (t1->next->size == 0)) // remove the empty node
    {

      t2 = t1->next;
      t1->next = t2->next;   //*****************************
      delete t2;
    }
    else    // will move to next node or t1 becomes NULL
	   t1 = t1->next;
  }
}

//--------------------------------------------------------
void insert_freeblock(FREEPTR fptr)
{
  if (freelist == NULL)
  {  freelist = fptr;
     return;
  }

  // handle if inserted at front of freelist;
  if (fptr ->start_byte < freelist->start_byte)  // need to insert
  {
    if (fptr -> end_byte +1 == freelist->start_byte) // merge
	{
	  freelist->start_byte = fptr->start_byte;
	  freelist->size = freelist->size + fptr->size;
	  delete fptr;
	  return;
	}
	// insert node at front of list
	fptr->next = freelist;
	freelist = fptr;
	return;
  }

  FREEPTR t = freelist;
  while (t->next != NULL)
  {
    if (t->next->start_byte > fptr->start_byte) // need to insert between nodes
	{
	  if ((t->end_byte+1 == fptr->start_byte) &&
	             (fptr->end_byte+1 == t->next->start_byte)) // merge both ends
	  { //update first node
	    t->end_byte = t->next->end_byte;
		t->size = t->size + t->next->size + fptr->size;
		//delete other two nodes
		fptr->next = t->next;
		t->next = t->next->next; // skip over node
		delete fptr->next; // delete end node
		delete fptr;
		return;
	  }
	  if (t->end_byte+1 == fptr->start_byte) // merge to front block
	  {
	    t->end_byte = fptr->end_byte;
		t->size = t->size + fptr->size;
		delete fptr;
		return;
	  }
	  if (fptr->end_byte+1 == t->next->start_byte) // merge to back block
	  {
	    t->next->start_byte = fptr->start_byte;
		t->next->size = t->next->size + fptr->size;
		delete fptr;
		return;
	  }
	  // no merge insert between
	  fptr->next = t->next;
	  t->next = fptr;
	  return;
	 }
	 t = t->next;
   }//while
   // need to insert or merge at end of list
   if (t->end_byte+1 == fptr->start_byte) // merge
   {
      t->end_byte = fptr->end_byte;
      t->size = t->size + fptr->size;
      delete fptr;
      return;
    }
    //insert at end of list
    t->next = fptr;

    return;
}


//======= INTERFACE FUNCTIONS ==================
void init_memory_manager(const int amount)
{
  total_memory_managed = amount;
  // set up the freelist linked list
  freelist = new FREE_NODE;
  freelist -> size = amount;
  freelist -> start_byte = 0;
  freelist -> end_byte = amount-1;
  freelist -> next = NULL;
  // set up the alloclist linked list
  alloclist = NULL;
}
//----------------------
int allocate_memory(const int job, const int amount)
{
   FREEPTR tmpfree   = NULL; // pointer to move through list
   ALLOCPTR tmpalloc = NULL; // use to point to new alloc node
   ALLOCPTR tmp2 = alloclist; //Basic tmp for this function

   // check if should continue
   if (amount < 1)  // 0 or neg value  - invalid
       return 0;
   if (amount > total_memory_managed)
       return 0;    // more then total managed
   if (largest_free() < amount) // no block big enough
       return 0;

   // there is a node with enough memory!!!

   //create alloc node to hold information

    tmpalloc = new ALLOCNODE;
    tmpalloc->next = NULL;


   // move to first node big enough and use it
   tmpfree = freelist;

   while (tmpfree != NULL)
   {
     if (tmpfree->size >= amount) //found node
     {
       // fill up the new alloc node
	   tmpalloc->end_byte = tmpfree->end_byte;
	   tmpalloc->start_byte = tmpalloc->end_byte - amount + 1;
	   tmpalloc->size = amount;
	   tmpalloc->id = job;

	   //update freenode
	   tmpfree->end_byte = tmpfree->end_byte - amount;
	   tmpfree->size = tmpfree->size - amount;
     } if (alloclist == NULL) {
        alloclist = tmpalloc;
        alloclist->next = NULL;
     } else {
        if (tmpalloc->end_byte < alloclist->start_byte){
            tmpalloc->next = alloclist;
            alloclist = tmpalloc;
        } else {
            tmp2 = alloclist;
            while (tmpalloc->end_byte > tmp2->start_byte){
                tmp2 = tmp2->next;
            }
            tmpalloc->next = tmp2->next;
            tmp2->next = tmpalloc;
        }
     }
     break;  // exit the while loop
    }
    tmpfree = tmpfree->next;
    return amount;
    // check for empty free node
	//remove_empty_freenodes();      /// *** CALL UTILITY !!!

    //------ now inset allocnode into list --------------
	// check if alloclist is empty
	     //if so then set alloclist = tmpalloc
	// else check if insert as first node
	    // if so insert before first node in alloclist
    // else find where to insert in alloclist (After first node)

   //return amount;  // return amount allocated
}
//-----------------------
void release_memory(const int job)
{
  // Free all memory allocated to process
  ALLOCPTR a1 = alloclist;
  ALLOCPTR temp;
  FREEPTR f1;

  if (a1 == NULL) return; // empty list

  // remove all first nodes THIS effects the alloclist pointer !!!!!
  while(a1 != NULL  && a1->id == job) //
  {
    // set up freenode to be inserted
	f1 = new FREE_NODE;
	f1->next = NULL;
	f1->start_byte = a1->start_byte;
	f1->end_byte = a1->end_byte;
	f1->size = a1-> size;

	// INSERT f1 into FREELIST
	insert_freeblock(f1);   // UTILITY FUNCTION !!!
	f1 = NULL;  // just to make sure;

	// clean up alloclist
	alloclist = a1->next;
	delete a1;
	a1 = alloclist;
  }
  // all first nodes have been deleted
  if (alloclist == NULL) return; // empty list

  while (a1->next != NULL)
  {
    while ((a1->next != NULL) && (a1->next->id == job))
    {
      // set up freenode to be inserted
	  f1 = new FREE_NODE;
	  f1->next = NULL;
	  f1->start_byte = a1->next->start_byte;
	  f1->end_byte = a1->next->end_byte;
	  f1->size = a1->next-> size;

      // INSERT f1 into free list
	  insert_freeblock(f1);
	  f1 =NULL;  // just to make sure

	  // clean up alloclist
	  temp = a1->next ;
      a1->next = temp->next;
      delete temp;
    }

    if (a1->next != NULL)
	  a1 = a1->next;
  }

} //release_memory
//----------------------
int total_free(void)
{
    //tmpfree == temporary free pointer
     FREEPTR tmpfree = freelist;
     int totalFree = 0;

     while (tmpfree != NULL){
        totalFree += tmpfree->size;
        tmpfree = tmpfree->next;
     }

  return totalFree; // return amount of free memory
}
//----------------------
int total_allocated(void)
{
    //alloc1 is simply a temporary pointer, nothing special
   ALLOCPTR alloc1 = alloclist;
   int totalAlloc = 0;

    while (alloc1 != NULL){
        totalAlloc += alloc1->size;
        alloc1 = alloc1->next;
    }
//This function did not work, good prototype*/
/*
   while (alloc1->next != NULL){
    if (alloc1->size == 0){
        totalAlloc = alloc1->size;
        alloc1 = alloc1->next;
    } else {
        totalAlloc += alloc1->size;
        alloc1 = alloc1->next;
    }
   }
*/
   return totalAlloc; //return amount of allocated memory
};
//----------------------
int largest_free(void)
{
  if (freelist == NULL) return 0;    int totalFree = 0;

  FREEPTR t = freelist;

  int largest = 0;

  while (t != NULL)
  {
     if (t->size > largest)
        largest = t ->size;
	 t = t->next;
  }

  return largest;
}
//----------------------
int job_allocated(const int job)
{
  ALLOCPTR alloc1 = alloclist;
  int count = 0;

  while (alloc1 != NULL){
    if(job == alloc1->id){
        count += alloc1->size;
    }
    alloc1 = alloc1->next;
  }

  return count; // return amount of allocated memory
}
//----------------------
void report_memory(void)
{
   ALLOCPTR alloc1 = alloclist;
   FREEPTR freealloc1 = freelist;

cout << "===================================" << endl;
cout << "MEMORY BLOCK    JOB" << endl;
   while(freealloc1 != NULL || alloc1 != NULL){
        if(freealloc1 != NULL){
            if(alloc1 != NULL){
                if(freealloc1->start_byte < alloc1->start_byte){
                    cout << setw(3) << freealloc1->start_byte << "  -  " <<
                    setw(3) << freealloc1->end_byte << "    "
                    << "FREE" << endl;
                    freealloc1 = freealloc1->next;
                } else {
                    cout << setw(3) << alloc1->start_byte << "  -  "
                    << setw(3) << alloc1->end_byte << "    "
                    << alloc1->id << endl;
                    alloc1 = alloc1->next;
                }
            } else {
                cout << setw(3) << freealloc1->start_byte << "  -  " <<
                setw(3) << freealloc1->end_byte << "    "
                << "FREE" << endl;
                freealloc1 = freealloc1->next;
            }
        } else if (alloc1 != NULL){
            cout << setw(3) << alloc1->start_byte << "  -  "
            << setw(3) << alloc1->end_byte << "    "
            << alloc1->id << endl;
            alloc1 = alloc1->next;
        }
   }
}
//----------------------
void report_jobs(void)
{
    cout << "===================================" << endl;
    cout << "JOB        Memory Usage" << endl;
    ALLOCPTR tmpalloc = alloclist;
    while(tmpalloc != NULL){
        cout << "   " << tmpalloc->id << "   " << tmpalloc->start_byte << "-" << tmpalloc->end_byte << endl;
        tmpalloc = tmpalloc->next;
    }
}
//==========  MAIN =============================
int main(void)
{
  char ch ;  // used to pause between tests
  int r;     // results of allocate_memory

  // ================================================================================
  cout << "====================================" << endl;
  cout << "  AUTHOR :  Jacob Hanafin " << endl;
  cout << "====================================" << endl;
  cout << endl;
  cout << "ENTER A CHARACTER ";
  cin >>ch;
  cout << endl;
  //=================================================================================
  cout << "====================================" << endl;
  cout << "TEST # 1" << endl;
  cout << "====================================" << endl << endl;
  init_memory_manager(200);

  r = allocate_memory(1,200);
  cout << "allocate_memory returns : " << r << endl << endl; // ALL memory
  r = allocate_memory(2,30);
  cout << "allocate_memory returns : " << r << endl << endl; // over allocate

  release_memory(1);        // free all memory

  r = allocate_memory(1,-1);
  cout << "allocate_memory returns : " << r << endl << endl; // try allocate  -1
  r = allocate_memory(3,0);
  cout << "allocate_memory returns : " << r << endl << endl; // allocate 0
  r = allocate_memory(1,256);
  cout << "allocate_memory returns : " << r << endl << endl;  // over allocate
  r = allocate_memory(1,100);
  cout << "allocate_memory returns : " << r << endl << endl;  //Ok allocate 100

  cout << "total free memory is  : " << total_free() << endl;  // 100
  cout << "total alloc memory is : " << total_allocated() << endl; // 100


  cout << endl;
  cout << "ENTER A CHARACTER ";
  cin >>ch;
  cout << endl;

  // =================================================================================
  cout << "====================================" << endl;
  cout << "TEST # 2 [Deallocate several of same ] " << endl;
  cout << "====================================" << endl << endl;
  init_memory_manager(200);
  r = allocate_memory(1,20);
  cout << "allocate_memory returns : " << r << endl << endl;
  r = allocate_memory(2,30);
  cout << "allocate_memory returns : " << r << endl << endl;
  r = allocate_memory(1,20);
  cout << "allocate_memory returns : " << r << endl << endl;
  r = allocate_memory(3,30);
  cout << "allocate_memory returns : " << r << endl << endl;
  r = allocate_memory(1,20);
  cout << "allocate_memory returns : " << r << endl << endl;

  cout << "total free memory is  : " << total_free() << endl;
  cout << "total alloc memory is : " << total_allocated() << endl;

  release_memory(1);
  //======================
  report_memory();
  report_jobs();

  cout << "total free memory is  : " << total_free() << endl;  // 100
  cout << "total alloc memory is : " << total_allocated() << endl; // 100

  cout << endl;
  cout << "ENTER A CHARACTER ";
  cin >>ch;
  cout << endl;

  // =================================================================================
  cout << "====================================" << endl;
  cout << "TEST # 3  BETWEEN [merge to both blocks]" << endl;
  cout << "====================================" << endl << endl;
  init_memory_manager(200);

  r = allocate_memory(1,25);
  cout << "allocate_memory returns : " << r << endl << endl;
  r = allocate_memory(2,25);
  cout << "allocate_memory returns : " << r << endl << endl;
  r = allocate_memory(3,25);
  cout << "allocate_memory returns : " << r << endl << endl;
  r = allocate_memory(4,25);
  cout << "allocate_memory returns : " << r << endl << endl;
  r = allocate_memory(5,25);
  cout << "allocate_memory returns : " << r << endl << endl;
  //=====================
  cout << "========================" << endl << endl;
  cout << "total free memory is  : " << total_free() << endl;  // 100
  cout << "total alloc memory is : " << total_allocated() << endl; // 100
  release_memory(1);
  release_memory(3);
  release_memory(2);
  //======================
  report_memory();
  report_jobs();

  return 0;
}

