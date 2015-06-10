/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2015 the Rosegarden development team.
 
    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/


#include "SegmentMapper.h"
#include "misc/Debug.h"

#include "misc/Strings.h"
#include "base/BaseProperties.h"
#include "base/Segment.h"
#include "document/RosegardenDocument.h"
#include "sound/ControlBlock.h"

namespace Rosegarden
{

SegmentMapper::SegmentMapper(RosegardenDocument *doc,
                             Segment *segment) :
    MappedEventBuffer(doc),
    m_segment(segment)
{
    SEQMAN_DEBUG << "SegmentMapper : " << this << endl;
}


void
SegmentMapper::
initSpecial(void)
{
    if (m_segment != 0) {
        SEQMAN_DEBUG << "SegmentMapper::initSpecial " 
                     << " for segment " << m_segment->getLabel() << endl;
    }    
};


SegmentMapper::~SegmentMapper()
{
    SEQMAN_DEBUG << "~SegmentMapper : " << this << endl;
}

int
SegmentMapper::getSegmentRepeatCount()
{
    int repeatCount = 0;

    timeT segmentStartTime = m_segment->getStartTime();
    timeT segmentEndTime = m_segment->getEndMarkerTime();
    timeT segmentDuration = segmentEndTime - segmentStartTime;
    timeT repeatEndTime = segmentEndTime;

    if (m_segment->isRepeating() && segmentDuration > 0) {
        repeatEndTime = m_segment->getRepeatEndTime();
        repeatCount = 1 + (repeatEndTime - segmentEndTime) / segmentDuration;
    }

    return repeatCount;
}

bool
SegmentMapper::
mutedEtc(void)
{
    TrackId trackId = m_segment->getTrack();

    // If we're soloing (i.e. playing only a selected track), that
    // overrides the usual muting logic.  We're effectively muted if
    // the solo track isn't ours.
    if (ControlBlock::getInstance()->isSolo()) {
        return (trackId != ControlBlock::getInstance()->getSelectedTrack());
    }

    // Otherwise use the normal muting logic.
    return ControlBlock::getInstance()->isTrackMuted(trackId);
}

}
