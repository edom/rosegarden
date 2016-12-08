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

#ifndef RG_AUDIOFILEMANAGER_H
#define RG_AUDIOFILEMANAGER_H

#include <string>
#include <vector>
#include <set>

#include <QPixmap>
#include <QObject>
#include <QUrl>
#include <QPointer>
#include <QProgressDialog>

#include "AudioFile.h"
#include "PeakFileManager.h"

#include "base/XmlExportable.h"
#include "base/Exception.h"

class QProcess;

namespace Rosegarden
{

typedef std::vector<AudioFile *>::const_iterator AudioFileManagerIterator;

/**
 * AudioFileManager loads and maps audio files to their
 * internal references (ids).  A point of contact for
 * AudioFile information - loading a Composition should
 * use this class to pick up the AudioFile references,
 * editing the AudioFiles in a Composition will be
 * made through this manager.
 *
 * This is in the sound library because it's so closely
 * connected to other sound classes like the AudioFile
 * ones.  However, the audio file manager itself within
 * Rosegarden is stored in the GUI process.  This class
 * is not (and should not be) used elsewhere within the
 * sound or sequencer libraries.
 */
class AudioFileManager : public QObject, public XmlExportable
{
    Q_OBJECT
public:
    AudioFileManager();
    virtual ~AudioFileManager();

    class BadAudioPathException : public Exception
    {
    public:
        BadAudioPathException(QString path) :
            Exception(QObject::tr("Bad audio file path ") + path), m_path(path) { }
        BadAudioPathException(QString path, QString file, int line) :
            Exception(QObject::tr("Bad audio file path ") + path, file, line), m_path(path) { }
        BadAudioPathException(const SoundFile::BadSoundFileException &e) :
            Exception(QObject::tr("Bad audio file path (malformed file?) ") + e.getPath()), m_path(e.getPath()) { }

        ~BadAudioPathException() throw() { }

        QString getPath() const { return m_path; }

    private:
        QString m_path;
    };

private:
    AudioFileManager(const AudioFileManager &aFM);
    AudioFileManager &operator=(const AudioFileManager &);

public:

    /// Create an AudioFile object from an absolute path
    /**
     * We use this interface to add an actual file.  This only works
     * with files that are already in a format RG understands natively.
     * If you are not sure about that, use importFile() or importURL()
     * instead.
     *
     * throws BadAudioPathException
     */
    AudioFileId addFile(const QString &filePath);

    /// Create an audio file by importing from a URL
    /**
     * throws BadAudioPathException, BadSoundFileException
     */
    AudioFileId importURL(const QUrl &filePath,
                          int targetSampleRate,
                          QPointer<QProgressDialog> progressDialog = 0);

    /// Used by RoseXmlHandler to add an audio file.
    /**
     * throws BadAudioPathException
     */
    bool insertFile(const std::string &name, const QString &fileName,
                    AudioFileId id);

    /// Does a specific file id exist?
    bool fileExists(AudioFileId id);

    /// Does a specific file path exist?  Return ID or -1.
    int fileExists(const QString &path);

    AudioFile* getAudioFile(AudioFileId id);

    /// Get an iterator into the list of AudioFile objects.
    AudioFileManagerIterator begin() const
        { return m_audioFiles.begin(); }

    AudioFileManagerIterator end() const
        { return m_audioFiles.end(); }

    /// Remove one audio file.
    bool removeFile(AudioFileId id);
    /// Remove all audio files.
    void clear();

    /// Get the audio record path
    QString getAudioPath() const  { return m_audioPath; }
    /// Set the audio record path
    void setAudioPath(const QString &path);

    /// Throw if the current audio path does not exist or is not writable
    /**
     * throws BadAudioPathException
     */
    void testAudioPath();

    /**
     * Get a new audio filename at the audio record path, inserting the
     * projectFilename and instrumentAlias into the filename for easier
     * recognition away from the file's original context.
     *
     * throws BadAudioPathException
     */
    AudioFile *createRecordingAudioFile(
            QString projectName, QString instrumentAlias);

    /// Return whether a file was created by recording within this "session"
    bool wasAudioFileRecentlyRecorded(AudioFileId id);

    /// Return whether a file was created by derivation within this "session"
    bool wasAudioFileRecentlyDerived(AudioFileId id);

    /**
     * Indicate that a new "session" has started from the point of
     * view of recorded and derived audio files (e.g. that the
     * document has been saved)
     */
    void resetRecentlyCreatedFiles();

    /// Create an empty file "derived from" the source (used by e.g. stretcher)
    AudioFile *createDerivedAudioFile(AudioFileId source,
                                      const char *prefix);

    virtual std::string toXmlString() const;

    /// Generate previews for all audio files.
    /**
     * throw BadSoundFileException, BadPeakFileException
     */
    void generatePreviews(QPointer<QProgressDialog> progressDialog);

    /// Generate preview for a single audio file.
    /**
     * throws BadSoundFileException, BadPeakFileException
     */
    bool generatePreview(AudioFileId id);

    /**
     * Get a preview for an AudioFile adjusted to Segment start and
     * end parameters (assuming they fall within boundaries).
     *
     * We can get back a set of values (floats) or a Pixmap if we
     * supply the details.
     *
     * throws BadPeakFileException, BadAudioPathException
     */
    std::vector<float> getPreview(AudioFileId id,
                                  const RealTime &startTime, 
                                  const RealTime &endTime,
                                  int width,
                                  bool withMinima);

