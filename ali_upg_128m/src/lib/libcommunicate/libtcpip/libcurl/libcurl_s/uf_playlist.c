#include <sys_config.h>
#include <api/libc/alloc.h>
#include <string.h>
#include <stdio.h>
#include "uf_playlist.h"

#define pl_warning libc_printf

static uf_playlist_t *gs_net_playlist = NULL;

uf_playlist_t* uf_playlist_new(void)
{
    uf_playlist_t *pl = NULL;

    pl = (uf_playlist_t *)MALLOC(sizeof(uf_playlist_t));

    if (NULL == pl)
    {
        pl_warning("%s, malloc playlist fail.\n", __FUNCTION__);
        return NULL;
    }

    pl->count = 0;
    pl->list = NULL;

    return pl;
}

uf_playlist_entry* uf_playlist_entry_new(void)
{
    uf_playlist_entry *entry = NULL;

    entry = (uf_playlist_entry*)MALLOC(sizeof(uf_playlist_entry));

    if (NULL == entry)
    {
        return NULL;
    }

    MEMSET(entry, 0, sizeof(uf_playlist_entry));
    
    return entry;
}

void uf_playlist_entry_free(uf_playlist_entry *entry)
{
    if (entry)
    {
        FREE(entry);
    }
}

void uf_playlist_free(uf_playlist_t *pl)
{
    uf_playlist_node_t *node = NULL;
    uf_playlist_node_t *n_node = NULL;

    if (pl)
    {
        node = pl->list;
        while (node)
        {
            uf_playlist_entry_free(node->entry);
            n_node = node->next;
            FREE(node);
            node = n_node;
        }

        FREE(pl);
    }
}

BOOL uf_playlist_add_entry(uf_playlist_t *pl, uf_playlist_entry *entry)
{
    BOOL ret = FALSE;
    uf_playlist_node_t *list = NULL;
    uf_playlist_node_t *node = NULL;
    if ((NULL == pl) || (NULL == entry))
    {
        return ret;
    }

    node = (uf_playlist_node_t *)MALLOC(sizeof(uf_playlist_node_t));
    node->next = NULL;
    node->entry = entry;

    list = pl->list;

    /* Insert to tail. */
    if (NULL == list)
    {
        pl->list = node;
    }
    else
    {
        while (list->next)
        {
            list = list->next;
        }
        list->next = node;
    }

    pl->count++;
    ret = TRUE;
    return ret;
}

BOOL uf_playlist_del_entry(uf_playlist_t *pl, uf_playlist_entry *entry)
{
    BOOL ret = FALSE;

    // NOT IMPLEMEMT YET.
    
    return ret;
}

INT32 uf_playlist_get_entry_cnt(uf_playlist_t *pl)
{
    if (NULL == pl)
    {
        return 0;
    }

    return pl->count;
}

BOOL uf_playlist_get_entry(uf_playlist_t *pl, UINT32 entry_idx, uf_playlist_entry *entry)
{
    BOOL    ret = FALSE;
    UINT32  k = 0;
    uf_playlist_node_t *node = NULL;

    if ((NULL == pl) || (NULL ==entry) || (entry_idx >= pl->count))
    {
        return ret;
    }

    node = pl->list;

    while (NULL != node)
    {
        if (k==entry_idx)
        {
            strncpy(entry->title, node->entry->title, UF_TITLE_LEN-1);
            strncpy(entry->link, node->entry->link, UF_URL_LEN-1);
            ret = TRUE;
            break;
        }
        node = node->next;
        k++;
    }

    return ret;
}

BOOL uf_playlist_set_cur_list(uf_playlist_t *list)
{
    if (gs_net_playlist)
    {
        uf_playlist_free(gs_net_playlist);
        gs_net_playlist = NULL;
    }
    gs_net_playlist = list;
    return TRUE;
}

INT32 uf_playlist_get_cur_list_entry_cnt(void)
{
    if (gs_net_playlist)
    {
        return uf_playlist_get_entry_cnt(gs_net_playlist);
    }

    return 0;
}

BOOL uf_playlist_get_cur_list_entry(UINT32 idx, uf_playlist_entry *entry)
{
    if (gs_net_playlist)
    {
        return uf_playlist_get_entry(gs_net_playlist, idx, entry);
    }
    return FALSE;
}

