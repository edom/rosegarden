
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

#ifndef RG_COMPOSITIONVIEW_H
#define RG_COMPOSITIONVIEW_H

#include "CompositionModelImpl.h"
#include "gui/general/RosegardenScrollView.h"
#include "base/Selection.h"  // SegmentSelection
#include "base/TimeT.h"  // timeT

#include <QColor>
#include <QPen>
#include <QPixmap>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QTimer>


class QWidget;
class QWheelEvent;
class QResizeEvent;
class QPaintEvent;
class QPainter;
class QMouseEvent;
class QEvent;


namespace Rosegarden
{

class SnapGrid;
class SegmentToolBox;
class SegmentTool;
class Segment;
class RosegardenDocument;
class CompositionRect;

/// Draws the Composition on the display.  The segment canvas.
/**
 * The key output routine is drawAll() which draws the segments
 * and the artifacts (playback position pointer, guides, "rubber band"
 * selection, ...) on the viewport (the portion of the Composition that
 * is currently visible).
 *
 * TrackEditor creates and owns the only instance of this class.  This
 * class works together with CompositionModelImpl to provide the composition
 * user interface (the segment canvas).
 */
class CompositionView : public RosegardenScrollView 
{
    Q_OBJECT
public:
    CompositionView(RosegardenDocument *, CompositionModelImpl *,
                    QWidget *parent);

    /// Move the playback position pointer to a new X coordinate.
    /**
     * @see getPointerPos(), setPointerPosition(), and drawArtifacts()
     */
    void setPointerPos(int pos);
    /// Get the X coordinate of the playback position pointer.
    /**
     * @see setPointerPos() and setPointerPosition()
     */
    int getPointerPos() { return m_pointerPos; }

    /// Draw the guides.
    /**
     * The guides are blue crosshairs that stretch across the entire view.
     * They appear when selecting or moving a segment.
     *
     * @see hideGuides() and drawArtifacts()
     */
    void drawGuides(int x, int y);
    /// Hide the guides.
    void hideGuides();

    /// Set the starting position of the "rubber band" selection.
    void drawSelectionRectPos1(const QPoint &pos);
    /// Set the ending position of the "rubber band" selection.
    void drawSelectionRectPos2(const QPoint &pos);
    /// Hide the "rubber band" selection rectangle.
    void hideSelectionRect();

    /// Get the snap grid from the CompositionModelImpl.
    SnapGrid &grid()  { return m_model->grid(); }

    /// Returns the segment tool box.  See setTool() and m_toolBox.
    SegmentToolBox *getToolBox()  { return m_toolBox; }

    /// Returns the composition model.  See m_model.
    CompositionModelImpl *getModel()  { return m_model; }

    void setNewSegmentColor(const QColor &color)  { m_newSegmentColor = color; }
    /// Used by SegmentPencil to draw a new segment.
    void drawNewSegment(const QRect &r);
    void hideNewSegment()  { drawNewSegment(QRect()); }
    const QRect &getNewSegmentRect() const  { return m_newSegmentRect; }

    /// Set whether the segment items contain previews or not.
    /**
     * @see isShowingPreviews()
     */
    void setShowPreviews(bool previews)  { m_showPreviews = previews; }

    /// Return whether the segment items contain previews or not.
    /**
     * @see setShowPreviews()
     */
    //bool isShowingPreviews()  { return m_showPreviews; }

    /**
     * Delegates to CompositionModelImpl::clearSegmentRectsCache().
     */
    void clearSegmentRectsCache(bool clearPreviews = false);

    /// Return the selected Segments if we're currently using a "Selector".
    /**
     * Delegates to CompositionModelImpl::getSelectedSegments().
     *
     * @see haveSelection()
     */
    SegmentSelection getSelectedSegments();

    /// Delegates to CompositionModelImpl::haveSelection().
    /**
     * @see getSelectedSegments()
     */
    bool haveSelection() const  { return m_model->haveSelection(); }

    /// Updates the portion of the view where the selected items are.
    /**
     * This is only used for segment join and update figuration.  It
     * might be useful in more situations.  However, since the
     * CompositionView is redrawn every time a command is executed
     * (TrackEditor::slotCommandExecuted()), there's no incentive to
     * use this.
     *
     * @see RosegardenScrollView::updateContents()
     */
    void updateSelectedSegments();

