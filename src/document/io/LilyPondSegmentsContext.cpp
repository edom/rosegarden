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



#include "LilyPondSegmentsContext.h"
#include "base/Composition.h"
#include "base/Segment.h"

#include <map>
#include <set>
#include <sstream>


namespace Rosegarden
{


LilyPondSegmentsContext::LilyPondSegmentsContext(LilyPondExporter *exporter,
                                                 Composition *composition) :
    m_exporter(exporter),
    m_composition(composition),
    m_firstSegmentStartTime(0),
    m_lastSegmentEndTime(0),
    m_nextRepeatId(1),
    m_repeatWithVolta(false),
    m_currentVoltaChain(0),
    m_lastVolta(false)
{
    m_segments.clear();
    m_epsilon = Note(Note::Hemidemisemiquaver).getDuration() / 2;

}

LilyPondSegmentsContext::~LilyPondSegmentsContext()
{
    TrackMap::iterator tit;
    SegmentSet::iterator sit;

    for (tit = m_segments.begin(); tit != m_segments.end(); ++tit) {
        for (sit = tit->second.begin(); sit != tit->second.end(); ++sit) {
            if (sit->rawVoltaChain) {
                VoltaChain::iterator i;
                for (i = sit->rawVoltaChain->begin(); i != sit->rawVoltaChain->end(); ++i) {
                    delete *i;
                }
                delete sit->rawVoltaChain;
                delete sit->sortedVoltaChain;
            }
        }
    }
}

void
LilyPondSegmentsContext::addSegment(Segment *segment)
{
    int trackPos = m_composition->getTrackPositionById(segment->getTrack());
    m_segments[trackPos].insert(SegmentData(segment));
}

bool
LilyPondSegmentsContext::containsNoSegment()
{
    return m_segments.size() == 0;
}

void
LilyPondSegmentsContext::precompute()
{
    TrackMap::iterator tit;
    SegmentSet::iterator sit;

    // Find the start time of the first segment and the end time of the
    // last segment.
    m_firstSegmentStartTime = m_composition->getEndMarker();
    m_lastSegmentEndTime = m_composition->getStartMarker();
    for (tit = m_segments.begin(); tit != m_segments.end(); ++tit) {
        sit = tit->second.begin();
        if (sit != tit->second.end()) {
            timeT start = sit->segment->getStartTime();
            if (start < m_firstSegmentStartTime) m_firstSegmentStartTime = start;
            timeT end = sit->segment->getEndMarkerTime();
            if (end > m_lastSegmentEndTime) m_lastSegmentEndTime = end;
        }
    }

    // Compute the duration and repeat count of each segment
    for (tit = m_segments.begin(); tit != m_segments.end(); ++tit) {
        /* int trackPos = tit->first; */
        /* Track * track = m_composition->getTrackByPosition(trackPos); */
        SegmentSet &segSet = tit->second;
        for (sit = segSet.begin(); sit != segSet.end(); ++sit) {
            Segment * seg = sit->segment;
            sit->duration = seg->getEndMarkerTime() - seg->getStartTime();
            if (!seg->isRepeating()) {
                sit->wholeDuration = sit->duration;
                sit->numberOfRepeats = 0;
                sit->remainderDuration = 0;
            } else {
                SegmentSet::iterator next = sit;
                ++next;
                timeT endOfRepeat;
                if (next == segSet.end()) {
                    endOfRepeat = m_composition->getEndMarker();
                } else {
                    endOfRepeat = (*next).segment->getStartTime();
                }
                sit->wholeDuration = endOfRepeat - seg->getStartTime();
                sit->numberOfRepeats = sit->wholeDuration / sit->duration;
                sit->remainderDuration = sit->wholeDuration % sit->duration;
                if (sit->remainderDuration < m_epsilon) {
                    sit->remainderDuration = 0;
                }
            }
        }
    }



    // Look for synchronous segments.
    // A synchronous segment has no other segment starting or ending while
    // it is running. When a segment is not synchronous, the other segments
    // whose time range overlaps its own time range can't be synchronous. 
    // Only synchronous segments may be shown with repeat in LilyPond.
    
    // For each segment, look at all the other ones and clear the synchronous
    // flag when the segments can't be printed out with repeat bars.
    for (tit = m_segments.begin(); tit != m_segments.end(); ++tit) {
        /* int trackPos = tit->first; */
        /* Track * track = m_composition->getTrackByPosition(trackPos); */
        SegmentSet &segSet = tit->second;
        for (sit = segSet.begin(); sit != segSet.end(); ++sit) {
            Segment * seg = sit->segment;
            timeT start = seg->getStartTime();
            timeT end = start + sit->wholeDuration;

            TrackMap::iterator tit2;
            SegmentSet::iterator sit2;
            for (tit2 = m_segments.begin(); tit2 != m_segments.end(); ++tit2) {
                /* int trackPos2 = tit2->first; */
                /* Track * track2 = m_composition->getTrackByPosition(trackPos2); */
                SegmentSet &segSet2 = tit2->second;
                for (sit2 = segSet2.begin(); sit2 != segSet2.end(); ++sit2) {
                    Segment * seg2 = sit2->segment;
                    if (seg == seg2) continue;
                    timeT start2 = seg2->getStartTime();
                    timeT end2 = start2 + sit2->wholeDuration;

                    // When the two segments have the same bounds,
                    // repeat is possible.
                    if ((start2 == start) && (end2 == end)
                        && (sit->duration == sit2->duration)) continue;

                    // If the second segment is starting somewhere inside
                    // the first one, repeat is neither possible for the first
                    // segment nor for the second one.
                    if ((start2 >= start) && (start2 < end)) {
                        sit->synchronous = false;
                        sit2->synchronous = false;
                    }
                }
            }
        }
    }


    // Look for linked segments which may be exported as repeat with volta
    for (tit = m_segments.begin(); tit != m_segments.end(); ++tit) {
        int trackPos = tit->first;
        lookForRepeatedLinks(trackPos);
    }

    // Check linked segment repeat consistency between tracks and mark
    // inconsistant repeats
    for (tit = m_segments.begin(); tit != m_segments.end(); ++tit) {
        for (sit = tit->second.begin(); sit != tit->second.end(); ++sit) {
            if (sit->repeatId) {
                const SegmentData * sd;
                for (sd = getFirstSynchronousSegment(sit->segment);
                             sd; sd = getNextSynchronousSegment()) {
                    if (!sd->repeatId) {
                        sit->noRepeat = true;
                        break;
                    }
                }
            }
        }
    }

    // Reset all the repeatId
    for (tit = m_segments.begin(); tit != m_segments.end(); ++tit) {
        for (sit = tit->second.begin(); sit != tit->second.end(); ++sit) {
            sit->repeatId = 0;
            sit->volta = false;
            sit->ignored = false;
        }
    }


    // Then look again for repeats from linked segments
    // (without looking at the inconsistant ones)
    for (tit = m_segments.begin(); tit != m_segments.end(); ++tit) {
        int trackPos = tit->first;
        lookForRepeatedLinks(trackPos);
    }

    // On each track, store the volta of each reapeat sequence
    // inside the main segment data
    int currentRepeatId = 0;
    const SegmentData * currentMainSeg = 0;
    for (tit = m_segments.begin(); tit != m_segments.end(); ++tit) {
        for (sit = tit->second.begin(); sit != tit->second.end(); ++sit) {
            if (sit->repeatId) {
                if (sit->repeatId != currentRepeatId) {
                    currentRepeatId = sit->repeatId;
                    currentMainSeg = &(*sit);
                    currentMainSeg->numberOfRepeatLinks = 1;
                    currentMainSeg->rawVoltaChain = new VoltaChain;
                } else {
                    // Main repeating segment or volta ?
                    if (sit->volta) {
                        // Insert volta in list
                        Volta * volta = new Volta(sit->segment,
                                                  sit->duration,
                                                  currentMainSeg->numberOfRepeatLinks);
                        currentMainSeg->rawVoltaChain->push_back(volta);
                    } else {
                        // Count more one repeat
                        currentMainSeg->numberOfRepeatLinks++;
                    }
                }
            }
        }
    }


    // Sort the volta in the print order and gather the duplicate ones

    // Look for the repeat sequences in the first track
    tit = m_segments.begin();
    if (tit == m_segments.end()) return;   // This should not happen
    for (sit = tit->second.begin(); sit != tit->second.end(); ++sit) {
        if (sit->rawVoltaChain) {
            SegmentDataList repeatList;
            repeatList.clear();
            const SegmentData * sd = &(*sit);
            repeatList.push_back(sd);

            // Gather data from the other tracks
            for (sd = getFirstSynchronousSegment(sit->segment); sd;
                 sd = getNextSynchronousSegment()) {
                repeatList.push_back(sd);
            }

            // Sort the volta
            sortAndGatherVolta(repeatList);
        }
    }

    // Compute the LilyPond start times with all segments unfolded.
    for (tit = m_segments.begin(); tit != m_segments.end(); ++tit) {
        /* int trackPos = tit->first; */
        /* Track * track = m_composition->getTrackByPosition(trackPos); */
        SegmentSet &segSet = tit->second;
        for (sit = segSet.begin(); sit != segSet.end(); ++sit) {
            Segment * seg = sit->segment;
            sit->startTime = seg->getStartTime() - m_firstSegmentStartTime;
        }
    }
}

void
LilyPondSegmentsContext::fixRepeatStartTimes()
{
    TrackMap::iterator tit;
    SegmentSet::iterator sit;

    // precompute() should have been called already and
    // we know what segment may be repeated and what segment may be unfolded.
    // We can compute the start time of each segment in the LilyPond score.

    // Sort the repeating segments into start times
    std::map<timeT, const SegmentData *> repeatedSegments;
    repeatedSegments.clear();
    for (tit = m_segments.begin(); tit != m_segments.end(); ++tit) {
        SegmentSet &segSet = tit->second;
        for (sit = segSet.begin(); sit != segSet.end(); ++sit) {
            if (sit->numberOfRepeats && sit->synchronous) {
                repeatedSegments[sit->startTime] = &(*sit);
            }
        }
    }

    // Then fix all the start times
    std::map<timeT, const SegmentData *>::reverse_iterator it;
    for (it=repeatedSegments.rbegin(); it!=repeatedSegments.rend(); ++it) {
        const SegmentData *segData = it->second;
        timeT deltaT = segData->wholeDuration - segData->duration;
        for (tit = m_segments.begin(); tit != m_segments.end(); ++tit) {
            SegmentSet &segSet = tit->second;
            for (sit = segSet.begin(); sit != segSet.end(); ++sit) {
                if (sit->startTime > it->first) {
                    sit->startTime -= deltaT;
                }
            }
        }
        // Fix the end of composition time
        m_lastSegmentEndTime -= deltaT;
    }
}


void
LilyPondSegmentsContext::fixVoltaStartTimes()
{
    TrackMap::iterator tit;
    SegmentSet::iterator sit;

    // precompute() should have been called already and
    // we know what segment may be repeated and what segment may be unfolded.
    // We can compute the start time of each segment in the LilyPond score.

    // Validate the output of repeat with volta in LilyPond score
    m_repeatWithVolta = true;

    // Sort the repeat/volta sequences into start times
    std::map<timeT, const SegmentData *> repeatedSegments;
    repeatedSegments.clear();
    for (tit = m_segments.begin(); tit != m_segments.end(); ++tit) {
        SegmentSet &segSet = tit->second;
        for (sit = segSet.begin(); sit != segSet.end(); ++sit) {
            if (sit->numberOfRepeatLinks) {
                repeatedSegments[sit->startTime] = &(*sit);
            }
        }
    }

    // Then fix all the start times
    std::map<timeT, const SegmentData *>::reverse_iterator it;
    for (it=repeatedSegments.rbegin(); it!=repeatedSegments.rend(); ++it) {
        const SegmentData *segData = it->second;

        // Compute the duration error
        timeT duration = segData->duration;
        timeT wholeDuration = duration * segData->numberOfRepeatLinks;
        VoltaChain::iterator vci;
        for (vci = segData->sortedVoltaChain->begin();
             vci != segData->sortedVoltaChain->end(); ++vci) {
            wholeDuration += (*vci)->duration * (*vci)->voltaNumber.size();
            duration += (*vci)->duration;
        }
        timeT deltaT = wholeDuration - duration;

        // Fix the segment start time when needed
        for (tit = m_segments.begin(); tit != m_segments.end(); ++tit) {
            SegmentSet &segSet = tit->second;
            for (sit = segSet.begin(); sit != segSet.end(); ++sit) {
                if (sit->startTime > it->first) {
                    sit->startTime -= deltaT;
                }
            }
        }
        // Fix the end of composition time
        m_lastSegmentEndTime -= deltaT;
    }
}

Track *
LilyPondSegmentsContext::useFirstTrack()
{
    m_trackIterator = m_segments.begin();
    if (m_trackIterator == m_segments.end()) return 0;
    return m_composition->getTrackByPosition((*m_trackIterator).first);
}

Track *
LilyPondSegmentsContext::useNextTrack()
{
    ++m_trackIterator;
    if (m_trackIterator == m_segments.end()) return 0;
    return m_composition->getTrackByPosition((*m_trackIterator).first);
}

int
LilyPondSegmentsContext::getTrackPos()
{
    if (m_trackIterator == m_segments.end()) return -1;
    return (*m_trackIterator).first;
}

Segment *
LilyPondSegmentsContext::useFirstSegment()
{
    m_lastVolta = false;
    m_segIterator = (*m_trackIterator).second.begin();
    if (m_segIterator == (*m_trackIterator).second.end()) return 0;
    if (m_repeatWithVolta && (*m_segIterator).ignored) return useNextSegment();
    return (*m_segIterator).segment;
}

Segment *
LilyPondSegmentsContext::useNextSegment()
{
    if (m_repeatWithVolta) {
        // Process possible volta segment
        if (m_segIterator->numberOfRepeatLinks) {
            if (!m_currentVoltaChain) {
                m_currentVoltaChain = m_segIterator->sortedVoltaChain;
                m_voltaIterator = m_currentVoltaChain->begin();
                if (m_voltaIterator != m_currentVoltaChain->end()) {
                    if (m_currentVoltaChain->size() == 1) m_lastVolta = true;
                    return (*m_voltaIterator)->segment;
                }
            } else {
                ++m_voltaIterator;
                if (m_voltaIterator != m_currentVoltaChain->end()) {
                    VoltaChain::iterator nextIt = m_voltaIterator;
                    ++nextIt;
                    if (nextIt == m_currentVoltaChain->end()) {
                        m_lastVolta = true;
                    }
                    return (*m_voltaIterator)->segment;
                } else {
                    m_lastVolta = false;
                    m_currentVoltaChain = 0;
                }
            }
        }
    }

    ++m_segIterator;
    if (m_segIterator == (*m_trackIterator).second.end()) return 0;
    if (m_repeatWithVolta && (*m_segIterator).ignored) return useNextSegment();
    return (*m_segIterator).segment;
}

timeT
LilyPondSegmentsContext::getSegmentStartTime()
{
    return (*m_segIterator).startTime;
}

int
LilyPondSegmentsContext::getNumberOfRepeats()
{
    if (m_repeatWithVolta && (*m_segIterator).numberOfRepeatLinks) {
        return (*m_segIterator).numberOfRepeatLinks;
    } else {
        return (*m_segIterator).numberOfRepeats;
    }
}

bool
LilyPondSegmentsContext::isRepeated()
{
    return (*m_segIterator).numberOfRepeats;
}

bool
LilyPondSegmentsContext::isRepeatWithVolta()
{
    if (m_repeatWithVolta) {
        return (*m_segIterator).numberOfRepeatLinks;
    } else {
        return false;
    }
}

bool
LilyPondSegmentsContext::isSynchronous()
{
    return (*m_segIterator).synchronous;
}

bool
LilyPondSegmentsContext::isVolta()
{
    return m_currentVoltaChain;
}

bool
LilyPondSegmentsContext::isLastVolta()
{
    return m_lastVolta;
}

std::string
LilyPondSegmentsContext::getVoltaText()
{
/// TODO : Modification needed to deal with the new "volta as ordinary segment" mode

    if (!(*m_segIterator).sortedVoltaChain) return std::string("");
    ++m_voltaIterator;
    if (m_voltaIterator == (*m_segIterator).sortedVoltaChain->end()) return std::string("");
    
    std::stringstream out;
    std::set<int>::iterator it;
    bool firstTime = true;
    for (it = (*m_voltaIterator)->voltaNumber.begin();
         it != (*m_voltaIterator)->voltaNumber.end(); ++it) {
        if (firstTime) {
            firstTime = false;
        } else {
            out << ", ";
        }
        out << *it;
    }

    return std::string(out.str());
}

bool
LilyPondSegmentsContext::SegmentDataCmp::operator()(const SegmentData &s1, const SegmentData &s2) const
{
    // Sort segments according to start time, then end time, then address.
    // Copied from StaffHeader::SegmentCmp::operator()
    if (s1.segment->getStartTime() < s2.segment->getStartTime()) return true;
    if (s1.segment->getStartTime() > s2.segment->getStartTime()) return false;
    if (s1.segment->getEndMarkerTime() < s2.segment->getEndMarkerTime()) return true;
    if (s1.segment->getEndMarkerTime() > s2.segment->getEndMarkerTime()) return false;
    return (long) s1.segment < (long) s2.segment;
}

const LilyPondSegmentsContext::SegmentData *
LilyPondSegmentsContext::getFirstSynchronousSegment(Segment * seg)
{
    m_GSSSegment = seg;

    m_GSSTrackIterator = m_segments.begin();
    if (m_GSSTrackIterator == m_segments.end()) return 0;

    m_GSSSegIterator = m_GSSTrackIterator->second.begin();
    if (m_GSSSegIterator == m_GSSTrackIterator->second.end()) return 0;

    if (m_GSSSegIterator->synchronous &&
        (m_GSSSegIterator->segment != m_GSSSegment) &&
        (m_GSSSegIterator->segment->getStartTime() == m_GSSSegment->getStartTime()) &&
        (m_GSSSegIterator->segment != m_GSSSegment)) {
            return &(*m_GSSSegIterator);
    }

    return getNextSynchronousSegment();
}

const LilyPondSegmentsContext::SegmentData *
LilyPondSegmentsContext::getNextSynchronousSegment()
{
    for (;;) {
        ++m_GSSSegIterator;
        if (m_GSSSegIterator == m_GSSTrackIterator->second.end()) {
            ++m_GSSTrackIterator;
            if (m_GSSTrackIterator == m_segments.end()) return 0;
            m_GSSSegIterator = m_GSSTrackIterator->second.begin();
            if (m_GSSSegIterator == m_GSSTrackIterator->second.end()) return 0;
        }

        if (m_GSSSegIterator->synchronous &&
            (m_GSSSegIterator->segment != m_GSSSegment) &&
            (m_GSSSegIterator->segment->getStartTime() == m_GSSSegment->getStartTime()) &&
            (m_GSSSegIterator->segment != m_GSSSegment)) {
                return &(*m_GSSSegIterator);
        }
    }
}

void
LilyPondSegmentsContext::lookForRepeatedLinks(int trackId)
{
    SegmentSet &segSet = m_segments[trackId];

    SegmentSet::iterator sit;
    for (sit = segSet.begin(); sit != segSet.end(); ++sit) {
        // Skip segments already registered in a repeat chain
        if (sit->repeatId) continue;

        // Look for a possible base of a repeat with volta chain
        // Such a base can't be a repeating segment
        if (sit->numberOfRepeats) continue;

        // Such a base must be a synchronous segment
        if (!sit->synchronous) continue;

        // Such a base can't be marked as "no repeat"
        if (sit->noRepeat) continue;

        // Such a base must be a plain linked segment
        Segment * seg = (*sit).segment;
        if (!seg->isPlainlyLinked()) continue;

        // Look for the repetition
        bool repeatFound = false;
        SegmentSet::iterator mainSegIt = sit;
        SegmentSet::iterator sitv = sit;
        SegmentSet::iterator sitm;
        ++sitv;
        for (sitm = sitv; sitv != segSet.end(); ) {
            ++sitm;
            // *sitv is the volta and *sitm the next repetition (if any).

            // Is *sitv a valid volta ?
            // A valid volta is not repeating
            if (sitv->numberOfRepeats) break;

            // A valid volta can't be the repeated segment
            if (sitv->segment->isLinkedTo(sit->segment)) break;

            // A valid volta can't be separated from the repeated
            // segment neither overlap it
            if (    sitv->segment->getStartTime()
                 != mainSegIt->segment->getEndMarkerTime()) break;

            // The size of the volta should not be too large 
            // (But a coda may be an exception...)
            ///!!! TODO Needs ameliorations
            if (sitv->duration >= sit->duration) break;

            // Is a new repetition following the volta ?
            bool again = false;
            if (sitm != segSet.end()) {
                again = true;

                // A repeated segment needs to be synchronous
                if (!sitm->synchronous) again = false;

                // A repeated segment can't be marked "no repeat"
                if (sitm->noRepeat) again = false;

                // A repeated segment must be linked to the original segment
                if (!sitm->segment->isLinkedTo(sit->segment)) again = false;

                // A repeated segment must have the same "bar offset" as
                // the original one.
                        /// XXXXXXXX  TODO TODO TODO !!!!!!!

                // A valid repeated segment can't be separated from the
                // previous volta segment neither overlap it
                if (    sitv->segment->getEndMarkerTime()
                     != sitm->segment->getStartTime())  again = false;
            }

            // Remember the repeating linked segment and the volta
            mainSegIt->repeatId = m_nextRepeatId;
            sitv->repeatId = m_nextRepeatId;
            sitv->volta = true;
            sitv->ignored = true;
            repeatFound = true;

            // Go to the next repetition (if any)
            if (!again) break;
            mainSegIt = sitm;
            sitm->ignored = true;
            ++sitv;
            ++sitv;
            ++sitm;
        }

        if (repeatFound) {
            m_nextRepeatId++;
        }
    }
}

void
LilyPondSegmentsContext::sortAndGatherVolta(SegmentDataList & repeatList)
{
    int idx;
    SegmentDataList::iterator it;
    SegmentDataList::iterator it1 = repeatList.begin();
    if (it1 == repeatList.end()) return;   // This should not happen

    // Initialize the sorted volta chains with the first raw volta
    for (it = repeatList.begin(); it != repeatList.end(); ++it) {
        (*it)->sortedVoltaChain = new VoltaChain;
        (*it)->sortedVoltaChain->push_back((*(*it)->rawVoltaChain)[0]);
    }

    // Add the following volta
    for (idx = 1; idx < (*it1)->numberOfRepeatLinks; idx++) {
        // Is the volta indexed by idx similar to a previous one ?
        bool found = false;
        int idx2;
        for (idx2 = 0; idx2 < idx; idx2++) {
            bool linked = true;
            for (it = repeatList.begin(); it != repeatList.end(); ++it) {
                Segment * seg1 = (*(*it)->rawVoltaChain)[idx]->segment;
                Segment * seg2 = (*(*it)->rawVoltaChain)[idx2]->segment;
                if (!seg1->isPlainlyLinkedTo(seg2)) {
                    linked = false;
                    break;
                }
            }
            if (linked) {
                found = true;
                break;
            }
        }
        if (found) {
            // Add new volta number in existing volta
            for (it = repeatList.begin(); it != repeatList.end(); ++it) {
                (*(*it)->sortedVoltaChain)[idx2]->voltaNumber.insert(idx + 1);   /// Number not needed in rawVoltaChain ???
            }
        } else {
            // Add one more volta
            for (it = repeatList.begin(); it != repeatList.end(); ++it) {
                (*it)->sortedVoltaChain->push_back((*(*it)->rawVoltaChain)[idx]);
            }
        }
    }
}

void
LilyPondSegmentsContext::dump()
{

    TrackMap::iterator tit;
    SegmentSet::iterator sit;

    std::cout << std::endl;
    for (tit = m_segments.begin(); tit != m_segments.end(); ++tit) {
        int trackPos = (*tit).first;
        Track * track = m_composition->getTrackByPosition(trackPos);
        SegmentSet &segSet = (*tit).second;

        std::cout << "Track pos=" << trackPos << " id=" << track->getId()
                  << "   \"" << track->getLabel() << "\"" << std::endl;
        for (sit = segSet.begin(); sit != segSet.end(); ++sit) {
            Segment * seg = (*sit).segment;

            std::cout << "     Segment \"" << seg->getLabel() << "\""
                      << " start=" << seg->getStartTime()
                      << " duration=" << (*sit).duration
                      << " wholeDuration=" <<  (*sit).wholeDuration
                      << std::endl;
            std::cout << "               numRepeat=" << (*sit).numberOfRepeats
                      << " remainder=" << (*sit).remainderDuration
                      << " synchronous=" << (*sit).synchronous
                      << " lilyStart=" << (*sit).startTime
                      << std::endl;
            std::cout << "               noRepeat=" << (*sit).noRepeat
                      << " repeatId=" << (*sit).repeatId
                      << " numberOfRepeatLinks=" << (*sit).numberOfRepeatLinks
                      << " rawVoltaChain=" << (*sit).rawVoltaChain
                      << std::endl;
            if (sit->rawVoltaChain) {
                VoltaChain::iterator i;
                for (i = sit->rawVoltaChain->begin(); i != sit->rawVoltaChain->end(); ++i) {
                    std::cout << "                 --> \"" << (*i)->segment->getLabel()
                              << "\": ";
                    std::set<int>::iterator j;
                    for (j = (*i)->voltaNumber.begin(); j != (*i)->voltaNumber.end(); ++j) {
                        std::cout << (*j) << " ";
                    }
                    std::cout << "\n";
                }
            }
            std::cout << "               sortedVoltaChain=" << (*sit).sortedVoltaChain
                      << " ignored=" << (*sit).ignored
                      << std::endl;
            if (sit->sortedVoltaChain) {
                VoltaChain::iterator i;
                for (i = sit->sortedVoltaChain->begin(); i != sit->sortedVoltaChain->end(); ++i) {
                    std::cout << "                 --> \"" << (*i)->segment->getLabel()
                              << "\": ";
                    std::set<int>::iterator j;
                    for (j = (*i)->voltaNumber.begin(); j != (*i)->voltaNumber.end(); ++j) {
                        std::cout << (*j) << " ";
                    }
                    std::cout << "\n";
                }
            }
        }
    }
    std::cout << std::endl;
}

}

