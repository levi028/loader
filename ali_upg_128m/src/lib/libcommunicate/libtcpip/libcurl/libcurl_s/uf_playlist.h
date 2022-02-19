
#ifndef _UF_PLAYLIST_H_
#define _UF_PLAYLIST_H_

#include <api/libcurl/urlfile.h>

typedef struct uf_playlist_node uf_playlist_node_t;
typedef struct uf_playlist      uf_playlist_t;

struct uf_playlist_node
{
    uf_playlist_node_t  *next;
    uf_playlist_entry   *entry;
};

struct uf_playlist
{
    int                 count;
    uf_playlist_node_t *list;
};

/* uf_playlist_new
 *
 * New one playlist from network.
 *
 * return One playlist header node.
 */
uf_playlist_t *uf_playlist_new(void);

/* uf_playlist_free
 *
 * Free one playlist, it will cause all malloc memory in this list free.
 *
 * @pl The playlist that going to free.
 */
void uf_playlist_free(uf_playlist_t *pl);

/* uf_playlist_entry_new
 *
 * New one entry node.
 *
 * return New entry node address if success; NULL if fail.
 */
uf_playlist_entry *uf_playlist_entry_new(void);

/* uf_playlist_entry_free
 *
 * Free entry node and member's memory.
 *
 * @entry The entry node going to be free.
 */
void uf_playlist_entry_free(uf_playlist_entry *entry);


/* uf_playlist_add_entry
 *
 * Add one entry into playlist.
 *
 * @pl The playlist that going to inserted.
 * #entry The entry that going to insert.
 *
 * return TRUE if success; FALSE if fail.
 */
BOOL uf_playlist_add_entry(uf_playlist_t *pl, uf_playlist_entry *entry);

/* uf_playlist_del_entry
 *
 * Delete one entry from the playlist, this function will compare
 * the input entry value vs the entrys in playlist.
 *
 * @pl The playlist that going to be operated.
 * @entry The entry value that going to be deleted.
 *
 * return TRUE if delete success; FALSE if fail.
 */
BOOL uf_playlist_del_entry(uf_playlist_t *pl, uf_playlist_entry *entry);

/* uf_playlist_get_entry_cnt
 *
 * Get the entry count of the playlist.
 *
 * @pl The playlist.
 *
 * return Entry count of the playlist.
 */
INT32 uf_playlist_get_entry_cnt(uf_playlist_t *pl);

/* uf_playlist_get_entry
 *
 * Get specified index entry from playlist, user shall free entry's members memory after use,
 * because this function will alloc new space for title and link.
 *
 * @pl The playlist
 * @entry_idx The specified index.
 * @entry Output entry pointer.
 *
 * return TRUE if success; FALSE if fail.
 */
BOOL uf_playlist_get_entry(uf_playlist_t *pl, UINT32 entry_idx, uf_playlist_entry *entry);

/* uf_playlist_set_cur_list
 *
 * Set current parsed network playlist, it will be keep until next playlist parsed and set.
 *
 * @list Parsed playlist.
 * return TRUE if set success; FALSE if fail.
 */
BOOL uf_playlist_set_cur_list(uf_playlist_t *list);

#endif