    /// Draw a text float on this canvas.
    /**
     * Used by SegmentMover::mouseMoveEvent() to display time,
     * bar, and beat on the view while the user is moving a segment.
     * Also used by SegmentSelector::mouseMoveEvent() for the same
     * purpose.
     *
     * @see hideTextFloat()
     */
    void drawTextFloat(int x, int y, const QString &text);
    /// See drawTextFloat().
    void hideTextFloat()  { m_drawTextFloat = false; }

    /// Enables/disables display of the text labels on each segment.
    /**
     * From the menu: View > Show Segment Labels.
     *
     * @see drawCompRectLabel()
     */
    void setShowSegmentLabels(bool b)  { m_showSegmentLabels = b; }

    /// Delegates to CompositionModelImpl::setAudioPreviewThread().
    void endAudioPreviewGeneration();
	
    /// Set the current segment editing tool.
    /**
     * @see getToolBox()
     */
    void setTool(const QString &toolName);

    /// Selects the segments via CompositionModelImpl::setSelected().
    /**
     * Used by RosegardenMainViewWidget.
     */
    void selectSegments(const SegmentSelection &segment);

    /// Show the splitting line on a Segment.  Used by SegmentSplitter.
    /**
     * @see hideSplitLine()
     */
    void showSplitLine(int x, int y);
    /// See showSplitLine().
    void hideSplitLine();

public slots:

    /// Handle TrackButtons scroll wheel events.  See TrackEditor::m_trackButtonScroll.
    void slotExternalWheelEvent(QWheelEvent *);

    /// Redraw everything.  Segments and artifacts.
    /**
     * This is called in many places after making changes that affect the
     * segment canvas.
     *
     * RosegardenMainViewWidget connects this to
     * CommandHistory::commandExecuted().
     */
    void slotUpdateAll();
    /// Deferred update of segments and artifacts within the specified rect.
    /**
     * Because this routine is called so frequently, it doesn't actually
     * do any work.  Instead it sets a flag, m_updateNeeded, and
     * slotUpdateTimer() does the actual work by calling
     * updateAll2(rect) on a more leisurely schedule.
     */
    void slotAllNeedRefresh(const QRect &rect);

    /// Resize the contents to match the Composition.
    /**
     * @see RosegardenScrollView::resizeContents().
     */
    void slotUpdateSize();

signals:
    /// Emitted when a segment is double-clicked to launch the default segment editor.
    /**
     * Connected to RosegardenMainViewWidget::slotEditSegment().
     */
    void editSegment(Segment *);

    /// Emitted when a segment repeat is double-clicked.
    /**
     * Connected to RosegardenMainViewWidget::slotEditRepeat() which converts
     * the repeat to a segment.  This doesn't actually start the segment
     * editor.  mouseDoubleClickEvent() emits editSegment() after
     * it emits this.
     *
     * rename: segmentRepeatDoubleClick(), repeatToSegment(), others?
     *         Not sure which might be most helpful to readers of the code.
     *         editRepeat() is misleading.  repeatToSegment() is more correct
     *         at the same level of abstraction.
     */
    void editRepeat(Segment *, timeT);

    /// Emitted when a double-click occurs on the ruler.
    /**
     * Connected to RosegardenDocument::slotSetPointerPosition().
     * Connection is made by the RosegardenMainViewWidget ctor.
     *
     * @see setPointerPos()
     */
    void setPointerPosition(timeT);

    /// Emitted when hovering over the CompositionView to show help text.
    /**
     * Connected to RosegardenMainWindow::slotShowToolHelp().
     */
    void showContextHelp(const QString &);

    //void editSegmentNotation(Segment*);
    //void editSegmentMatrix(Segment*);
    //void editSegmentAudio(Segment*);
    //void editSegmentEventList(Segment*);
    //void audioSegmentAutoSplit(Segment*);

private slots:

    /// Updates the artifacts in the entire viewport.
    /**
     * In addition to being used locally several times, this is also
     * connected to CompositionModelImpl::needArtifactsUpdate().
     */
    void slotUpdateArtifacts() {
        updateContents();
    }

    /// Redraw everything with the new color scheme.
    /**
     * Connected to RosegardenDocument::docColoursChanged().
     */
    void slotRefreshColourCache();

