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

#include <sosal/list.h>
#include <sosal/debug.h>

node_t *node_create(value_t data)
{
    node_t *node = calloc(1, sizeof(node_t));
    if (node)
        node->data = data;
    return node;
}

value_t node_destroy(node_t *node)
{
    value_t data = 0;
    if (node)
    {
        data = node->data;
        free(node);
    }
    return data;
}

// creates all the nodes, and puts them into the free list
list_t *list_create()
{
    return (list_t *)calloc(1,sizeof(list_t));
}

// deletes all the nodes and data structures
void list_destroy(list_t *list)
{
    if (list)
    {
        // delete all remaining items in the list (head first)
        while (list_pop(list) != NULL);
        // screw up the data
        memset(list, 0xEF, sizeof(list_t));
        free(list);
    }
}

// adds the node to the tail of the list
void list_append(list_t *list, node_t *node)
{
    if (list == NULL || node == NULL)
        return;

    if (list->tail != NULL)
    {
        assert(list->tail != node);
        list->tail->next = node;
        node->prev = list->tail;
    }
    else
    {
        node->prev = NULL;
        list->head = node;
    }
    node->next = NULL;
    list->tail = node;
}

// subtracts from the tail of the list
node_t *list_trim(list_t *list)
{
    node_t *node = NULL;

    if (list == NULL)
        return NULL;

    if (list->tail != NULL)
    {
        node = list->tail;
        // try to detect a cycle
        assert(node != list->tail->prev);
        list->tail = list->tail->prev;

        if (list->tail != NULL)
            list->tail->next = NULL;
        else // tail is NULL
            list->head = NULL;

        if (list->head == NULL)
            assert(list->tail == NULL);

        node->prev = NULL;
        node->next = NULL;
    }
    return (node);
}

// adds to the head of the list
void list_push(list_t *list, node_t *node)
{
    if (list == NULL)
        return;

    if (list->head != NULL)
    {
        assert(list->head != node);
        list->head->prev = node;
        node->next = list->head;
    }
    else
    {
        node->next = NULL;
        list->tail = node;
    }
    node->prev = NULL;
    list->head = node;
}

// subtracts from the head of the list
node_t *list_pop(list_t *list)
{
    node_t *node = NULL;

    if (list == NULL)
        return NULL;

    if (list->head != NULL)
    {
        node = list->head;
        // try to detect a cycle
        assert(node != list->head->next);
        list->head = list->head->next;

        if (list->head != NULL)
            list->head->prev = NULL;
        else // head is now null
            list->tail = NULL;

        if (list->tail == NULL)
            assert(list->head == NULL);

        node->next = NULL;
        node->prev = NULL;
    }
    return (node);
}

// inserts anywhere in the list, based on a comparision algorithm, and an order
node_t *list_insert(list_t *list,
                    node_t *node,
                    node_compare_f compare,
                    bool_e sortAscending)
{
    node_t *n = NULL;

    if (list == NULL)
        return NULL;

    if (list->head != NULL)
    {
        n = list->head;
        while(n != NULL)
        {
            if (sortAscending)
            {
                // if node < n
                if (compare(node, n) < 0)
                {
                    // this can only really happen in the head case
                    node->next = n;
                    node->prev = n->prev;
                    if (n->prev != NULL)
                        n->prev->next = node;
                    n->prev = node;

                    if (n == list->head)
                        list->head = node;

                    return (node);
                }
                // if n <= node <= n->next
                else if ((compare(n, node) <= 0) && (compare(node, n->next) <= 0))
                {
                    node->next = n->next;
                    node->prev = n;
                    if (n->next != NULL)
                        n->next->prev = node;
                    n->next = node;

                    if (n == list->tail)
                        list->tail = node;

                    return (node);
                }
            }
            else
            {
                // if node > n
                if (compare(node, n) > 0)
                {
                    // this can only really happen in the head case
                    node->next = n;
                    node->prev = n->prev;
                    if (n->prev != NULL)
                        n->prev->next = node;
                    n->prev = node;

                    if (n == list->head)
                        list->head = node;

                    return (node);
                }
                // if n >= node >= n->next
                else if ((compare(n, node) >= 0) && (compare(node, n->next) >= 0))
                {
                    node->next = n->next;
                    node->prev = n;
                    if (n->next != NULL)
                        n->next->prev = node;
                    n->next = node;

                    if (n == list->tail)
                        list->tail = node;

                    return (node);
                }
            }

            n = n->next;
        }
    }
    else // there is no list
    {
        list->head = node;
        list->tail = node;
        node->prev = NULL;
        node->next = NULL;
        return (node);
    }
    return (NULL);
}

