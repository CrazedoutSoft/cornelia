/*
Copyright (c) 2022 CrazedoutSoft / Fredrik Roos

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished
to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _CORNY_L_LIST_
#define _CORNY_L_LIST_

typedef struct l_node_t {

        void* data;
        struct l_node_t* prev;
        struct l_node_t* next;

} l_node;

typedef l_node	l_list;

typedef int (*COMPARE_FUNC)(void* data, int l_index);

l_list* list_init();
l_node* list_remove(l_list* list, int index);
l_node* list_get(l_list* list, int index);
l_list* list_restore(l_list* list, const char* file);
void    list_add_item(l_list* list, void* data, int size);
void    list_add(l_node* list, l_node* node);
void    list_free(l_list* list);
void    list_clean(l_list* list);
void    list_insert(l_list* list, l_node* item, int index);
int     list_size(l_list* list);
int 	list_compare(l_list* list, COMPARE_FUNC);
void    list_store(l_list* list, const char* file);

#endif
