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

#ifndef _VISION_CAM_UTILS_H_
#define _VISION_CAM_UTILS_H_

#ifdef DVP_USE_ION
#   include <sosal/allocator.h>
#endif // DVP_USE_ION

#include <sosal/sosal.h>
#include <dvp/dvp_debug.h>

#define FREE_IF_PRESENT(data) { if(data) free(data); data = NULL; }

#ifdef DVP_USE_ION
#   define ALLOCATOR_HANDLES_NUM   (4)
#   define ALLOCATOR_DIMENSIONS    (1)
#endif // DVP_USE_ION

#ifndef here
#define here {printf("=======> VCam Utils - %d <=======\n", __LINE__);fflush(stdout);}
#endif // here

/** @fn void flushList(list_t *list)
   * Flushes a list data.
*/

class DataBuffer_t
{
public:
    /** @fn DataBuffer_t::DataBuffer_t(size_t allocSize )
      * Constructor.
      * Allocates a data buffers with a given size.
      * @param size_t allocSize - the size of the buffer to be allocated.
    */
    DataBuffer_t(size_t allocSize );
    ~DataBuffer_t();

    void *getData();

    size_t getSize();

    status_e push(void* data, size_t size);

private:
    /** Locks read-write operations.
    */
    mutex_t mBuffLock;

#ifdef DVP_USE_ION
    /** System Shared Mem allocator
    */
    allocator_t *pAllocator;
    _allocator_dimensions_t dims[ALLOCATOR_DIMENSIONS];
    allocator_dimensions_t strides;
    value_t handles[ALLOCATOR_HANDLES_NUM];// = { 0, 0, 0, 0 };
#endif // DVP_USE_ION

    /** Size of pDate buffer. */
    size_t nBuffSize;

    /** Used for memcpy monitoring */
    size_t bytesWritten;

    /** The buffer for 3A parameters in hardware acceptable format. */
    void *pData;

private: /// Private Methods
    /** @fn DataBuffer_t::DataBuffer_t(){}
      * Hide default constructor.
      * We want DataBuffer_t variables to be instantiated only with a valid data inside.
    */
    DataBuffer_t(){}

    /** Forbid operator=() . outside this class. */
    DataBuffer_t &operator=(DataBuffer_t &sBuff __attribute__ ((unused)) ){return *this;}
    DataBuffer_t &operator=(const DataBuffer_t &sBuff __attribute__ ((unused)) ){return *this;}
};

/** @class VisionCamExecutionService
  * This class provide the ability to register a member function and parameter size against an ID.
  * The registered function must be a class member function, and must accept a void* parameter with known size.
  * Please take care to avoid collisinos in ID values.
*/
template <class CookieType, typename funcPtrType>
class VisionCamExecutionService {
public:
    /** @typedef execFuncPrt_t
      * defines a type pointer to a 'Set_3A_*' member to function of 'CookieType' class.
    */
    typedef funcPtrType execFuncPrt_t;

    /** @struct execListNode_t
      * Defines a structure that holds an ID against each Set_3A_* method in this class.
    */
    typedef struct _exec_list_node_t_ {
        int32_t         exec_index;
        execFuncPrt_t   exec_func;
        size_t          exec_data_size;
        void            *exec_data;
    }execListNode_t;

    /** @fn VisionCamExecutionService(void *cook)
      * VisionCamExecutionService constructor.
      *
      * @param cook the caller class name.
    */
    VisionCamExecutionService(void *cook) {
        mExecutionDataList = list_create();
        cookie = (CookieType*)cook;
    }

    /** @fn ~VisionCamExecutionService()
      * VisionCamExecutionService class destructor.
    */
    ~VisionCamExecutionService() {

        flushList( mExecutionDataList );

        list_destroy(mExecutionDataList);

        mExecutionDataList = NULL;
    }

