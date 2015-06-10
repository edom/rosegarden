
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

#ifndef RG_AUDIOMIXERWINDOW_H
#define RG_AUDIOMIXERWINDOW_H

#include "base/MidiProgram.h"
#include "MixerWindow.h"
#include "gui/general/ActionFileClient.h"
#include "gui/widgets/PluginPushButton.h"

#include <QPixmap>
#include <QSharedPointer>

#include <vector>
#include <map>


class QWidget;
class QPushButton;
class QHBoxLayout;
class QFrame;


namespace Rosegarden
{

class Rotary;
class RosegardenDocument;
class MappedEvent;
class Fader;
class AudioVUMeter;
class AudioRouteMenu;
class PluginPushButton;
class Instrument;
class InstrumentStaticSignals;


class AudioMixerWindow : public MixerWindow, public ActionFileClient
{
    Q_OBJECT

public:
    AudioMixerWindow(QWidget *parent, RosegardenDocument *document);
    ~AudioMixerWindow();

    void updateMeters();
    void updateMonitorMeters();

public slots:
    /// Handle events from the external controller port.
    /**
     * @see RosegardenMainViewWidget::slotControllerDeviceEventReceived()
     * @see MidiMixerWindow::slotControllerDeviceEventReceived()
     */
    void slotControllerDeviceEventReceived(MappedEvent *,
                                           const void *preferredCustomer);

signals:
    void selectPlugin(QWidget *, InstrumentId id, int index);

    void play();
    void stop();
    void fastForwardPlayback();
    void rewindPlayback();
    void fastForwardPlaybackToEnd();
    void rewindPlaybackToBeginning();
    void record();
    void panic();

protected slots:
    void slotFaderLevelChanged(float level);
    void slotPanChanged(float value);
    void slotChannelsChanged();
    void slotSoloChanged();
    void slotMuteChanged();
    void slotRecordChanged();
    void slotSelectPlugin();
    void slotRepopulate();
    
    // to be called if something changes in an instrument parameter box
    void slotInstrumentChanged(Instrument *);

    void slotTrackAssignmentsChanged();

    // from Plugin dialog
    void slotPluginSelected(InstrumentId id, int index, int plugin);
    void slotPluginBypassed(InstrumentId id, int pluginIndex, bool bp);

    void slotSetInputCountFromAction();
    void slotSetSubmasterCountFromAction();
    void slotSetPanLaw();

    void slotToggleFaders();
    void slotToggleSynthFaders();
    void slotToggleSubmasters();
    void slotTogglePluginButtons();
    void slotToggleUnassignedFaders();

    void slotUpdateFaderVisibility();
    void slotUpdateSynthFaderVisibility();
    void slotUpdateSubmasterVisibility();
    void slotUpdatePluginButtonVisibility();
    void slotHelpRequested();
    void slotHelpAbout();

protected:
    virtual void sendControllerRefresh();

private:

    void toggleNamedWidgets(bool show, const char* const);
    

    // manage the various bits of it in horizontal/vertical slices
    // with other faders:

    struct FaderRec {

        FaderRec() :
            m_populated(false),
            m_input(0), m_output(0), m_pan(0), m_fader(0), m_meter(0),
            m_recordButton(0), m_stereoButton(0), m_stereoness(false),
            m_pluginBox(0)
        { }

        void setVisible(bool);
        void setPluginButtonsVisible(bool);
        
        bool m_populated;

        AudioRouteMenu *m_input;
        AudioRouteMenu *m_output;

        Rotary *m_pan;
        Fader *m_fader;
        AudioVUMeter *m_meter;

        QPushButton *m_recordButton;
        QPushButton *m_stereoButton;
        bool m_stereoness;

        QWidget *m_pluginBox;
        std::vector<PluginPushButton *> m_plugins;
    };

    QWidget *m_surroundBox;
    QHBoxLayout *m_surroundBoxLayout;
    QFrame *m_mainBox;

    typedef std::map<InstrumentId, FaderRec> FaderMap;
    FaderMap m_faders;

    typedef std::vector<FaderRec> FaderVector;
    FaderVector m_submasters;
    FaderRec m_monitor;
    FaderRec m_master;

    void depopulate();
    void populate();

    bool isInstrumentAssigned(InstrumentId id);

    void updateFader(int id); // instrument id if large enough, monitor if -1, master/sub otherwise
    void updateRouteButtons(int id);
    void updateStereoButton(int id);
    void updatePluginButtons(int id);
    void updateMiscButtons(int id);

    QPixmap m_monoPixmap;
    QPixmap m_stereoPixmap;

    void setRewFFwdToAutoRepeat();

    QSharedPointer<InstrumentStaticSignals> m_instrumentStaticSignals;
};



}

#endif
