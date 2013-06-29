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

#include <sosal/btreelist.h>
#include <sosal/debug.h>

static btreelistnode_t *btreelist_rebtree(btreelist_t *btl, int32_t center, int32_t count)
{
    btreelistnode_t *node = NULL;

    if (count == btl->count)
    {
        if (btl->array) free(btl->array);
        btl->array = (btreelistnode_t **)calloc(count, sizeof(btreelistnode_t *));
        SOSAL_PRINT(SOSAL_ZONE_BTREE, "Re-allocaing array to %p for %u pointers\n", btl->array, btl->count);
        if (btl->array)
        {
            btreelistnode_t *node = btl->head;
            int32_t i = 0;
            SOSAL_PRINT(SOSAL_ZONE_BTREE, "Root=%p\n",btl->root);
            SOSAL_PRINT(SOSAL_ZONE_BTREE, "Head=%p\n",btl->head);
            while (node) {
                SOSAL_PRINT(SOSAL_ZONE_BTREE, "[%u] = %p (%s) prev=%p next=%p left=%p right=%p\n", i, node, btl->print(node), node->prev, node->next, node->left, node->right);
                btl->array[i++] = node;
                node = node->next;
            }
            SOSAL_PRINT(SOSAL_ZONE_BTREE, "Tail=%p\n",btl->tail);
        }
    }

    if (center < 0)
    {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "Center should never be negative! (%d)\n", center);
        return NULL;
    }

    if (count > 0)
    {
        SOSAL_PRINT(SOSAL_ZONE_BTREE, "Re-BTREE from center %d for %u nodes\n", center, count);
        node = btl->array[center];
        switch(count)
        {
            case 0:
                break; // just to be safe
            case 1:
                node->left = NULL;
                node->right = NULL;
                break;
            case 2:
                node->left = btreelist_rebtree(btl, center-1, 1);
                node->right = NULL;
                break;
            case 3:
                node->left = btreelist_rebtree(btl, center-1, 1);
                node->right = btreelist_rebtree(btl, center+1, 1);
                break;
            case 4:
                node->left = btreelist_rebtree(btl, center-1, 2);
                node->right = btreelist_rebtree(btl, center+1, 1);
                break;
            default:
            {
                uint32_t ll, lc;    // left length and count
                uint32_t rl, rc;    // right length and count
                if (count & 1) // odd, both sides are balanced
                {
                    ll = rl = count/2;
                }
                else // left side will be odd, right side will be even
                {
                    ll = count/2;
                    rl = ll-1;
                }
                lc = center - ll/2;
                if (ll & 1)
                    lc -= 1;
                rc = center + rl/2 + 1; // ever or odd
                node->left = btreelist_rebtree(btl, lc, ll);
                node->right = btreelist_rebtree(btl, rc, rl);
                break;
            }
        }
    }
    return node;
}

static void btreelist_insert(btreelist_t *btl, btreelistnode_t *node, btreelistnode_t *obj)
{
    int ret = 0;

    if (btl->root == NULL && btl->head == NULL && btl->tail == NULL) // very first insert
    {
        btl->root = obj;
        btl->head = obj;
        btl->tail = obj;
        btl->count = 1;
        memset(obj, 0, sizeof(btreelistnode_t));
        SOSAL_PRINT(SOSAL_ZONE_BTREE, "Creating first item %p in BTREELIST!\n", obj);
        return;
    }

    if (node == NULL)
        return;

    ret = btl->sort(node, obj);

    SOSAL_PRINT(SOSAL_ZONE_BTREE, "Inserting %p into btreelist, node=%p, ret=%d\n", obj, node, ret);

    if (ret == 0)
    {
        SOSAL_PRINT(SOSAL_ZONE_ERROR, "ERROR: Insert does not overwrite matching nodes!\n");
        return; // we already have one
    }
    else if (ret == 1)
    {
        if (node->right)
            btreelist_insert(btl, node->right, obj);
        else
        {
            SOSAL_PRINT(SOSAL_ZONE_BTREE, "Inserting %p as next to %p\n", obj, node);
            obj->next = node->next;
            if (node->next == NULL)
                btl->tail = obj;
            else
                node->next->prev = obj;
            node->next = obj;
            obj->prev = node;
            btl->count++;
        }
    }
    else if (ret == -1)
    {
        if (node->left)
            btreelist_insert(btl, node->left, obj);
        else
        {
            SOSAL_PRINT(SOSAL_ZONE_BTREE, "Inserting %p as prev to %p\n", obj, node);
            obj->prev = node->prev;
            if (node->prev == NULL)
                btl->head = obj;
            else
                node->prev->next = obj;
            node->prev = obj;
            obj->next = node;
            btl->count++;
        }
    }
}

