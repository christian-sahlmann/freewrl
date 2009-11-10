/*
  $Id: list.c,v 1.2 2009/11/10 10:18:25 couannette Exp $

  FreeWRL support library.
  Linked lists.

*/

/****************************************************************************
    This file is part of the FreeWRL/FreeX3D Distribution.

    Copyright 2009 CRC Canada. (http://www.crc.gc.ca)

    FreeWRL/FreeX3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FreeWRL/FreeX3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FreeWRL/FreeX3D.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/



#include <config.h>
#include <system.h>
#include <internal.h>

#include <list.h>


/**
 * ml_new: create a new list item with a contained element 'elem'
 */
s_list_t* ml_new(const void *elem)
{
    s_list_t *item;
    item = XALLOC(s_list_t);
    ml_elem(item) = (void *) elem;
    return item;
}

/**
 * ml_count: count the number of items in 'list'
 */
int ml_count(s_list_t *list)
{
    int c = 0;
    while (list) {
        c++;
        list = ml_next(list);
    }
    return c;
}

/**
 * ml_prev: returns item preceding 'item' in 'list'
 * or NULL if either 'item' is null or 'item' cannot be found
 * in forward search from 'list'
 */
s_list_t* ml_prev(s_list_t *list, s_list_t *item)
{
    s_list_t *n;
    if (!item) return NULL;
    while (list) {
        n = ml_next(list);
        if (!n)
            return NULL;
        if (n == item)
            return list;
        list = n;
    }
    return NULL;
}

/**
 * ml_last: returns last item from 'list'
 */
s_list_t* ml_last(s_list_t *list)
{
    while (list) {
        if (!ml_next(list))
            break;
        list = ml_next(list);
    }
    return list;
}

/**
 * ml_find: returns 'item' if it can be found in 'list'
 */
s_list_t* ml_find(s_list_t *list, s_list_t *item)
{
    while (list) {
        if (list == item)
            return list;
        list = ml_next(list);
    }
    return NULL;
}

/**
 * ml_find_elem: returns first item found in 'list' where
 * item's element is equal to 'elem'
 */
s_list_t* ml_find_elem(s_list_t *list, void *elem)
{
    while (list) {
        if (ml_elem(list) == elem)
            return list;
        list = ml_next(list);
    }
    return NULL;
}

/**
 * ml_insert: inserts item 'item' before 'point' in 'list'
 */
s_list_t* ml_insert(s_list_t *list, s_list_t *point, s_list_t *item)
{
    if (!list) {
        if (item) ml_next(item) = point;
        return item;
    }
    if (!point || (list == point)) {
        if (item) ml_next(item) = list;
    } else {
        s_list_t *prev;
        prev = ml_prev(list, point);
        if (prev) {
                ml_next(prev) = item;
                ml_next(item) = point;
        } else {
                item = NULL;
        }
    }
    return item;
}

/**
 * ml_append: appends 'item' after last item of 'list'
 */
s_list_t* ml_append(s_list_t *list, s_list_t *item)
{
    if (list) {
        ml_next(ml_last(list)) = item;
        return list;
    }
    return item;
}

/**
 * ml_delete: destroy 'item' from 'list'
 * when 'item' holds a single value not to be freed
 */
void ml_delete(s_list_t *list, s_list_t *item)
{
    s_list_t *prev;
    prev = ml_prev(list, item);
    if (prev) {
	ml_next(prev) = ml_next(item);
	XFREE(item);
    } else {
	ERROR_MSG("ml_delete: trying to destroy first element in a list\n");
    }
}


/**
 * ml_delete_self: destroy 'item' from 'list', even if item is the first
 * when 'item' holds a single value not to be freed
 * return the new list, in case we delete the first item: this is a "new" list
 *
 * could also be implemented as a macro....
 */
s_list_t* ml_delete_self(s_list_t *list, s_list_t *item)
{
    s_list_t *it;
    if (list == item) {
	/* first element */
	it = item->next;
	XFREE(item);
	return it;
    }
    ml_delete(list, item);
    return list;
}

/**
 * ml_delete2: destroy element 'item' from list 'list'
 * when 'item' holds a pointer to be freed with f
 */
void ml_delete2(s_list_t *list, s_list_t *item, f_free_t f)
{
    s_list_t *prev;
    prev = ml_prev(list, item);
    ml_next(prev) = ml_next(item);
    if (ml_elem(item)) {
	f(ml_elem(item));
    } else {
	ERROR_MSG("ml_delete2: *error* deleting empty item %p from list %p\n", item, list);
    }
    XFREE(item);
}

/**
 * ml_delete: destroy all items from list 'list' when
 * items hold a single value not to be freed
 */
void ml_delete_all(s_list_t *list)
{
    s_list_t *next;
    while (list) {
        next = ml_next(list);
        XFREE(list);
        list = next;
    }
}

/**
 * ml_delete2: destroy all items from 'list' when
 * items hold a pointer to be freed with f
 */
void ml_delete_all2(s_list_t *list, f_free_t f)
{
    s_list_t *begin, *next;
    begin = list;
    if (!f) f = free;
    while (list) {
        if (ml_elem(list)) {
            f(ml_elem(list));
        } else {
		ERROR_MSG("ml_delete_all2: *error* deleting empty item %p from list %p\n", list, begin);
	}
        next = ml_next(list);
        XFREE(list);
        list = next;
    }
}

/**
 * ml_get: get item list[index] is if list 
 * was an array
 */
s_list_t* ml_get(s_list_t* list, int index)
{
    int i = 0;
    
    while (list) {
	if (i == index)
	    return list;
	list = ml_next(list);
	i++;
    }
    return NULL;
}

void ml_dump(s_list_t *list)
{
	TRACE_MSG("ml_dump (%p) : ", list);
	ml_foreach(list, TRACE_MSG("%p ", __l));
	TRACE_MSG("\n");
}

void ml_dump_char(s_list_t *list)
{
	TRACE_MSG("ml_dump_char (%p) : ", list);
	ml_foreach(list, TRACE_MSG("%s ", (char*)ml_elem(__l)));
	TRACE_MSG("\n");
}
