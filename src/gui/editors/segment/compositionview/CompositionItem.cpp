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


#include "CompositionItem.h"

#include "SegmentRect.h"
#include "base/SnapGrid.h"

#include <QRect>

#include <math.h>


namespace Rosegarden
{


CompositionItem::CompositionItem(Segment &s, const SegmentRect &rect)
        : m_segment(s),
        m_rect(rect),
        m_z(0)
{}

QRect CompositionItem::rect() const
{
    QRect res = m_rect;

    // For repeating segments, use the base width
    if (m_rect.isRepeating()) {
        res.setWidth(m_rect.getBaseWidth());
    }

    return res;
}

timeT CompositionItem::getRepeatTimeAt(const SnapGrid &grid, const QPoint &pos)
{
    timeT startTime = m_segment.getStartTime();
    timeT repeatInterval = m_segment.getEndMarkerTime() - startTime;

    int repeatWidth = int(nearbyint(grid.getRulerScale()->getXForTime(repeatInterval)));

    int count = (pos.x() - rect().x()) / repeatWidth;

    // Let the caller know that the position was not within a repeat.
    if (count == 0)
        return 0;

    return startTime + count * repeatInterval;
}


}
