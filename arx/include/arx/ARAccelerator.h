/*
 *  Copyright (C) 2012 Texas Instruments, Inc.
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

#ifndef _ARACCELERATOR_H_
#define _ARACCELERATOR_H_

#include <dvp/dvp_types.h>
#include <arx/ARXStatus.h>

/*!
 * \file ARAccelerator.h
 * \brief The Augmented Reality Acceleration client API specification.
 */

namespace tiarx {

class ARXFlatBufferMgr;
class ARXImageBufferMgr;

/*!
 * \brief The callback interface used by ARAccelerator to notify clients about property changes.
 */
class ARXPropertyListener
{
public:
    ARXPropertyListener() {};
    virtual ~ARXPropertyListener() {};

    /*!
     * Callback received when a property changes value.
     * 
     */
    virtual void onPropertyChanged(uint32_t property, int32_t value) = 0;

};

/*!
 * \brief The API for ARX framework.
 */

class ARAccelerator
{
public:

    /*!
     * Creates an instance of ARAccelerator.
     * Returns NULL if it cannot connect to the ARX daemon.
     */
    static ARAccelerator* create(ARXPropertyListener *listener);

    /*!
     * Creates an instance of ARAccelerator.
     * @param listener The listener where the client will receive callbacks when properties change value
     * @param useDVP If true, a full DVP handle will be created which can then be obtained through getDVPHandle.
     * Additionally, the image and flat buffers received from ARX will be usable with DVP.
     * Returns NULL if it cannot connect to the ARX daemon.
     */
    static ARAccelerator* create(ARXPropertyListener *listener, bool useDVP);

    /*!
     * Destroys the object and releases any resources acquired during create().
     * The object is dead after destroy returns
     * @see create()
     */
    virtual void destroy() = 0;

    /*!
     * Loads the specified AR Engine module into the ARXDaemon.
     */
    virtual arxstatus_t loadEngine(const char *modname) = 0;

    /*!
     * Gets the value for the specified property ID.
     * @param prop The property to query
     * @param value The output value
     * @return INVALID_PROPERTY if the property doesn't exist
     * @see setProperty()
     */
    virtual arxstatus_t getProperty(uint32_t prop, int32_t *value) = 0;

    /*!
     * Sets the value for the specified property ID.
     * @param prop The property to set
     * @param value The value to set prop to
     * @return INVALID_PROPERTY if the property doesn't exist
     * @return INVALID_VALUE if the value is not appropiate for the property
     * @see getProperty()
     */
    virtual arxstatus_t setProperty(uint32_t prop, int32_t value) = 0;

    /*!
     * Gets the flat buffer Manager for the requested ID. These type of managers
     * only handle generic buffers such as face detect information.
     * @param bufID the Buffer ID to query for
     * @return the buffer manager corresponding to the buffer ID
     * @see getImageBufferMgr()
     */
    virtual ARXFlatBufferMgr *getFlatBufferMgr(uint32_t bufID) = 0;

    /*!
     * Gets the image buffer Manager for the requested ID.
     * @param bufID the Buffer ID to query for
     * @return the buffer manager corresponding to the buffer ID
     * @see getFlatBufferMgr()
    */
   virtual ARXImageBufferMgr *getImageBufferMgr(uint32_t bufID) = 0;

   virtual DVP_Handle getDVPHandle() = 0;

protected:
    /*!
     * Non-public constructor.
     * Users should call create instead
     * @see create()
     */
    ARAccelerator() {};
    /*!
     * Non-public copy constructor.
     * Users should call create instead
     * @see create()
     */
    ARAccelerator(const ARAccelerator&);
    /*!
     * Non-public assignment operator.
     * Users should call create to obtain a proper instance
     * @see create()
     */
    ARAccelerator& operator=(const ARAccelerator&);
    /*!
     * Non-public destructor.
     * Users should call destroy to properly release the instance given by create()
     * @see destroy()
     * @see create()
     */
    virtual ~ARAccelerator() {};
};

}

#endif //_ARACCELERATOR_H_
