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

#define RG_MODULE_STRING "[EditViewBase]"

#include "EditViewBase.h"

#include "document/RosegardenDocument.h"
#include "document/CommandHistory.h"
#include "gui/dialogs/ConfigureDialog.h"
#include "gui/dialogs/TimeDialog.h"
#include "base/Clipboard.h"
#include "commands/segment/SegmentReconfigureCommand.h"

#include "gui/widgets/TmpStatusMsg.h"

#include "misc/Debug.h"

#include <QShortcut>
#include <QTabWidget>
#include <QAction>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>

namespace Rosegarden
{

EditViewBase::EditViewBase(RosegardenDocument *doc,
                           std::vector<Segment *> segments,
                           QWidget *parent) :
    QMainWindow(parent),
    m_doc(doc),
    m_segments(segments),
    m_configDialogPageIndex(0),
    m_shortcuts(0)
{
    setAttribute(Qt::WA_DeleteOnClose);

    m_doc->attachEditView(this);

    connect(CommandHistory::getInstance(), SIGNAL(commandExecuted()),
            this, SLOT(slotTestClipboard()));

    m_shortcuts = new QShortcut(this);
}

EditViewBase::~EditViewBase()
{
    m_doc->detachEditView(this);
    slotSaveOptions();
}

void
EditViewBase::slotSegmentDeleted(Segment *s)
{
    RG_DEBUG << "EditViewBase::slotSegmentDeleted" << endl;
    for (std::vector<Segment *>::iterator i = m_segments.begin();
         i != m_segments.end(); ++i) {
        if (*i == s) {
            m_segments.erase(i);
            return;
        }
    }
}

void EditViewBase::slotSaveOptions()
{
}

void EditViewBase::readOptions()
{
    QAction *a = findAction("options_show_statusbar");
    if (a) a->setChecked( ! statusBar()->isHidden() );

//    a = findAction("options_show_toolbar");
//    if (a) a->setChecked( ! m_toolBar->isHidden());
}

void EditViewBase::setCheckBoxState(QString actionName, QString toolbarName)
{
  // Use !isHidden() for visibility since ancestors may not be visible
  // since this is called during the Matrixview constructor.
  bool view = !findToolbar(toolbarName)->isHidden();
  findAction(actionName)->setChecked(view);
}


void EditViewBase::setupBaseActions(bool haveClipboard)
{
    // Actions all edit views will have

//    createAction("options_show_toolbar", SLOT(slotToggleToolBar()));
    createAction("options_show_statusbar", SLOT(slotToggleStatusBar()));
    createAction("options_configure", SLOT(slotConfigure()));
//    createAction("options_configure_keybindings", SLOT(slotEditKeys()));
//    createAction("options_configure_toolbars", SLOT(slotEditToolbars()));

    createAction("file_save", SIGNAL(saveFile()));
    createAction("file_close", SLOT(slotCloseWindow()));

    if (haveClipboard) {
        createAction("edit_cut", SLOT(slotEditCut()));
        createAction("edit_copy", SLOT(slotEditCopy()));
        createAction("edit_paste", SLOT(slotEditPaste()));
    }

    createAction("open_in_matrix", SLOT(slotOpenInMatrix()));
    createAction("open_in_percussion_matrix", SLOT(slotOpenInPercussionMatrix()));
    createAction("open_in_notation", SLOT(slotOpenInNotation()));
    createAction("open_in_event_list", SLOT(slotOpenInEventList()));
    createAction("open_in_pitch_tracker", SLOT(slotOpenInPitchTracker()));
    createAction("set_segment_start", SLOT(slotSetSegmentStartTime()));
    createAction("set_segment_duration", SLOT(slotSetSegmentDuration()));
}

void EditViewBase::slotConfigure()
{
    ConfigureDialog *configDlg =
        new ConfigureDialog(getDocument(), this);

    configDlg->show();
}

void EditViewBase::slotEditKeys()
{
//&&&    KKeyDialog::configure(actionCollection());
}

void EditViewBase::slotEditToolbars()
{
//&&&
//    KEditToolbar dlg(actionCollection(), getRCFileName());

//    connect(&dlg, SIGNAL(newToolbarConfig()),
//            SLOT(slotUpdateToolbars()));

//    dlg.exec();
}

void EditViewBase::slotUpdateToolbars()
{
//!!!    createGUI(getRCFileName());
    //m_viewToolBar->setChecked(!toolBar()->isHidden());
}

void
EditViewBase::slotOpenInNotation()
{
    emit openInNotation(m_segments);
}

void
EditViewBase::slotOpenInMatrix()
{
    emit openInMatrix(m_segments);
}

void
EditViewBase::slotOpenInPercussionMatrix()
{
    emit openInPercussionMatrix(m_segments);
}

void
EditViewBase::slotOpenInEventList()
{
    emit openInEventList(m_segments);
}

void
EditViewBase::slotOpenInPitchTracker()
{
    emit slotOpenInPitchTracker(m_segments);
}

void EditViewBase::closeEvent(QCloseEvent* /* e */)
{
    RG_DEBUG << "EditViewBase::closeEvent()\n";
/*!!!
    if (isInCtor()) {
        RG_DEBUG << "EditViewBase::closeEvent() : is in ctor, ignoring close event\n";
        e->ignore();
    } else {
//         KMainWindow::closeEvent(e);
		close(e);
    }
*/
}

void EditViewBase::slotCloseWindow()
{
    close();
}
/*
void EditViewBase::slotToggleToolBar()
{
    TmpStatusMsg msg(tr("Toggle the toolbar..."), this);

    if (m_toolBar->isVisible())
		m_toolBar->hide();
    else
		m_toolBar->show();
}
*/
void EditViewBase::slotToggleStatusBar()
{
    TmpStatusMsg msg(tr("Toggle the statusbar..."), this);

    if (statusBar()->isVisible())
        statusBar()->hide();
    else
        statusBar()->show();
}

void EditViewBase::slotStatusMsg(const QString &text)
{
    ///////////////////////////////////////////////////////////////////
    // change status message permanently
    statusBar()->clearMessage();
    statusBar()->showMessage(text);	//, ID_STATUS_MSG);
}

void EditViewBase::slotStatusHelpMsg(const QString &text)
{
    ///////////////////////////////////////////////////////////////////
    // change status message of whole statusbar temporary (text, msec)
    statusBar()->showMessage(text, 2000);
}

void
EditViewBase::slotTestClipboard()
{
    if (getDocument()->getClipboard()->isEmpty()) {
        RG_DEBUG << "EditViewBase::slotTestClipboard(): empty" << endl;
        leaveActionState("have_clipboard");
	leaveActionState("have_clipboard_single_segment");
    } else {
        RG_DEBUG << "EditViewBase::slotTestClipboard(): not empty" << endl;
        enterActionState("have_clipboard");
        if (getDocument()->getClipboard()->isSingleSegment()) {
            enterActionState("have_clipboard_single_segment");
        } else {
            leaveActionState("have_clipboard_single_segment");
        }           
    }
}

void
EditViewBase::slotToggleSolo()
{
    QAction *toggleSoloAction = findAction("toggle_solo");
    if (!toggleSoloAction) return;

    bool newSoloState = toggleSoloAction->isChecked();

    //RG_DEBUG << "EditViewBase::slotToggleSolo() : solo action is " << (toggleSoloAction->isCheckable() ? "" : "NOT") << " checkable.";
    //RG_DEBUG << "EditViewBase::slotToggleSolo() : solo  = " << newSoloState;

    emit toggleSolo(newSoloState);

    if (newSoloState) {
        getDocument()->getComposition().setSelectedTrack(
                getCurrentSegment()->getTrack());
        getDocument()->getComposition().notifyTrackSelectionChanged(
                getCurrentSegment()->getTrack());

        // Old notification mechanism.
        emit selectTrack(getCurrentSegment()->getTrack());
    }
}

void
EditViewBase::slotSetSegmentStartTime()
{
    Segment *s = getCurrentSegment();
    if (!s)
        return ;

    TimeDialog dialog(this, tr("Segment Start Time"),
                      &getDocument()->getComposition(),
                      s->getStartTime(), false);

    if (dialog.exec() == QDialog::Accepted) {

        SegmentReconfigureCommand *command =
            new SegmentReconfigureCommand(tr("Set Segment Start Time"),
                    &getDocument()->getComposition());

        command->addSegment
        (s, dialog.getTime(),
         s->getEndMarkerTime() - s->getStartTime() + dialog.getTime(),
         s->getTrack());

        CommandHistory::getInstance()->addCommand(command);
    }
}

void
EditViewBase::slotSetSegmentDuration()
{
    Segment *s = getCurrentSegment();
    if (!s)
        return ;

    TimeDialog dialog(this, tr("Segment Duration"),
                      &getDocument()->getComposition(),
                      s->getStartTime(),
                      s->getEndMarkerTime() - s->getStartTime(), 
                      Note(Note::Shortest).getDuration(), false);

    if (dialog.exec() == QDialog::Accepted) {

        SegmentReconfigureCommand *command =
            new SegmentReconfigureCommand(tr("Set Segment Duration"),
                    &getDocument()->getComposition());

        command->addSegment
        (s, s->getStartTime(),
         s->getStartTime() + dialog.getTime(),
         s->getTrack());

        CommandHistory::getInstance()->addCommand(command);
    }
}

void
EditViewBase::slotCompositionStateUpdate()
{
    // update state of 'solo' toggle
    //
    QAction *toggleSolo = findAction("toggle_solo");
    if (!toggleSolo) return;

    toggleSolo->setChecked(getDocument()->getComposition().isSolo());
    RG_DEBUG << "EditViewBase::slotCompositionStateUpdate(): set solo to "
             << (toggleSolo->isChecked() ? "true" : "false") << endl;

    // update the window caption
    //
    updateViewCaption();
}

void
EditViewBase::windowActivationChange(bool /* oldState */)
{
    if (isActiveWindow()) {
        emit windowActivated();
    }
}

void
EditViewBase::handleEventRemoved(Event */* event */)
{
}

}
#include "EditViewBase.moc"