    /**
     * Delegates to CompositionModelImpl::addRecordingItem().
     * Connected to RosegardenDocument::newMIDIRecordingSegment().
     *
     * Suggestion: Try eliminating this middleman.
     */
    void slotNewMIDIRecordingSegment(Segment *);
    /**
     * Delegates to CompositionModelImpl::addRecordingItem().
     * Connected to RosegardenDocument::newAudioRecordingSegment().
     *
     * Suggestion: Try eliminating this middleman.
     */
    void slotNewAudioRecordingSegment(Segment *);

    // no longer used, see RosegardenDocument::insertRecordedMidi()
    //     void slotRecordMIDISegmentUpdated(Segment*, timeT updatedFrom);

    /**
     * Delegates to CompositionModelImpl::clearRecordingItems().
     * Connected to RosegardenDocument::stoppedAudioRecording() and
     * RosegardenDocument::stoppedMIDIRecording().
     */
    void slotStoppedRecording();

    /// Updates the tool context help and shows it if the mouse is in the view.
    /**
     * The tool context help appears in the status bar at the bottom.
     *
     * Connected to SegmentToolBox::showContextHelp().
     *
     * @see showContextHelp()
     */
    void slotToolHelpChanged(const QString &);

    /// Used to reduce the frequency of updates.
    /**
     * slotAllNeedRefresh(rect) sets the m_updateNeeded flag to
     * tell slotUpdateTimer() that it needs to perform an update.
     */
    void slotUpdateTimer();

private:
    /// Redraw in response to AudioPreviewThread::AudioPreviewQueueEmpty.
    virtual bool event(QEvent *);

    /// Passes the event on to the current tool.
    virtual void mousePressEvent(QMouseEvent *);
    /// Passes the event on to the current tool.
    virtual void mouseReleaseEvent(QMouseEvent *);
    /// Launches a segment editor or moves the position pointer.
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    /// Passes the event on to the current tool.
    /**
     * Also handles scrolling as needed.
     */
    virtual void mouseMoveEvent(QMouseEvent *);

    /// Delegates to drawAll() for each rect that needs painting.
    virtual void paintEvent(QPaintEvent *);
    /// Handles resize.  Uses slotUpdateSize().
    virtual void resizeEvent(QResizeEvent *);

    // These are sent from the top level app when it gets key
    // depresses relating to selection add (usually Qt::SHIFT) and
    // selection copy (usually CONTROL)

    /// Called when the mouse enters the view.
    /**
     * Override of QWidget::enterEvent().  Shows context help (in the status
     * bar at the bottom) for the current tool.
     *
     * @see leaveEvent() and slotToolHelpChanged()
     */
    virtual void enterEvent(QEvent *);
    /// Called when the mouse leaves the view.
    /**
     * Override of QWidget::leaveEvent().
     * Hides context help (in the status bar at the bottom) for the current
     * tool.
     *
     * @see enterEvent() and slotToolHelpChanged()
     */
    virtual void leaveEvent(QEvent *);

    /// Draw the segments and artifacts on the viewport (screen).
    void drawAll();

    /// Scrolls and refreshes the segment layer (m_segmentsLayer) if needed.
    /**
     * Returns enough information to determine how much additional work
     * needs to be done to update the viewport.
     * Used by drawAll().
     */
    void scrollSegmentsLayer();

    /// Draw the segments on the segment layer (m_segmentsLayer).
    /**
     * Draws the background then calls drawSegments() to draw the segments on the
     * segments layer (m_segmentsLayer).  Used by
     * scrollSegmentsLayer().
     */
    void drawSegments(const QRect &);

    /// Draw the artifacts on the viewport.
    /**
     * "Artifacts" include anything that isn't a segment.  E.g. The playback
     * position pointer, guides, and the "rubber band" selection.
     * Used by drawAll().
     */
    void drawArtifacts();

    /// Draw the track dividers on the segments layer.
    void drawTrackDividers(QPainter *segmentsLayerPainter, const QRect &clipRect);

    /// drawImage() for tiled audio previews.
    /**
     * This routine hides the fact that audio previews are stored as a
     * series of QImage tiles.  It treats them as if they were one large
     * QImage.  This simplifies drawing audio previews.
     */
    void drawImage(
            QPainter *painter,
            QPoint dest, const PixmapArray &tileVector, QRect source);

    /// Draw the previews for audio segments on the segments layer (m_segmentsLayer).
    /**
     * Used by drawSegments().
     */
    void drawAudioPreviews(QPainter *segmentsLayerPainter, const QRect &clipRect);

