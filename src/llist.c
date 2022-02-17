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

#include "../include/llist.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


l_list* list_init(){

	l_list* l = (l_list*)malloc(sizeof(l_list));
	memset(l,0,sizeof(l_node));
	l->data=(malloc)(5);
	memcpy(l->data,"head",4);
	l->prev=NULL;
	l->next=NULL;

  return l;
}

void list_add_item(l_list* list, void* data, int size){

	l_node *ptr = list;
	while((ptr->next)!=NULL){
	  ptr=ptr->next;
	}

	l_node* node = list_init();
	node->data=malloc(size);
	memcpy(node->data,data,size);
	node->prev=ptr;
	node->next=NULL;
	ptr->next=node;
}

void list_add(l_list* list, l_node* node){

        l_node *ptr = list;
        while((ptr->next)!=NULL){
          ptr=ptr->next;
        }
        node->prev=ptr;
        node->next=NULL;
        ptr->next=node;
}

void list_free(l_list* list){

        l_node *ptr = list;
	l_node *tmp;
        while((ptr->next)!=NULL){
	  tmp=ptr;
          ptr=ptr->next;
	  free(tmp->data);
	  free(tmp);
        }
}

void list_clean(l_list* list){

        l_node *ptr = list->next;
        l_node *tmp;
	l_node *ptr2 = list;
        while((ptr->next)!=NULL){
          tmp=ptr;
          ptr=ptr->next;
          free(tmp->data);
          free(tmp);
        }
	ptr2->next=NULL;
	ptr2->prev=NULL;
}

l_node* list_get(l_list* list, int index){

        l_node *ptr = list;
        l_node *item = NULL;

	if(index > list_size(list)-1) return NULL;

        int n = 0;
        while((ptr->next)!=NULL){
          ptr=ptr->next;
          if(n==index){
	   return ptr;
          }
         n++;
        }

 return item;


}

l_node* list_remove(l_list* list, int index){

	l_node *ptr = list;
	l_node *item = NULL;

	int n = 0;
	while((ptr->next)!=NULL){
	  ptr=ptr->next;
	  if(n==index){
	   item=ptr;
	   ptr->prev->next = item->next;
	   break;
	  }
	 n++;
	}

 return item;
}

void list_insert(l_list* list, l_node* item, int index){

        l_node *ptr = list;
	l_node *tmp;

        int n = 0;
        while((ptr->next)!=NULL){
          if(n==index){
	    tmp=ptr->next;
	    ptr->next=item;
	    item->next=tmp;
	    item->prev=ptr;
           break;
          }
          ptr=ptr->next;
         n++;
        }

}

int list_compare(l_list* list, COMPARE_FUNC callback){

	int n = 0;
	int ret = 0;
	l_node* ptr=list;
	while((ptr->next)!=NULL){
	  ptr=ptr->next;
	  if(callback(ptr->data, n)) {
	   ret=1;
	   break;
	  };
 	  n++;
	}

 return ret;
}

int list_size(l_list* list){

	int n = 0;
	l_node* ptr=list;
	while((ptr->next)!=NULL){
	  ptr=ptr->next;
 	  n++;
	}

 return n;
}

void list_store(l_list* list, const char* file){

	FILE* fd;
	int size = list_size(list);

	if((fd=fopen(file,"wb"))!=NULL){
	  fwrite(&size,1,sizeof(int),fd);
	  for(int i = 0; i < size; i++){
	    l_node* n = list_get(list,i);
	    fwrite(n,1,sizeof(l_node),fd);
	  }
	  fclose(fd);
	}

}

l_list* list_restore(l_list* list, const char* file){

	FILE* fd;
	int size=0;
	l_node* n;

	if((fd=fopen(file,"rb"))!=NULL){
	 fread(&size,1,sizeof(int),fd);
	 for(int i = 0; i < size; i++){
	  n = (l_node*)malloc(sizeof(l_node));
	  fread(n,1,sizeof(l_node),fd);
	  list_add(list,n);
	 }
	 fclose(fd);
	}

 return list;
}
