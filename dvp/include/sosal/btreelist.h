/*
 *  Copyright (C) 2009-2011 Texas Instruments, Inc.
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

#include <sosal/types.h>

/*!
 * \file btreelist.h
 * \brief The Binary Search Tree Structure.
 * \author Erik Rainey <erik.rainey@ti.com>
 */

/*! \brief The btreelistnode is both in a double link list of all nodes and
 * in the binary tree structure of nodes
 * \ingroup group_btreelists
 */
typedef struct btreelistnode {
    struct btreelistnode *next;     /*!< Next node in the double link list */
    struct btreelistnode *prev;     /*!< Previous node in the double link list */
    struct btreelistnode *left;     /*!< Left node in the binary search tree. */
    struct btreelistnode *right;    /*!< Right node in the binary search tree. */
} btreelistnode_t;

/*! \brief A typedef of a compare function used to find a specific node in the tree.
 * \ingroup group_btreelists
 */
typedef int (*btreelistnode_compare)(btreelistnode_t *node, value_t value);

/*! \brief A typedef of a sorting function used to sort nodes in the double link list
 * \ingroup group_btreelists
 */
typedef int (*btreelistnode_sort)(btreelistnode_t *node, btreelistnode_t *obj);

/*! \brief A typedef of a printing function used to debug the nodes
 * \ingroup group_btreelists
 */
typedef char *(*btreelistnode_print)(btreelistnode_t *node);

/*! \brief The top level Binary Tree List structure
 * \ingroup group_btreelists
 */
typedef struct btreelist {
    btreelistnode_t *root;      /*!< Top node in the binary tree */
    btreelistnode_t *head;      /*!< First node in the double link list */
    btreelistnode_t *tail;      /*!< Last node in the double link list */
    btreelistnode_t **array;    /*!< Pointer to array of pointers to all nodes in the tree (unordered). */
    btreelistnode_compare compare; /*!< The supplied compare function. */
    btreelistnode_sort    sort;    /*!< The supplied sorting function. */
    btreelistnode_print   print;   /*!< The supplied printing function. */
    int32_t        count;          /*!< The number of nodes in the tree. */
} btreelist_t;

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief This creates a binary tree list with the supplied funtions.
 * \param compfunc The user defined comparison function.
 * \param sortfunc The user defined sorting function.
 * \param printfunc The user defined printing function.
 * \returns btreelist_t *
 * \ingroup group_btreelists
 */
btreelist_t *btreelist_create(btreelistnode_compare compfunc, btreelistnode_sort sortfunc, btreelistnode_print printfunc);

/*! \brief This destroys a binary tree list.
 * \param btl The pointer to the binary tree list.
 * \ingroup group_btreelists
 * \pre \ref btreelist_create
 */
void btreelist_destroy(btreelist_t *btl);

/*! \brief This adds a node to a binary tree list.
 * \param btl The pointer to the binary tree list.
 * \param btln The pointer to the binary tree node to add to the structure.
 * \note the node must be preallocated elsewhere.
 * \ingroup group_btreelists
 */
void btreelist_add(btreelist_t *btl, btreelistnode_t *btln);

/*! \brief This finds a specific node in a binary tree list based on the
 * supplied value which is passed to the compare function.
 * \param btl The pointer to the binary tree list.
 * \param value The value used to find the matching node in the btreelist.
 * \ingroup group_btreelists
 */
btreelistnode_t *btreelist_find(btreelist_t *btl, value_t value);

/*! \brief This finds and removes a node from the binary tree list based on a
 * match to the supplied value via the compare method.
 * \param btl The pointer to the binary tree list.
 * \param value The value used to find and remove a matching node in the btreelist.
 * \ingroup group_btreelists
 */
btreelistnode_t *btreelist_remove(btreelist_t *btl, value_t value);

#ifdef __cplusplus
}
#endif