    /// Draws a rectangle on the given painter with proper clipping.
    /**
     * This is an improved QPainter::drawRect().
     *
     * @see drawCompRect()
     */
    void drawRect(QPainter *painter, const QRect &clipRect, const QRect &rect,
                  bool isSelected = false, int intersectLvl = 0);
    /// A version of drawRect() that handles segment repeats.
    void drawCompRect(QPainter *painter, const QRect &clipRect,
                      const CompositionRect &rect, int intersectLvl = 0);

    /// Used by drawSegments() to draw the segment labels.
    /**
     * @see setShowSegmentLabels()
     */
    void drawCompRectLabel(QPainter *painter,
                           const CompositionRect &rect);
    /// Used by drawCompRectLabel() to draw a halo around a text label.
    std::vector<QPoint> m_haloOffsets;

    /// Used by drawSegments() to draw any intersections between rectangles.
    void drawIntersections(QPainter *painter, const QRect &clipRect,
                           const CompositionModelImpl::RectContainer &rects);

    /// Used by drawArtifacts() to draw floating text.
    /**
     * @see setTextFloat()
     */
    void drawTextFloat(QPainter *painter);

    /// Deferred update of the segments within the entire viewport.
    /**
     * This will cause scrollSegmentsLayer() to refresh the entire
     * segments layer (m_segmentsLayer) the next time it is called.
     * This in turn will cause drawAll() to redraw the entire
     * viewport the next time it is called.
     */
    void segmentsNeedRefresh() {
        m_segmentsRefresh.setRect(contentsX(), contentsY(), viewport()->width(), viewport()->height());
    }

    /// Deferred update of the segments within the specified rect.
    /**
     * This will cause the given portion of the viewport to be refreshed
     * the next time drawAll() is called.
     */
    void segmentsNeedRefresh(const QRect &r) {
        m_segmentsRefresh |=
            (QRect(contentsX(), contentsY(), viewport()->width(), viewport()->height())
             & r);
    }

    /// Updates the artifacts in the given rect.
    void updateArtifacts(const QRect &r) {
        updateContents(r);
    }

    /// Update segments and artifacts within rect.
    void updateAll2(const QRect &rect);

    /// Update segments and artifacts within the entire viewport.
    void updateAll() {
        segmentsNeedRefresh();
        slotUpdateArtifacts();
    }

    //--------------- Data members ---------------------------------

    CompositionModelImpl *m_model;

    /// The tool that receives mouse events.
    SegmentTool    *m_currentTool;
    SegmentToolBox *m_toolBox;

    /// Performance testing.
    bool         m_enableDrawing;

    bool         m_showPreviews;
    bool         m_showSegmentLabels;

    //int          m_minWidth;

    //QColor       m_rectFill;
    //QColor       m_selectedRectFill;

    int          m_pointerPos;
    QPen         m_pointerPen;

    QRect        m_newSegmentRect;
    QColor       m_newSegmentColor;
    QPoint       m_splitLinePos;

    QColor       m_trackDividerColor;

    bool         m_drawGuides;
    const QColor m_guideColor;
    int          m_guideX;
    int          m_guideY;

    bool         m_drawSelectionRect;
    QRect        m_selectionRect;

    bool         m_drawTextFloat;
    QString      m_textFloatText;
    QPoint       m_textFloatPos;

    /// Layer that contains the segment rectangles.
    /**
     * @see drawAll() and drawSegments()
     */
    QPixmap      m_segmentsLayer;

    /// Portion of the viewport that needs segments refreshed.
    /**
     * Used only by scrollSegmentsLayer() to limit work done redrawing
     * the segment rectangles.
     */
    QRect        m_segmentsRefresh;

    int          m_lastContentsX;
    int          m_lastContentsY;
    int          m_lastPointerRefreshX;
    QPixmap      m_backgroundPixmap;

    QString      m_toolContextHelp;
    bool         m_contextHelpShown;

    CompositionModelImpl::AudioPreviewDrawData m_audioPreview;
    CompositionModelImpl::RectRanges m_notationPreview;

    /// Drives slotUpdateTimer().
    QTimer m_updateTimer;
    /// Lets slotUpdateTimer() know there's work to do.
    bool m_updateNeeded;
    /// Accumulated update rectangle.
    QRect m_updateRect;
};


}

#endif
