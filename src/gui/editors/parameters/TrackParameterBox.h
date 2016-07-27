/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2016 the Rosegarden development team.

    This file is Copyright 2006
        Pedro Lopez-Cabanillas <plcl@users.sourceforge.net>
        D. Michael McIntyre <dmmcintyr@users.sourceforge.net>

    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef RG_TRACKPARAMETERBOX_H
#define RG_TRACKPARAMETERBOX_H

#include "RosegardenParameterArea.h"
#include "RosegardenParameterBox.h"

#include "base/Track.h"
#include "base/Composition.h"
#include "gui/widgets/ColourTable.h"
#include "base/Device.h"  // For DeviceId

#include <QString>

class QWidget;
class QPushButton;
class QLabel;
class QComboBox;
class QCheckBox;

#include <map>
#include <vector>

namespace Rosegarden
{


class CollapsingFrame;
class RosegardenDocument;


/// The "Track Parameters" box in the left pane of the main window.
class TrackParameterBox : public RosegardenParameterBox,
                          public CompositionObserver
{
Q_OBJECT
        
public:
    TrackParameterBox(RosegardenDocument *doc, QWidget *parent = 0);
    
    void setDocument(RosegardenDocument *doc);

    // ??? What about CompositionObserver::selectedTrackChanged()?  Should
    //     we use that instead of having our own?  Is this redundant?
    void selectedTrackChanged2();

public slots:
    // Signals from widgets.
    void slotPlaybackDeviceChanged(int index);
    void slotInstrumentChanged(int index);
    void slotArchiveChanged(bool checked);
    void slotRecordingDeviceChanged(int index);
    void slotRecordingChannelChanged(int index);
    void slotThruRoutingChanged(int index);

    /// Update all controls in the Track Parameters box.
    /** The "dummy" int is for compatibility with the
     *  TrackButtons::instrumentSelected() signal.  See
     *  RosegardenMainViewWidget's ctor which connects the two.
     *
     *  ??? This would probably be better handled with a separate
     *      slotInstrumentSelected() that takes the instrument ID, ignores it,
     *      and calls a new public updateControls() (which would no longer need
     *      the dummy).  Then callers of updateControls() would no longer need
     *      the cryptic "-1".
     */
    void slotUpdateControls(int dummy);
    void slotInstrumentChanged(Instrument *instrument);

    void slotClefChanged(int clef);
    void slotTransposeChanged(int transpose);
    void slotTransposeIndexChanged(int index);
    void slotTransposeTextChanged(QString text);
    void slotDocColoursChanged();
    void slotColorChanged(int index);
    void slotHighestPressed();
    void slotLowestPressed();
    void slotPresetPressed();

    void slotStaffSizeChanged(int index);
    void slotStaffBracketChanged(int index);
    void slotPopulateDeviceLists();

signals:
    void instrumentSelected(TrackId, int);

protected:
    void populatePlaybackDeviceList();
    void populateRecordingDeviceList();
    void updateHighLow();

private:
    RosegardenDocument *m_doc;

    int m_selectedTrackId;
    Track *getTrack();

    // Track number and name
    QLabel *m_trackLabel;
    void selectedTrackNameChanged();

    // --- Playback parameters --------------------------------------

    // Playback Device
    QComboBox *m_playDevice;
    typedef std::vector<DeviceId> IdsVector;
    IdsVector m_playDeviceIds;

    // Playback Instrument
    QComboBox *m_instrument;
    std::map<DeviceId, IdsVector> m_instrumentIds;
    std::map<DeviceId, QStringList> m_instrumentNames;

    // Archive
    QCheckBox *m_archive;

    // --- Record filters -------------------------------------------

    // Record Device
    QComboBox *m_recDevice;
    IdsVector m_recDeviceIds;
    char m_lastInstrumentType;

    // Record Channel
    QComboBox *m_recChannel;

    // Thru Routing
    QComboBox *m_thruRouting;

    // --- Staff export options -------------------------------------

    // Notation size
    QComboBox *m_notationSizeCombo;

    // Bracket type
    QComboBox *m_bracketTypeCombo;

    // --- Create segments with -------------------------------------

    CollapsingFrame *m_createSegmentsWithFrame;

    // Preset
    QLabel *m_preset;
    QPushButton *m_loadButton;

    // Clef
    QComboBox *m_clefCombo;

    // Transpose
    QComboBox *m_transposeCombo;

    // Pitch
    QPushButton *m_lowestButton;
    QPushButton *m_highestButton;
    int m_lowestPlayable;
    int m_highestPlayable;

    // Color
    QComboBox *m_colorCombo;
    // Position of the Add Colour item in m_colorCombo.
    int m_addColourPos;
    ColourTable::ColourList m_colourList;


    // CompositionObserver interface
    virtual void trackChanged(const Composition *comp, Track *track);
    virtual void tracksDeleted(const Composition *comp, std::vector<TrackId> &trackIds);
    virtual void trackSelectionChanged(const Composition *, TrackId);
};


}

#endif
