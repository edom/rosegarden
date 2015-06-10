/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2015 the Rosegarden development team.

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

#include "base/MidiProgram.h"
#include "base/Track.h"
#include "base/Composition.h"
#include "gui/widgets/ColourTable.h"

#include <QString>
#include <QSharedPointer>

#include <map>
#include <vector>


class QWidget;
class QPushButton;
class QLabel;
class QFrame;
class QComboBox;
class QCheckBox;

namespace Rosegarden
{

class RosegardenDocument;
class InstrumentStaticSignals;


class TrackParameterBox : public RosegardenParameterBox,
                          public CompositionObserver
{
Q_OBJECT
        
public:
    TrackParameterBox( RosegardenDocument *doc,
                       QWidget *parent=0);
    ~TrackParameterBox();
    
    void setDocument( RosegardenDocument *doc );
    virtual void showAdditionalControls(bool); // no longer needed; preserved because it's in the base class

    virtual QString getPreviousBox(RosegardenParameterArea::Arrangement) const;

public slots:
    void slotSelectedTrackChanged();
    void slotPlaybackDeviceChanged(int index);
    void slotInstrumentChanged(int index);
    void slotRecordingDeviceChanged(int index);
    void slotRecordingChannelChanged(int index);
    /// Update all controls in the Track Parameters box.
    /** The "dummy" int is for compatibility with the
     *  TrackButtons::instrumentSelected() signal.  See
     *  RosegardenMainViewWidget's ctor which connects the two.  This
     *  would probably be better handled with a separate
     *  slotInstrumentSelected() that takes the instrument ID, ignores it,
     *  and calls a new public updateControls() (which would no longer need
     *  the dummy).  Then callers of updateControls() would no longer need
     *  the cryptic "-1".
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
    RosegardenDocument    *m_doc;

    QComboBox           *m_playDevice;
    QComboBox           *m_instrument;
    QComboBox           *m_recDevice;
    QComboBox           *m_recChannel;

    QPushButton         *m_presetButton;
    QPushButton         *m_highButton;
    QPushButton         *m_lowButton;

    QComboBox           *m_defClef;
    QComboBox           *m_defColor;
    QComboBox           *m_defTranspose;
    QComboBox           *m_staffSizeCombo;
    QComboBox           *m_staffBracketCombo;


    int                 m_addColourPos;
    int                 m_highestPlayable;
    int                 m_lowestPlayable;
    ColourTable::ColourList  m_colourList;
    
    QLabel              *m_trackLabel;
    
    typedef std::vector<DeviceId> IdsVector;
    
    IdsVector           m_playDeviceIds;    
    IdsVector           m_recDeviceIds;

    std::map<DeviceId, IdsVector>   m_instrumentIds;
    std::map<DeviceId, QStringList> m_instrumentNames;
    
    int                 m_selectedTrackId;
    
    char                m_lastInstrumentType;

    // Additional elements that may be hidden in vertical stacked mode
    QFrame              *m_playbackGroup;
    QFrame              *m_recordGroup;
    QFrame              *m_defaultsGroup;
    QFrame              *m_staffGroup;
    QLabel              *m_segHeader;
    QLabel              *m_presetLbl;
    QLabel              *m_staffGrpLbl;
    QLabel              *m_grandStaffLbl;
    QLabel              *m_clefLbl;
    QLabel              *m_transpLbl;
    QLabel              *m_colorLbl;
    QLabel              *m_rangeLbl;
    QLabel              *m_psetLbl;

    // CompositionObserver interface
    virtual void trackChanged(const Composition *comp, Track *track);
    virtual void tracksDeleted(const Composition *comp, std::vector<TrackId> &trackIds);
    virtual void trackSelectionChanged(const Composition *, TrackId);

    void selectedTrackNameChanged();

    QSharedPointer<InstrumentStaticSignals> m_instrumentStaticSignals;
};


}

#endif