node_t *list_extract(list_t *list, node_t *node)
{
    if (list == NULL || node == NULL)
        return NULL;

    if (list->head == NULL || list->tail == NULL)
        return NULL; // there are no nodes

    if (node->next == NULL && node->prev == NULL)
    {
        // only node
        node = list_pop(list);
    }
    else if (node->next != NULL && node->prev == NULL)
    {
        // head
        node = list_pop(list);
    }
    else if (node->next == NULL && node->prev != NULL)
    {
        // tail
        node = list_trim(list);
    }
    else if (node->next != NULL && node->prev != NULL)
    {
        // middle
        node->next->prev = node->prev;
        node->prev->next = node->next;
    }
    node->next = NULL;
    node->prev = NULL;
    return node;
}

// this removes the first node in which the compare function returns 0
node_t *list_remove_match(list_t *list,
                    node_t *node,
                    node_compare_f compare)
{
    if (list != NULL)
    {
        // find the node
        node_t *temp = list_search(list, node, compare);
        if (temp != NULL)
        {
            temp = list_extract(list, temp);
        }
        return (temp);
    }
    else
        return NULL;
}


node_t *list_search(list_t *list,
                    node_t *node,
                    node_compare_f compare)
{
    if (list != NULL && node != NULL)
    {
        node_t *temp = list->head;
        while (temp != NULL)
        {
            if (compare(temp, node) == 0)
                break;
            temp = temp->next;
        }
        return (temp);
    }
    else
        return NULL;
}

node_t *list_score(list_t *list, node_t *node, node_rate_f rate)
{
    if (list != NULL)
    {
        node_t *temp = list->head;
        node_t *best = NULL;
        score_t rating = 0;

        if (node == NULL)
            return NULL;
        while (temp != NULL)
        {
            score_t this_score = rate(temp, node);
            SOSAL_PRINT(SOSAL_ZONE_LIST, "list_score(): Node %p had a score of %08x\n", temp, this_score);
            if (rating < this_score) {
                rating = this_score;
                best = temp;
            }
            if (temp != temp->next)
                temp = temp->next;
            else
            {
                SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR! CYCLE IN LIST!\n");
                return NULL;
            }
        }
        SOSAL_PRINT(SOSAL_ZONE_LIST, "list_score(): the best node was %p\n", best);
        return best;
    }
    else
        return NULL;
}

size_t list_length(list_t *list)
{
    if (list != NULL)
    {
        size_t len = 0;
        node_t *temp = list->head;
        while (temp != NULL)
        {
            len++;
            temp = temp->next;
        }
        return len;
    }
    else
        return 0;
}

void list_print(list_t *list, node_print_f print)
{
    if (list != NULL)
    {
        node_t *temp = list->head;
        while (temp != NULL)
        {
            print(temp);
            temp = temp->next;
        }
    }
}

static int node_sorter(node_t *a, node_t *b)
{
    if (a && b)
    {
        if (a->data < b->data)
            return -1;
        else if (a->data == b->data)
            return 0;
        else if (a->data > b->data)
            return 1;
    }
    return 0;
}

bool_e list_unittest(int argc __attribute__((unused)),
                     char *argv[] __attribute__((unused)))
{
    bool_e ret = true_e;
    uint32_t i;
    node_t nodes[10];
    list_t *list = list_create();

    for (i = 0; i < dimof(nodes); i++)
    {
        do {
            nodes[i].data = (value_t)rand();
        } while (nodes[i].data == 0);
        //printf("Inserting %u %p\n", i, nodes[i].data);
        list_insert(list, &nodes[i], node_sorter, true_e);
    }

    if (list_length(list) != dimof(nodes))
        ret = false_e;

    for (i = 0; i < dimof(nodes); i++)
    {
        list_pop(list);
    }

    if (list->head != NULL || list->tail != NULL)
        ret = false_e;

    list_destroy(list);

    return ret;
}

/*******************************************************************************
 * END
 ******************************************************************************/
