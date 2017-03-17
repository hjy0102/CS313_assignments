/*
 * cache.c
 */
#include "cache.h"
#include <stdlib.h>
#include <string.h>

/*
 * Initialize a new cache line with a given block size.
 */
static void cache_line_init(cache_line_t *cache_line, size_t block_size)
{
    cache_line->is_valid = 0;
    cache_line->data = (unsigned char *) malloc(block_size * sizeof(unsigned char));
}

/*
 * Initialize a new cache set with the given associativity and block size.
 */
static void cache_set_init(cache_set_t *cache_set, unsigned int associativity, size_t block_size)
{
    int i;
    cache_set->cache_lines = (cache_line_t **) malloc(associativity * sizeof (cache_line_t *));

    for (i = 0; i < associativity; i++)
    {
	cache_set->cache_lines[i] = (cache_line_t *) malloc(sizeof(cache_line_t));
	cache_line_init(cache_set->cache_lines[i], block_size);
    }
}

/*
 * Compute the shift and mask given the number of bytes/sets.
 */
static void get_shift_and_mask(int value, int *shift, int *mask, int init_shift)
{
    *mask = 0;
    *shift = init_shift;

    while (value > 1)
    {
	(*shift)++;
	value >>= 1;
	*mask = (*mask << 1) | 1;
    }
}

/*
 * Create a new cache that contains a total of num_blocks blocks, each of which is block_size
 * bytes long, with the given associativity, and the given set of cache policies for replacement
 * and write operations.
 */
cache_t *cache_new(size_t num_blocks, size_t block_size, unsigned int associativity, int policies)
{
    int i, j;

    /*
     * Create the cache and initialize constant fields.
     */
    cache_t *cache = (cache_t *) malloc(sizeof(cache_t));
    cache->access_count = 0;
    cache->miss_count = 0;

    /*
     * Initialize size fields.
     */
    cache->policies = policies;
    cache->block_size = block_size;
    cache->associativity = associativity;
    cache->num_sets = num_blocks / associativity;

    /*
     * Initialize shifts and masks.
     */
    get_shift_and_mask(block_size, &cache->cache_index_shift, &cache->block_offset_mask, 0);
    get_shift_and_mask(cache->num_sets, &cache->tag_shift, &cache->cache_index_mask, cache->cache_index_shift);

    /*
     * Initialize cache sets.
     */
    cache->sets = (cache_set_t *) malloc(cache->num_sets * sizeof (cache_set_t));
    for (i = 0; i < cache->num_sets; i++)
    {
	cache_set_init(&cache->sets[i], cache->associativity, cache->block_size);
    }

    /*
     * Done.
     */
    return cache;
}

/*
 * Determine whether or not a cache line is valid for a given tag.
 */
static int cache_line_check_validity_and_tag(cache_line_t *cache_line, intptr_t tag)
{
    /* TO BE COMPLETED BY THE STUDENT */
    // DONE
    return cache_line->is_valid && cache_line->tag == tag;
}

/*
 * Return integer data from a cache line.
 */
static int cache_line_retrieve_data(cache_line_t *cache_line, size_t offset)
{
    /* TO BE COMPLETED BY THE STUDENT */
    // DONE
    int i = 0;
        i += (cache_line->data[offset]);
        i += (cache_line->data[offset +1]) << 8;
        i += (cache_line->data[offset +2]) << 16;
        i += (cache_line->data[offset +3]) << 24;
        return i;
}

/*
 * Move the cache lines inside a cache set so the cache line with the given index is
 * tagged as the most recently used one. The most recently used cache line will be the 
 * 0'th one in the set, the second most recently used cache line will be next, etc.
 * Cache lines whose valid bit is 0 will occur after all cache lines whose valid bit
 * is 1.
 */
static cache_line_t *cache_line_make_mru(cache_set_t *cache_set, size_t line_index)
{
    int i;
    cache_line_t *line = cache_set->cache_lines[line_index];

    for (i = line_index - 1; i >= 0; i--)
    {
	cache_set->cache_lines[i + 1] = cache_set->cache_lines[i];
    }

    cache_set->cache_lines[0] = line;
    return line;
}

/*
 * Retrieve a matching cache line from a set, if one exists.
 */
static cache_line_t *cache_set_find_matching_line(cache_t *cache, cache_set_t *cache_set, intptr_t tag)
{
    /* TO BE COMPLETED BY THE STUDENT */

    /*
     * Don't forget to call cache_line_make_mru(cache_set, i) if you find a matching cache line.
     */
    // TODO !! This is wrong

    int cache_set_size = cache->associativity;
    int i;
    cache_line_t * line = NULL;
    for (i = 0; i < cache_set_size; i++){
            line = cache_set->cache_lines[i];
            if(cache_line_check_validity_and_tag(line, tag)){
        return cache_line_make_mru(cache_set, i);
      }
    }
        return NULL;
}

/*
 * Function to find a cache line to use for new data.
 */
static cache_line_t *find_available_cache_line(cache_t *cache, cache_set_t *cache_set)
{
    /* TO BE COMPLETED BY THE STUDENT */

    /*
     * Don't forget to call cache_line_make_mru(cache_set, i) once you have selected the
     * cache line to use.
     */
     int cache_set_size = cache->associativity;
    
    return cache_line_make_mru(cache_set, cache_set_size-1);

}

/*
 * Add a block to a given cache set.
 */
static cache_line_t *cache_set_add(cache_t *cache, cache_set_t *cache_set, intptr_t address, intptr_t tag)
{
    /*
     * First locate the cache line to use.
     */
    cache_line_t *line = find_available_cache_line(cache, cache_set);

    /*
     * Now set it up.
     */

    line->tag = tag;
    line->is_valid = 1;
    memcpy(line->data, (void *) (address & ~cache->block_offset_mask), cache->block_size);

    /*
     * And return it.
     */
     
    return line;
}

/*
 * Read a single integer from the cache.
 */
int cache_read(cache_t *cache, int *address)
{
    /* TO BE COMPLETED BY THE STUDENT */
    intptr_t a = address;
    intptr_t tag = a >> (cache->tag_shift);
    //intptr_t tag = a >> (cache->tag_shift - 4);

    int cache_index = (a >> cache->cache_index_shift) & cache->cache_index_mask;
    //int cache_index = (a & cache->cache_index_mask) >> cache->cache_index_shift;

    int block_offset = a & cache->block_offset_mask;
    cache_set_t * cache_set = cache->sets + cache_index;
    cache_line_t * line = cache_set_find_matching_line(cache, cache_set, tag);
    if(!line){
        cache->miss_count++;
        line = cache_set_add(cache, cache_set, address, tag);
    }
    
    cache->access_count++;
    
    return cache_line_retrieve_data(line, block_offset);
}

/*
 * Return the number of cache misses since the cache was created.
 */
int cache_miss_count(cache_t *cache)
{
    return cache->miss_count;
}

/*
 * Return the number of cache accesses since the cache was created.
 */
int cache_access_count(cache_t *cache)
{
    return cache->access_count;
}