    /** @fn bool_e Register(int32_t index, execFuncPrt_t execFunc, size_t dataSize )
      * Registers a class member function against an ID and parameter size.
      *
      * @param index - integer value used as a key in function pointer list.
      * @param execFunc - pointer to a function, with return type status_e and a sigle vaoid* parameter.
      * @param dataSize - size of the parameter that this function (pointed by execFunc) accepts.
      * @return true_e on success and false_e on failure.
    */
    bool_e Register(int32_t index, execFuncPrt_t exec_func, size_t data_size, void *data_in =  NULL )
    {
        bool_e ret = false_e;
        node_t *node = (node_t*)calloc(1, sizeof(node_t));

        if( node )
        {
            node->data = (value_t)calloc(1, sizeof(execListNode_t));
            if( node->data )
            {
                execListNode_t* ex_node = (execListNode_t*)(node->data);
                ex_node->exec_index   = index;
                ex_node->exec_func       = exec_func;
                ex_node->exec_data_size   = data_size;
                ex_node->exec_data       = NULL;

                if( data_in && data_size )
                {
                    ex_node->exec_data = calloc(1, data_size );

                    if( ex_node->exec_data )
                        memcpy( ex_node->exec_data, data_in, data_size );
                }

                list_push(mExecutionDataList, node);

                ret = true_e;
            }
        }

        return ret;
    }

    /** @fn execFuncPrt_t getFunc( int32_t index )
      * retunrs a pointer to a member function that is registered against given ID.
      *
      * @param index - the ID with which the function is registered.
      * @return pointer to a function of type execFuncPrt_t.
    */
    execFuncPrt_t getFunc( int32_t index )
    {
        execListNode_t *n = getExecNode(index);
        if ( n ) return n->exec_func;
        return NULL;
    }

    /** @fn void *getData( int32_t index )
      * gets the data that will be passed to a function, registerred against index
      *
      * @param index - the ID with which the function is registered.
      * @return pointer to a data that will be passed.
    */
    void *getData( int32_t index )
    {
        execListNode_t *n = getExecNode(index);
        if( n ) return n->exec_data;
        return NULL;
    }

    /** @fn void setData( int32_t index )
      * sets the data that will be passes to a function, registeres againt index
      *
      * @param index - the ID with which the function is registered.
      * @return true_e if operation is successful; false_e if nothing is registerred against index
    */
    bool_e setData( int32_t index , void *data)
    {
        execListNode_t *n = getExecNode(index);
        if( n )
        {
            if( NULL == n->exec_data )
                n->exec_data = calloc(1, n->exec_data_size);

            memcpy(n->exec_data, data, n->exec_data_size );

            return true_e;
        }
        else
        {
            return false_e;
        }
    }

    /** @fn size_t getDataSize( int32_t index )
      * Returns the size of the data that must be passed to a function registered against an ID.
      * @param index - the ID with which the function is registered.
      * @return the size of the parameter.
    */
    size_t getDataSize( int32_t index )
    {
        execListNode_t *n = getExecNode(index);
        if( n ) return n->exec_data_size;
        return 0;
    }

private:
    /** Instance to the caller class.
    */
    CookieType *cookie;

    /** list_t *mExecutionDataList
    */
    list_t *mExecutionDataList;

    static int nodeCompare(node_t *a, node_t *b)
    {
        int ret = -1;
        if( a && b )
        {
            execListNode_t *aa = (execListNode_t*)a->data;
            execListNode_t *bb = (execListNode_t*)b->data;

            if( aa && bb )
            {
                if( aa->exec_index == bb->exec_index ) ret = 0;
                else if( aa->exec_index < bb->exec_index ) ret = -1;
                else ret = 1;
            }
        }
        return ret;
    }

    /** @fn execListNode_t *getExecNode(int32_t index)
      * @return the data structure in which given execution function, stays beside the "index" value.
    */
    execListNode_t *getExecNode(int32_t index)
    {
        execListNode_t execNode;
        execNode.exec_index     = index;
        execNode.exec_func      = NULL;
        execNode.exec_data      = NULL;
        execNode.exec_data_size = 0;

        node_t node = { NULL, NULL, (value_t)&execNode };
        node_t *found = list_search( mExecutionDataList, &node, reinterpret_cast<node_compare_f>(nodeCompare));

        return ( found ? (execListNode_t*)(found->data) : NULL);
    }


    void flushList(list_t *list )
    {
        node_t *node = list_pop( list );

        while( node )
        {
            flush_node( node );
            node = list_pop( list );
        }
    }

    void flush_node(node_t *node)
    {
        if( node != NULL )
        {
            execListNode_t *ex_node = (execListNode_t*)(node->data);

            if( ex_node && ex_node->exec_data )
            {
                free( ex_node->exec_data );
                ex_node->exec_data = NULL;
            }

            if (ex_node)
            {
                free(ex_node);
                ex_node = NULL;
            }

            free(node);
            node = NULL;
        }
    }
};

#endif // _VISION_CAM_UTILS_H_
