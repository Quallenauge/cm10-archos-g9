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

#include <binder/IServiceManager.h>
#include <utils/StrongPointer.h>
#include <utils/String16.h>

#include <ipc/IARXDaemon.h>
#include <buffer/IImageBufferMgr.h>
#include <arx/ARXBufferTypes.h>
#include <arx_debug.h>

using namespace android;
using namespace tiarx;

int main()
{
	sp<IARXDaemon> daemon;
	status_t err = getService(String16("ARXDaemon"), &daemon);
	if (err != NO_ERROR) {
		ARX_PRINT(ARX_ZONE_ERROR, "ERROR: failed getting ARXDaemon interface with error=0x%x\n", err);
		return err;
	}

	sp<IImageBufferMgr> mgr;
	arxstatus_t status = daemon->getBufferMgr(BUFF_CAMOUT, &mgr);

	if (mgr != NULL && status == NOERROR) {
	    uint32_t count = mgr->getCount();
	    ARX_PRINT(ARX_ZONE_ALWAYS, "Default count is %d\n", count);
	} else {
	    ARX_PRINT(ARX_ZONE_ERROR, "ERROR: failed getting IImageBufferMgr interface with error=0x%x\n", status);
	}

	daemon->disconnect();

	ARX_PRINT(ARX_ZONE_ALWAYS, "Test done.\n");
}