void btreelist_add(btreelist_t *btl, btreelistnode_t *btln)
{
    btreelist_insert(btl, btl->root, btln);
    btl->root = btreelist_rebtree(btl, btl->count/2, btl->count);
}

static btreelistnode_t *btree_find(btreelist_t *btl, btreelistnode_t *btln, value_t value)
{
    int ret = 0;

    if (btln == NULL)
        return NULL;

    ret = btl->compare(btln, value);
    if (ret == 0)
        return btln;
    else if (ret == 1)
        return btree_find(btl, btln->right, value);
    else //if (ret == -1)
        return btree_find(btl, btln->left, value);
}

btreelistnode_t *btreelist_find(btreelist_t *btl, value_t value)
{
    btreelistnode_t *node = NULL;
    if (btl && btl->compare)
    {
        node = btree_find(btl, btl->root, value);
        if (node)
        {
            SOSAL_PRINT(SOSAL_ZONE_BTREE, "Found node %p, value %s\n", node, btl->print(node));
        }
    }
    return node;
}

btreelistnode_t *btreelist_remove(btreelist_t *btl, value_t value)
{
    // Find the item then rebtree
    btreelistnode_t *node = NULL;
    if (btl)
    {
        node = btreelist_find(btl, value);
        if (node)
        {
            // remove this item from the list
            if (node->prev == NULL) //head
                btl->head = node->next;
            else
                node->prev->next = node->next;
            if (node->next == NULL) //tail
                btl->tail = node->prev;
            else
                node->next->prev = node->prev;
            btl->count--;
            memset(node, 0, sizeof(btreelistnode_t));
            btl->root = btreelist_rebtree(btl, btl->count/2, btl->count);
        }
    }
    return node;
}

void btreelist_destroy(btreelist_t *btl)
{
    btreelistnode_t *node = btl->head;
    btreelistnode_t *next = NULL;
    while (node)
    {
        next = node->next;
        SOSAL_PRINT(SOSAL_ZONE_BTREE, "Freeing node %p (next=%p) (%s)\n", node, next, btl->print(node));
        memset(node, 0, sizeof(btreelistnode_t));
        free(node);
        node = next;
    }
    free(btl->array);
    memset(btl, 0, sizeof(btreelist_t));
    free(btl);
}

btreelist_t *btreelist_create(btreelistnode_compare compfunc, btreelistnode_sort sortfunc, btreelistnode_print printfunc)
{
    btreelist_t *btl = (btreelist_t *)calloc(1, sizeof(btreelist_t));
    if (btl)
    {
        btl->compare = compfunc;
        btl->sort = sortfunc;
        btl->print = printfunc;
    }
    return btl;
}

typedef struct _testvalue_t {
    btreelistnode_t node;
    value_t value;
} testvalue_t;

static int value_compare(btreelistnode_t *node, value_t value)
{
    testvalue_t *v = (testvalue_t *)node;
    if (v->value == value)
        return 0;
    else if (v->value < value)
        return 1;
    else //if (v->value > value)
        return -1;
}

static int value_sort(btreelistnode_t *node, btreelistnode_t *obj)
{
    testvalue_t *a = (testvalue_t *)node;
    testvalue_t *b = (testvalue_t *)obj;
    if (a->value == b->value)
        return 0;
    else if (a->value > b->value)
        return -1;
    else //if (a->value < b->value)
        return 1;
}

static char *value_print(btreelistnode_t *node)
{
    static char values[60];
    testvalue_t *a = (testvalue_t *)node;
    sprintf(values, ""FMT_VALUE_T"", a->value);
    return values;
}

int btreelist_unittest(int argc __attribute__((unused)),
                       char *argv[] __attribute__((unused)))
{
    btreelist_t *btl = btreelist_create(value_compare, value_sort, value_print);
    if (btl)
    {
        uint32_t i,c = 128;
        testvalue_t *vs = (testvalue_t *)calloc(c, sizeof(testvalue_t));
        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Adding All Nodes!\n");
        for (i = 0; i < c; i++)
        {
            vs[i].value = (value_t)rand();
            btreelist_add(btl, (btreelistnode_t *)&vs[i]);
        }

        SOSAL_PRINT(SOSAL_ZONE_ALWAYS, "Removing All Nodes!\n");
        for (i = 0; i < c; i++)
        {
            btreelist_remove(btl, vs[i].value);
        }
        btreelist_destroy(btl);
        btl = NULL;
    }
    return 0;
}

