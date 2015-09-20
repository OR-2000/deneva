#ifndef _HELPER_H_
#define _HELPER_H_

#include <cstdlib>
#include <iostream>
#include <stdint.h>
//#ifdef NOGRAPHITE
//#include <ctime>
//#include <ratio>
//#include <chrono>
//#endif
#include "global.h"

/************************************************/
// Debugging
/************************************************/
#define DEBUG(...) \
  if(DEBUG_DISTR) { \
    fprintf(stdout,__VA_ARGS__); \
  }
#define DEBUG_FLUSH() \
  if(DEBUG_DISTR) { \
    fflush(stdout); \
  }


/************************************************/
// atomic operations
/************************************************/
#define ATOM_ADD(dest, value) \
	__sync_fetch_and_add(&(dest), value)
#define ATOM_SUB(dest, value) \
	__sync_fetch_and_sub(&(dest), value)
// returns true if cas is successful
#define ATOM_CAS(dest, oldval, newval) \
	__sync_bool_compare_and_swap(&(dest), oldval, newval)
#define ATOM_ADD_FETCH(dest, value) \
	__sync_add_and_fetch(&(dest), value)
#define ATOM_FETCH_ADD(dest, value) \
	__sync_fetch_and_add(&(dest), value)
#define ATOM_SUB_FETCH(dest, value) \
	__sync_sub_and_fetch(&(dest), value)

/************************************************/
// ASSERT Helper
/************************************************/
#define M_ASSERT(cond, str) \
	if (!(cond)) {\
		printf("ASSERTION FAILURE [%s : %d] msg:%s\n", __FILE__, __LINE__, str);\
		exit(0); \
	}
#define ASSERT(cond) assert(cond)


/************************************************/
// STACK helper (push & pop)
/************************************************/
#define STACK_POP(stack, top) { \
	if (stack == NULL) top = NULL; \
	else {	top = stack; 	stack=stack->next; } }
#define STACK_PUSH(stack, entry) {\
	entry->next = stack; stack = entry; }

/************************************************/
// LIST helper (read from head & write to tail)
/************************************************/
#define LIST_GET_HEAD(lhead, ltail, en) {\
	en = lhead; \
	lhead = lhead->next; \
	if (lhead) lhead->prev = NULL; \
	else ltail = NULL; \
	en->next = NULL; }
#define LIST_PUT_TAIL(lhead, ltail, en) {\
	en->next = NULL; \
	en->prev = NULL; \
	if (ltail) { en->prev = ltail; ltail->next = en; ltail = en; } \
	else { lhead = en; ltail = en; }}
#define LIST_INSERT_BEFORE(entry, newentry,lhead) { \
	newentry->next = entry; \
	newentry->prev = entry->prev; \
	if (entry->prev) entry->prev->next = newentry; \
	entry->prev = newentry; \
  if (lhead == entry) lhead = newentry;}
#define LIST_REMOVE(entry) { \
	if (entry->next) entry->next->prev = entry->prev; \
	if (entry->prev) entry->prev->next = entry->next; }
#define LIST_REMOVE_HT(entry, head, tail) { \
	if (entry->next) entry->next->prev = entry->prev; \
	else { assert(entry == tail); tail = entry->prev; } \
	if (entry->prev) entry->prev->next = entry->next; \
	else { assert(entry == head); head = entry->next; } \
}

/************************************************/
// STATS helper
/************************************************/
#define SET_STATS(tid, name, value) \
	if (STATS_ENABLE) \
		stats._stats[tid]->name = value;

#define INC_STATS(tid, name, value) \
	if (STATS_ENABLE) \
		stats._stats[tid]->name += value;

#define INC_STATS_ARR(tid, name, value) \
	if (STATS_ENABLE) \
		stats._stats[tid]->name.insert(value);

#define INC_TMP_STATS(tid, name, value) \
	if (STATS_ENABLE) \
		stats.tmp_stats[tid]->name += value;

#define INC_GLOB_STATS(name, value) \
	if (STATS_ENABLE) \
		stats.name += value;

