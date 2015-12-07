/*****************************************************************************
 * Copyright (C) 2013 x265 project
 *
 * Authors: Chung Shin Yee <shinyee@multicorewareinc.com>
 *          Min Chen <chenm003@163.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at license @ x265.com.
 *****************************************************************************/

#ifndef X265_FRAMEFILTER_H
#define X265_FRAMEFILTER_H

#include "common.h"
#include "frame.h"
#include "deblock.h"
#include "sao.h"
#include "threadpool.h" // class BondedTaskGroup

namespace X265_NS {
// private x265 namespace

class Encoder;
class Entropy;
class FrameEncoder;
struct ThreadLocalData;

// Manages the processing of a single frame loopfilter
class FrameFilter
{
public:

    x265_param*   m_param;
    Frame*        m_frame;
    FrameEncoder* m_frameEncoder;
    int           m_hChromaShift;
    int           m_vChromaShift;
    int           m_pad[2];

    int           m_numRows;
    int           m_saoRowDelay;
    int           m_lastHeight;
    
    void*         m_ssimBuf; /* Temp storage for ssim computation */

#define MAX_PFILTER_CUS     (4) /* maximum CUs for every thread */
    class ParallelFilter : public BondedTaskGroup, public Deblock
    {
    public:
        static uint32_t     numCols;
        uint32_t            m_rowAddr;
        FrameEncoder*       m_frameEncoder;
        FrameData*          m_encData;
        SAO                 m_sao;
        ThreadSafeInteger   m_lastCol;          /* The column that next to process */
        ThreadSafeInteger   m_allowedCol;       /* The column that processed from Encode pipeline */

        ParallelFilter()
            : m_rowAddr(0)
            , m_frameEncoder(NULL)
            , m_encData(NULL)
        {
        }

        ~ParallelFilter()
        { }

        void processTasks(int workerThreadId);

    protected:

        ParallelFilter operator=(const ParallelFilter&);
    };

    ParallelFilter*     m_parallelFilter;

    FrameFilter()
        : m_param(NULL)
        , m_frame(NULL)
        , m_frameEncoder(NULL)
        , m_ssimBuf(NULL)
        , m_parallelFilter(NULL)
    {
    }

    void init(Encoder *top, FrameEncoder *frame, int numRows, uint32_t numCols);
    void destroy();

    void start(Frame *pic, Entropy& initState, int qp);

    void processRow(int row);
    void processRowPost(int row);
    void processSao(int row);
    uint32_t getCUHeight(int rowNum) const;
};
}

#endif // ifndef X265_FRAMEFILTER_H
