/**
 * @file gl_thread.c
 * @author Matthew Zito (goldmund@freenode)
 * @brief Implements a GLUE Doubly Linked List
 * @version 0.1
 * @date 2021-07-11
 *
 * @copyright Copyright (c) 2021 Matthew Zito (goldmund)
 *
 */

#include "libcartilage.h"

#include <stdlib.h>

/**
 * @brief Initialize a new glthread
 *
 * @param glthread
 */
void glthread_init(glthread_t* glthread) {
	glthread->prev = NULL;
	glthread->next = NULL;
}

/**
 * @brief Insert a new glthread node after the given mark
 *
 * @param mark
 * @param next
 */
void glthread_insert_after(glthread_t* mark, glthread_t* next) {
	if (!mark->next) {
		mark->next = next;
		next->prev = mark;
	} else {
		glthread_t* tmp = mark->next;
		mark->next = next;
		next->prev = mark;
		next->next = tmp;
		tmp->prev = next;
	}
}

/**
 * @brief Insert a new glthread node before the given mark
 *
 * @param mark
 * @param next
 */
void glthread_insert_before(glthread_t* mark, glthread_t* next) {
	if (!mark->prev) {
		next->prev = NULL;
		next->next = mark;
		mark->prev = next;
	} else {
		glthread_t* tmp = mark->prev;
		tmp->next = next;
		next->prev = tmp;
		next->next = mark;
		mark->prev = next;
	}
}

/**
 * @brief Remove a given glthread node
 *
 * @param mark
 */
void glthread_remove(glthread_t* mark) {
	if (!mark->prev) {
		if (mark->next) {
			mark->next->prev = NULL;
			mark->next = 0;
			return;
		}
		return;
	}

	if (!mark->next) {
		mark->prev->next = NULL;
		mark->prev = NULL;
		return;
	}

	mark->prev->next = mark->next;
	mark->next->prev = mark->prev;
	mark->prev = 0;
	mark->next = 0;
}

/**
 * @brief Push new glthread node to tail
 *
 * @param head
 * @param next
 */
void glthread_push(glthread_t* head, glthread_t* next) {
	glthread_t* thread_node = NULL,
		* prev_thread_node = NULL;

	ITERATE_GLTHREAD_BEGIN(head, thread_node) {
		prev_thread_node = thread_node;
	} ITERATE_GLTHREAD_END(head, thread_node);

	if (prev_thread_node) {
		glthread_insert_after(prev_thread_node, next);
	} else {
		glthread_insert_after(head, next);
	}
}

/**
 * @brief Delete all nodes in a given glthread
 *
 * @param head
 */
void glthread_del_list(glthread_t* head) {
	glthread_t* thread_node = NULL;

	ITERATE_GLTHREAD_BEGIN(head, thread_node) {
		glthread_remove(thread_node);
	} ITERATE_GLTHREAD_END(head, thread_node);
}

/**
 * @brief Get current size of glthread
 *
 * @param head
 * @return unsigned int
 */
unsigned int glthread_size(glthread_t* head) {
	unsigned int size = 0;
	glthread_t* thread_node = NULL;

	ITERATE_GLTHREAD_BEGIN(head, thread_node) {
		size++;
	} ITERATE_GLTHREAD_END(head, thread_node);

	return size;
}

/**
 * @brief
 *
 * @param head
 * @param glthread
 * @param comparator
 * @param offset
 */
void glthread_priority_insert(
	glthread_t* head,
	glthread_t* glthread,
	int(*comparator)(void*, void*),
	int offset
) {
	glthread_t* curr = NULL,
		* prev = NULL;

	glthread_init(glthread);

	if (IS_GLTHREAD_EMPTY(head)) {
		glthread_insert_after(head, glthread);
		return;
	}

	// single node glthread
	if (head->next && !head->next->next) {
		if (comparator(GET_DATA_FROM_OFFSET(glthread, offset),
			GET_DATA_FROM_OFFSET(head->next, offset)) == -1) {
			glthread_insert_after(head, glthread);
		} else {
			glthread_insert_after(head->next, glthread);
		}

		return;
	}

	ITERATE_GLTHREAD_BEGIN(head, curr) {
		if (comparator(GET_DATA_FROM_OFFSET(glthread, offset),
			GET_DATA_FROM_OFFSET(curr, offset)) != -1) {
			prev = curr;
			continue;
		}

		if (!prev) glthread_insert_after(head, glthread);
		else glthread_insert_after(prev, glthread);

		return;
	} ITERATE_GLTHREAD_END(head, curr);

	// add tail
	glthread_insert_after(prev, glthread);
}

/**
 * @brief Dequeue the head node
 *
 * @param head
 * @return glthread_t*
 */
glthread_t* glthread_dequeue_first(glthread_t* head) {
	glthread_t* tmp;

	if (!head->next) return NULL;

	tmp = head->next;
	glthread_remove(tmp);

	return tmp;
}
