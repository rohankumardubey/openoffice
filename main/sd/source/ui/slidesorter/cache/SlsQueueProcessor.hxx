/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



#ifndef SD_SLIDESORTER_QUEUE_PROCESSOR_HXX
#define SD_SLIDESORTER_QUEUE_PROCESSOR_HXX

#include "cache/SlsPageCache.hxx"
#include "SlsRequestPriorityClass.hxx"
#include "SlsBitmapFactory.hxx"
#include "view/SlideSorterView.hxx"
#include "tools/IdleDetection.hxx"
#include "SlsBitmapCache.hxx"
#include "sdpage.hxx"
#include "Window.hxx"
 
#include <svx/svdpagv.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <boost/function.hpp>


namespace sd { namespace slidesorter { namespace view {
class SlideSorterView;
} } }



namespace sd { namespace slidesorter { namespace cache {

class BitmapCache;
class RequestQueue;



/** This queue processor is timer based, i.e. when an entry is added to the
    queue and the processor is started with Start() in the base class a
    timer is started that eventually calls ProcessRequest().  This is
    repeated until the queue is empty or Stop() is called.
*/
class QueueProcessor 
{
public:
    typedef ::boost::function<bool()> IdleDetectionCallback;
    QueueProcessor (
        RequestQueue& rQueue, 
        const ::boost::shared_ptr<BitmapCache>& rpCache,
        const Size& rPreviewSize,
        const bool bDoSuperSampling,
        const SharedCacheContext& rpCacheContext);
	virtual ~QueueProcessor();

    /** Start the processor.  This implementation is timer based and waits
        an defined amount of time that depends on the given argument before
        the next entry in the queue is processed.
        @param nPriorityClass
            A priority class of 0 tells the processor that a high priority
            request is waiting in the queue.  The time to wait is thus
            shorter then that for a low priority request (denoted by a value
            of 1.)  When the timer is already running it is not modified.
    */
    void Start (int nPriorityClass = 0);
    void Stop (void);
    void Pause (void);
    void Resume (void);

    void Terminate (void);

    void SetPreviewSize (
        const Size& rSize,
        const bool bDoSuperSampling);

    /** As we can not really terminate the rendering of a preview bitmap for
        a request in midair this method acts more like a semaphor.  It
        returns only when it is save for the caller to delete the request.
        For this to work it is important to remove the request from the
        queue before calling this method.
    */
    void RemoveRequest (CacheKey aKey);

    /** Use this method when the page cache is (maybe) using a different
        BitmapCache.  This is usually necessary after calling
        PageCacheManager::ChangeSize().
    */
    void SetBitmapCache (const ::boost::shared_ptr<BitmapCache>& rpCache);

private:
    /** This mutex is used to guard the queue processor.  Be carefull not to
        mix its use with that of the solar mutex.
    */
    ::osl::Mutex maMutex;

    Timer maTimer;
    DECL_LINK(ProcessRequestHdl, Timer*);
    sal_uInt32 mnTimeBetweenHighPriorityRequests;
    sal_uInt32 mnTimeBetweenLowPriorityRequests;
    sal_uInt32 mnTimeBetweenRequestsWhenNotIdle;
    Size maPreviewSize;
    bool mbDoSuperSampling;
    SharedCacheContext mpCacheContext;
    RequestQueue& mrQueue;
    ::boost::shared_ptr<BitmapCache> mpCache;
    BitmapFactory maBitmapFactory;
    bool mbIsPaused;

    void ProcessRequests (void);
    void ProcessOneRequest (
        CacheKey aKey,
        const RequestPriorityClass ePriorityClass);
};




} } } // end of namespace ::sd::slidesorter::cache

#endif