/************************************************/
// mem copy helper
/************************************************/
#define COPY_VAL(v,d,p) \
  memcpy(&v,&d[p],sizeof(v)); \
  p += sizeof(v);

#define COPY_VAL_SIZE(v,d,p,s) \
  memcpy(&v,&d[p],s); \
  p += s;

#define COPY_BUF(d,v,p) \
  memcpy(&((char*)d)[p],(char*)&v,sizeof(v)); \
  p += sizeof(v);

#define COPY_BUF_SIZE(d,v,p,s) \
  memcpy(&((char*)d)[p],(char*)&v,s); \
  p += s;
/************************************************/
// malloc helper
/************************************************/
// In order to avoid false sharing, any unshared read/write array residing on the same 
// cache line should be modified to be read only array with pointers to thread local data block.
// TODO. in order to have per-thread malloc, this needs to be modified !!!

#define ARR_PTR_MULTI(type, name, size, scale) \
	name = new type * [size]; \
	if (g_part_alloc || THREAD_ALLOC) { \
		for (UInt32 i = 0; i < size; i ++) {\
			UInt32 padsize = sizeof(type) * (scale); \
			if (g_mem_pad && padsize % CL_SIZE != 0) \
				padsize += CL_SIZE - padsize % CL_SIZE; \
			name[i] = (type *) mem_allocator.alloc(padsize, i); \
			for (UInt32 j = 0; j < scale; j++) \
				new (&name[i][j]) type(); \
		}\
	} else { \
		for (UInt32 i = 0; i < size; i++) \
			name[i] = new type[scale]; \
	}

#define ARR_PTR(type, name, size) \
	ARR_PTR_MULTI(type, name, size, 1)

#define ARR_PTR_INIT(type, name, size, value) \
	name = new type * [size]; \
	if (g_part_alloc) { \
		for (UInt32 i = 0; i < size; i ++) {\
			int padsize = sizeof(type); \
			if (g_mem_pad && padsize % CL_SIZE != 0) \
				padsize += CL_SIZE - padsize % CL_SIZE; \
			name[i] = (type *) mem_allocator.alloc(padsize, i); \
			new (name[i]) type(); \
		}\
	} else \
		for (UInt32 i = 0; i < size; i++) \
			name[i] = new type; \
	for (UInt32 i = 0; i < size; i++) \
		*name[i] = value; \

#define YCSB_QUERY_FREE(qry) \
  ycsb_query * query = (ycsb_query*) qry; \
  mem_allocator.free(query->part_to_access,sizeof(uint64_t)*query->part_num); \
  mem_allocator.free(query->requests,sizeof(ycsb_query)*query->request_cnt); \
  mem_allocator.free(query,sizeof(ycsb_query)); 
enum Data_type {DT_table, DT_page, DT_row };

// TODO currently, only DR_row supported
// data item type. 
class itemid_t {
public:
	itemid_t() { };
	itemid_t(Data_type type, void * loc) {
        this->type = type;
        this->location = loc;
    };
	Data_type type;
	void * location; // points to the table | page | row
	itemid_t * next;
	bool valid;
	void init();
	bool operator==(const itemid_t &other) const;
	bool operator!=(const itemid_t &other) const;
	void operator=(const itemid_t &other);
};

int get_thdid_from_txnid(uint64_t txnid);

// key_to_part() is only for ycsb
uint64_t key_to_part(uint64_t key);
uint64_t get_part_id(void * addr);
// TODO can the following two functions be merged?
uint64_t merge_idx_key(uint64_t key_cnt, uint64_t * keys);
uint64_t merge_idx_key(uint64_t key1, uint64_t key2);
uint64_t merge_idx_key(uint64_t key1, uint64_t key2, uint64_t key3);

void init_client_globals(); 

extern timespec * res;
uint64_t get_server_clock();
uint64_t get_sys_clock(); // return: in ns

class myrand {
public:
	void init(uint64_t seed);
	uint64_t next();
private:
	uint64_t seed;
};

#endif
