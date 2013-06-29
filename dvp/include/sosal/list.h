/*
 *  Copyright (C) 2009-2012 Texas Instruments, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _SOSAL_LIST_H_
#define _SOSAL_LIST_H_

/*! \file
 * \brief The SOSAL Double Link List API.
 * \author Erik Rainey <erik.rainey@ti.com>
 */

#include <sosal/types.h>

typedef uint32_t score_t;
#define MAX_SCORE   (0xFFFFFFFF)
#define MIN_SCORE   (0)

/*! \brief The unit of a list_t. Typically a pointer is placed
 * in the value field.
 * \ingroup group_lists
 */
typedef struct _node_t{
    struct _node_t *next;   /*!< \brief The next node in the list */
    struct _node_t *prev;   /*!< \brief The previous node in the list */
    value_t data;             /*!< \brief The generic value in this node */
} node_t;

/*! \brief The List meta-data.
 * \ingroup group_lists
 */
typedef struct _list_t {
    node_t *head;
    node_t *tail;
} list_t;

/*!
 * \brief This type of function evaluates two nodes and returns some form of
 * comparison between the two.
 * \param [in] a The first node.
 * \param [in] b The second node.
 * \retval 0 when a = b or b == NULL
 * \retval 1 when a > b
 * \revtal (-1) when a < b
 * \note How the comparison works is up to the implementor.
 * \ingroup group_lists
 */
typedef int (*node_compare_f)(node_t *a, node_t *b);

/*!
 * \brief This type of function rates a node against another another node to
 * produce a score which is used to evaluate the current node. This is usually
 * use for a best-fit algorithm.
 * \param [in] current The node to anaylize
 * \param [in] best The node which is determined to be the best.
 * \return Returns a score which rates the evaluations.
 * \ingroup group_lists
 */
typedef score_t (*node_rate_f)(node_t *current, node_t *best);

/*!
 * \brief This prints the value of a node in a list.
 * \param [in] node The node to print.
 * \ingroup group_lists
 */
typedef void (*node_print_f)(node_t *node);

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief This creates a node from a data value.
 * \param [in] data The value to place in the node.
 * \return Returns a node pointer.
 * \retval NULL Indicates a memory allocation failed.
 * \post \ref node_destroy
 * \ingroup group_lists
 */
node_t *node_create(value_t data);

/*! \brief This destroys a node.
 * \param [in] node The node to destroy.
 * \return Returns the internal value_t.
 * \pre \ref node_create
 * \post Set your node pointer to NULL
 * \ingroup group_lists
 */
value_t node_destroy(node_t *node);

/*! \brief This creates a list
 * \return Returns a list meta-data structure.
 * \retval NULL indicates an error in allocation.
 * \post list_destroy
 * \ingroup group_lists
 */
list_t *list_create(void);

/*! \brief Destroys a list.
 * \param [in] list The list to destroy.
 * \pre \ref list_create
 * \post Set your list pointer to NULL.
 * \ingroup group_lists
 * \note This will remove all nodes and destroy them.
 */
void    list_destroy(list_t *list);

/*! \brief Appends a node to the tail of the list.
 * \param [in] list The list to modify
 * \param [in] node The node to add.
 * \pre \ref list_create
 * \ingroup group_lists
 */
void    list_append(list_t *list, node_t *node);

/*! \brief Removes the node at the tail of the list.
 * \param [in] list The list to modify.
 * \return Returns the node which was at the tail of the list.
 * \retval NULL if the tail was empty.
 * \pre \ref list_append or list_insert
 * \post \ref node_destroy
 * \ingroup group_lists
 */
node_t *list_trim(list_t *list);

/*! \brief This puts a node at the head of the list.
 * \param [in] list The list to modify.
 * \param [in] node The node to add to the head.
 * \pre \ref list_create
 * \ingroup group_lists
 */
void    list_push(list_t *list, node_t *node);

/*! \brief This removes a node at the head of the list.
 * \param [in] list
 * \return Returns the node at the head of the list.
 * \retval NULL if the list is empty.
 * \pre \ref list_create
 * \post \ref node_destroy
 * \ingroup group_lists
 */
node_t *list_pop(list_t *list);

/*! \brief This inserts in the order based on some sorting algorithm
 * \param [in] list The list to insert into.
 * \param [in] node The node to insert.
 * \param [in] compare The function to determine the comparision.
 * \return Returns node pointer inserted.
 * \pre \ref list_create
 * \ingroup group_lists
 * \note A list which uses unsorted access like \ref list_append, \ref list_trim
 *  or \ref list_push and \ref list_pop should not use the insert as the
 * nodes are unsorted and the insert depends on ordering.
 */
node_t *list_insert(list_t *list,
                    node_t *node,
                    node_compare_f compare,
                    bool_e sortAscending);

/*! \brief This removes the node supplied from the list.
 * \param [in] list The list to modify.
 * \param [in] node The node to remove.
 * \return Returns the node.
 * \pre \ref list_create
 * \post \ref node_destroy
 * \ingroup group_lists
 * \note Base on the pointer of node, not the value.
 */
node_t *list_extract(list_t *list, node_t *node);

/*! \brief This removes the first node in which the compare function returns 0
 * \param [in] list The list to modify
 * \param [in] node The node which contains the value to search for.
 * \param [in] compare The function which determines the comparisions.
 * \return Returns the node removed from the list.
 * \retval NULL if no node matched.
 * \pre \ref list_create
 * \post \ref node_destroy
 * \ingroup group_lists
 */
node_t *list_remove_match(list_t *list,
                          node_t *node,
                          node_compare_f compare);

/*! \brief This finds a node within the list based on some sorting algorithm
 * \param [in] list The list to modify
 * \param [in] node The node which contains the value to search for.
 * \param [in] compare The function which determine the comparisions.
 * \return Returns the found node.
 * \retval NULL if the node was not found.
 * \pre \ref list_create
 * \post \ref node_destroy
 * \ingroup group_lists
 */
node_t *list_search(list_t *list,
                    node_t *node,
                    node_compare_f compare);

/*! \brief This finds the node which has the best score aginst the supplied node.
 * \param [in] list The list to rate.
 * \param [in] node The node to rate against.
 * \param [in] rate The rating function.
 * \return Returns the node which best fits the rating function.
 * \retval NULL if no node is found.
 * \pre \ref list_create
 * \ingroup group_lists
 */
node_t *list_score(list_t *list,
                   node_t *node,
                   node_rate_f rate);

/*! \brief This computes the list's length.
 * \param [in] list The list to query.
 * \return Returns the size_t number of items in the list.
 * \retval 0 if there are no items.
 * \pre \ref list_create
 * \ingroup group_lists
 */
size_t list_length(list_t *list);

/*! \brief This prints all the nodes in the list using the supplied function.
 * \param [in] list The list to print.
 * \param [in] print The printing function.
 * \pre \ref list_create
 * \ingroup group_lists
 */
void list_print(list_t *list, node_print_f print);

#ifdef __cplusplus
}
#endif

#endif

