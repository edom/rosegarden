/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

/*
    Rosegarden
    A MIDI and audio sequencer and musical notation editor.
    Copyright 2000-2016 the Rosegarden development team.

    Other copyrights also apply to some parts of this work.  Please
    see the AUTHORS file and individual file headers for details.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef RG_PROGRESSDIALOG_H
#define RG_PROGRESSDIALOG_H

#include <QProgressDialog>
#include <QTimer>

class QString;
class QWidget;


namespace Rosegarden
{

/// A simple dialog for reporting progress.
/**
 * This was originally a subclass of KProgressDialog from KDE 3.
 */
class ProgressDialog : public QProgressDialog
{
    Q_OBJECT

public:
    ProgressDialog(const QString &labelText, QWidget *parent = 0);
    virtual ~ProgressDialog();

    /// Sets indeterminate state (Knight Rider mode) on the progress bar.
    void setIndeterminate(bool ind);

public slots:
    /// Stop and hide if we're shown
    void slotFreeze();

    /// Restore to our normal state after freezing
    void slotThaw();

    /// Set the value for this dialog's progress bar.
    /**
     * This replaces setValue() and advance() with the same function, and
     * any calls or connections to advance() must be switched over to
     * setValue().  All management of the progress bar is done by this
     * dialog now, and it does not expose its internally-managed progress
     * bar to the outside.
     */
    void setValue(int value);

protected:
    QTimer *m_showAfterTimer;
    int m_totalSteps;
    bool m_indeterminate;
};


}

#endif