    /// Draw a fixed size (fixed by QPixmap) preview of an audio file
    /**
     * throws BadPeakFileException, BadAudioPathException
     */
    void drawPreview(AudioFileId id,
                     const RealTime &startTime, 
                     const RealTime &endTime,
                     QPixmap *pixmap);

    /**
     * Usually used to show how an audio Segment makes up part of
     * an audio file.
     *
     * throws BadPeakFileException, BadAudioPathException
     */
    void drawHighlightedPreview(AudioFileId it,
                                const RealTime &startTime,
                                const RealTime &endTime,
                                const RealTime &highlightStart,
                                const RealTime &highlightEnd,
                                QPixmap *pixmap);

    /// Convert the user's home directory to a "~".
    /**
     * ??? rename: homeToTilde()
     */
    QString substituteHomeForTilde(const QString &path) const;
    /// Expand "~" to the user's home directory.
    /**
     * ??? rename: tildeToHome()
     */
    QString substituteTildeForHome(const QString &path) const;

    /// Get a split point vector from a peak file
    /**
     * throws BadPeakFileException, BadAudioPathException
     */
    std::vector<SplitPointPair> 
        getSplitPoints(AudioFileId id,
                       const RealTime &startTime,
                       const RealTime &endTime,
                       int threshold,
                       const RealTime &minTime = RealTime(0, 100000000));

    int getExpectedSampleRate() const  { return m_expectedSampleRate; }
    void setExpectedSampleRate(int rate)  { m_expectedSampleRate = rate; }

    std::set<int> getActualSampleRates() const;

    /// Show entries for debug purposes
    void print();

    /**
     * Insert an audio file into the AudioFileManager and get the
     * first allocated id for it.  Used from the RG file as we already
     * have both name and filename/path.
     *
     * throws BadAudioPathException
     */
    //AudioFileId insertFile(const std::string &name,
    //                       const QString &fileName);

    //const PeakFileManager &getPeakFileManager() const  { return m_peakManager; }
    //PeakFileManager &getPeakFileManager()  { return m_peakManager; }

    /// Get the last file in the vector - the last created.
    //AudioFile *getLastAudioFile();

signals:
    /// For progress dialogs.
    /**
     * We need to get rid of this eventually and use m_progressDialog
     * instead.  For now, though, there are many parts of the system
     * that rely on this signal.
     *
     * ??? rename: progressValue() (Beware the perils of renaming signals!)
     */
    void setValue(int);

    /// For progress dialogs.
    /**
     * This allows us to modify the progress dialog's text label.
     *
     * We need to get rid of this.  Clients should pass in a progress
     * dialog and we should call setLabelText() directly.
     *
     * RosegardenMainViewWidget::slotDroppedNewAudio() and
     * AudioManagerDialog::addFile() are the only users of this.
     *
     * ??? rename: progressLabel()
     */
    void setOperationName(QString);

public slots:
    /// Cancel a running preview.
    /**
     * This is used by the progress dialogs when Cancel is pressed.
     *
     * Once m_progressDialog is in place, this can likely go.
     */
    void slotStopPreview();

    /// Does nothing.
    void slotStopImport();

private:
    /// The audio files we are managing.
    std::vector<AudioFile *> m_audioFiles;

    /**
     * Create an audio file by importing (i.e. converting and/or
     * resampling) an existing file using the conversion library.  If
     * you are not sure whether to use addFile() or importFile(), go for
     * importFile().
     *
     * throws BadAudioPathException, BadSoundFileException
     */
    AudioFileId importFile(const QString &filePath,
                           int targetSampleRate);

    /**
     * Convert an audio file from arbitrary external format to an
     * internal format suitable for use by addFile, using packages in
     * Rosegarden.  This replaces the Perl script previously used. It
     * returns 0 for OK.  This is used by importFile and importURL
     * which normally provide the more suitable interface for import.
     */
    int convertAudioFile(const QString &inFile, const QString &outFile);

    /// Get a short file name from a long one (with '/'s)
    QString getShortFilename(const QString &fileName) const;

    /// Get a directory from a full file path
    QString getDirectory(const QString &path) const;

    /// See if we can find a given file in our search path
    /**
     * Returns the first occurrence of a match or the empty
     * std::string if no match.
     */
    QString getFileInPath(const QString &file);

    /// Reset ID counter based on actual Audio Files in Composition
    void updateAudioFileID(AudioFileId id);

    /// Fetch a new unique Audio File ID.
    AudioFileId getUniqueAudioFileID();
    /// Last Audio File ID that was handed out by getUniqueAudioFileID().
    unsigned int m_lastAudioFileID;

    QString m_audioPath;

    PeakFileManager m_peakManager;

    // All audio files are stored in m_audioFiles.  These additional
    // sets of pointers just refer to those that have been created by
    // recording or derivations within the current session, and thus
    // that the user may wish to remove at the end of the session if
    // the document is not saved.
    std::set<AudioFile *> m_recordedAudioFiles;
    std::set<AudioFile *> m_derivedAudioFiles;

    int m_expectedSampleRate;

    /// Progress Dialog passed in by clients.
    QPointer<QProgressDialog> m_progressDialog;
};


}

#endif // RG_AUDIOFILEMANAGER_H
