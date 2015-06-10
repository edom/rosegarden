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


#ifndef RG_LILYPOND_SEGMENTS_CONTEXT_H
#define RG_LILYPOND_SEGMENTS_CONTEXT_H

/*
 * LilyPondSegmentsContext.h
 *
 * This file defines a class which maintains the context of the segments
 * which have to be exported to LilyPond.
 *
 * This context is used to 
 *      - See when a repeating segment may be printed inside
 *        repeat bars. (i.e. when no other unrepeating segment
 *        coexists at the same time on an other track).
 *      - Find out which linked segments may be printed as repeat with volta.
 *      - Compute the time offset which have to be set in LilyPond for
 *        each segment (with keyword \skip).
 */


#include "base/NotationTypes.h"

#include <set>
#include <map>
#include <list>
#include <string>


namespace Rosegarden
{

class Composition;
class Track;
class Segment;
class LilyPondExporter;

class LilyPondSegmentsContext
{

public:
    /**
     * Create an empty segment context
     */
    LilyPondSegmentsContext(LilyPondExporter *exporter, Composition *composition);

    ~LilyPondSegmentsContext();

    /**
     * Add a segment to the context
     */
    void addSegment(Segment *segment);

    /**
     * Return true if the segment context is empty
     */
    bool containsNoSegment();

    /**
     * Walk through all segments, find the repeating ones and compute
     * their start times in the LilyPond score assuming they are synchronous.
     */
    void precompute();

    /**
     * Walk through all segments and fix their start times when some repeating
     * segments are printed with repeat bars in the LilyPond score.
     */
    void fixRepeatStartTimes();

    /**
     * Walk through all segments and fix their start times when some linked
     * segments are printed in the LilyPond score as repeat with volta.
     */
    void fixVoltaStartTimes();

    /**
     * Return the smaller start time of the segments being exported.
     * Only valid after precompute() has been executed.
     */
    timeT getFirstSegmentStartTime() { return m_firstSegmentStartTime; }

    /**
     * Return the smaller start time of the segments being exported.
     * Only valid after precompute() has been executed.
     */
    timeT getLastSegmentEndTime() { return m_lastSegmentEndTime; }

    /**
     * Prepare to get the segments on the first track.
     * Return null if there is no track.
     */
    Track * useFirstTrack();

    /**
     * Go to the next track
     * Return null if there is no more track.
     */
    Track * useNextTrack();

    /**
     * Return the position of the current track or -1
     */
    int getTrackPos();

    /**
     * Prepare to get the segments on the current track.
     * Return null if there is no segment on the current track.
     */
    Segment * useFirstSegment();

    /**
     * Go to the next segment.
     * Return null if there is no more segment on the current track.
     */
    Segment * useNextSegment();

    /**
     * Return the start time of the current segment in LilyPond
     */
    timeT getSegmentStartTime();

    /**
     * Return how many time the current segment is repeated in LilyPond
     */
    int getNumberOfRepeats();

    /**
     * Return true if the segment is repeated
     */
    bool isRepeated();

    /**
     * Return true if the segment is inside a "repeat with volta" chain
     */
    bool isRepeatWithVolta();

    /**
     * Return true if the segment is synchronous
     */
    bool isSynchronous();

    /**
     * Return true if the segment is a volta
     */
    bool isVolta();

    /**
     * Return true if the segment is the last volta of a chain
     */
    bool isLastVolta();

    /**
     * Return the text of the current volta.
     */
    std::string getVoltaText();

    /// Only for debug
    void dump();

protected :

    /**
     * Look in the specified track for linked segments which may be
     * exported as repeat with volta and mark them accordingly.
     */
    void lookForRepeatedLinks(int trackId);


private :

    typedef std::list<Segment *> SegmentList;   /// CURRENTLY NOT USED
  
    struct Volta {
        Segment * segment;
        timeT duration;
        std::set<int> voltaNumber;

        Volta(Segment * seg, timeT voltaDuration, int number)
        {
            segment = seg;
            duration = voltaDuration;
            voltaNumber.insert(number);
        }
    };
    
    typedef std::vector<Volta *> VoltaChain;

    struct SegmentData
    {
        Segment * segment;

        mutable timeT duration;               // Duration without repeat
        mutable timeT wholeDuration;          // Duration with repeat
        mutable int numberOfRepeats;          // 0 if not repeating
        mutable timeT remainderDuration;

        mutable bool synchronous;             // Multitrack repeat is possible
        mutable bool noRepeat;                // Repeat is forbidden
        mutable int repeatId;                 // Identify a repeat chain
        mutable int numberOfRepeatLinks;      // How many repeat in a chain

        mutable bool startOfRepeatChain;
        mutable bool volta;                   // Mark a volta
        mutable bool ignored;                 // Mark a segment inserted
                                              // in a repeat chain.
        mutable VoltaChain * rawVoltaChain;
        mutable VoltaChain * sortedVoltaChain;

        mutable timeT startTime;              // In LilyPond output
        mutable timeT endTime;                // In LilyPond output

        SegmentData(Segment * seg)
        {
            segment = seg;
            duration = 0;
            wholeDuration = 0;
            numberOfRepeats = 0;
            remainderDuration = 0;
            synchronous = true;
            noRepeat = false;
            repeatId = 0;
            numberOfRepeatLinks = 0;
            startOfRepeatChain = false;
            volta = false;
            ignored = false;
            rawVoltaChain = 0;
            sortedVoltaChain = 0;
            startTime = 0;
            endTime = 0;
        }
    };

    struct SegmentDataCmp {
        bool operator()(const SegmentData &s1, const SegmentData &s2) const;
    };
    typedef std::multiset<SegmentData, LilyPondSegmentsContext::SegmentDataCmp> SegmentSet;
    typedef std::map<int, SegmentSet> TrackMap;

    typedef std::list<const SegmentData *> SegmentDataList;


   /**
    * Begin to look on all tracks for all segments synchronous of the given one.
    * Return null if no segment found.
    */
    const SegmentData * getFirstSynchronousSegment(Segment * seg);

   /**
    * Get the next segment synchronous of the one passed as argument of
    * the last call of getFirstSynchronousSegment().
    * Return null if no more segment found.
    */
    const SegmentData * getNextSynchronousSegment();

   /**
    * Look for similar segments in the raw volta chain (on all tracks
    * simultaneously) and fill the sorted volta chain accordingly.
    * The argument is the list of the associated synchronous main repeat
    * segment data from all the tracks.
    */
    void sortAndGatherVolta(SegmentDataList &);

    TrackMap m_segments;

    LilyPondExporter * m_exporter;
    Composition * m_composition;
    
    timeT m_epsilon;
    timeT m_firstSegmentStartTime;
    timeT m_lastSegmentEndTime;

    TrackMap::iterator m_trackIterator;
    SegmentSet::iterator m_segIterator;
    VoltaChain::iterator m_voltaIterator;

    int m_nextRepeatId;

    // Used by "Get Synchronous Segment" methods getFirstSynchronousSegment()
    // and getNextSynchronousSegment()
    Segment * m_GSSSegment;
    TrackMap::iterator m_GSSTrackIterator;
    SegmentSet::iterator m_GSSSegIterator;

    bool m_repeatWithVolta;
    VoltaChain * m_currentVoltaChain;
    bool m_lastVolta;

};


}

#endif // RG_LILYPOND_SEGMENTS_CONTEXT_H
