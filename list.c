/*
Simple fat32 implementation 
Copyright (C) 2016  Andreu Carminati
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "list.h"
#include <stdio.h>
#include <stdlib.h>

void insert(LIST *q, int key,  void *data){
	NODE *n = NULL;
	NODE *aux = NULL;

	n = (NODE*) malloc(sizeof(NODE));
	n->data = data;
	n->key = key;
	
	if(!q->head){	
		q->head = n;
	} else{
		aux = q->head;
		while(aux->next != NULL){
			aux = aux->next;
		}
		aux->next = n;
		
	}
	q->size++;
} 

LIST* alloc_list(void){
	LIST *q;

	q = (LIST*) malloc(sizeof(LIST));
	q->head = NULL;
	q->size = 0;

	return q;
}

int is_empty(LIST *q){
	int result;

	if(q->head == NULL){
		result = 1;
	} else {
		result = 0;
	}
	return result;
}

void rem(LIST* list, int key){
	NODE *head;
	head = list->head;

	while(head != NULL){
		if(head->key == key){
			head->key = -1; // invalidate
		}
		head = head->next;
	}
}

int contains(LIST* list, int key){
	NODE *head;
	head = list->head;

	while(head != NULL){
		if(head->key == key){
			return 1;
		}
		head = head->next;
	}
	return 0;
}

NODE* get(LIST* list, int key){
	NODE *head;
	head = list->head;

	while(head != NULL){
		if(head->key == key){
			return head->data;
		}
		head = head->next;
	}
	return NULL;
}


