/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*- vi:set ts=8 sts=4 sw=4: */

#include "base/NotationTypes.h"
#include "document/io/LilyPondExporter.h"
#include "document/RosegardenDocument.h"
#include "misc/ConfigGroups.h"

#include <QTest>
#include <QDebug>
#include <QProcess>
#include <QSettings>

using namespace Rosegarden;
using std::cout;

// Unit test for lilypond export

class TestLilypondExport : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase(); // called once
    void init(); // called before every test
    void testEmptyDocument();
    void testExamples_data();
    void testExamples();
};

// Qt5 has a nice QFINDTESTDATA, but to support Qt4 we'll have our own function
static QString findFile(const QString &fileName) {
    QString attempt = QFile::decodeName(SRCDIR) + '/' + fileName;
    if (QFile::exists(attempt))
        return attempt;
    qWarning() << fileName << "NOT FOUND";
    return QString();
}

static QList<QByteArray> readLines(const QString &fileName)
{
    QList<QByteArray> lines;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Couldn't open" << fileName;
    } else {
        while (!file.atEnd()) {
            lines.append(file.readLine());
        }
    }
    return lines;
}

static void checkFile(const QString &fileName, const QString &baseline)
{
    const QList<QByteArray> expected = readLines(baseline);
    QList<QByteArray> actual = readLines(fileName);
    for (int i = 0; i < expected.count() && i < actual.count(); ++i) {
        const QByteArray line = actual.at(i);
        const QByteArray expectedLine = expected.at(i);
        QCOMPARE(QString(line), QString(expectedLine));
    }
    QCOMPARE(actual.count(), expected.count());

    // All good, clean up
    QFile::remove(fileName);
}

void TestLilypondExport::initTestCase()
{
    // We certainly don't want to mess up the user's QSettings, use a separate file.
    QCoreApplication::setApplicationName("test_lilypond_export");
}

void TestLilypondExport::init()
{
    QSettings settings;
    settings.beginGroup(LilyPondExportConfigGroup);
    settings.setValue("lilyfontsize", 12); // the default of 26 is really huge!
    settings.setValue("lilyexportbeamings", false);
    settings.endGroup();
}

void TestLilypondExport::testEmptyDocument()
{
    // GIVEN a document and a lilypond exporter
    RosegardenDocument doc(0, 0, true /*skip autoload*/, true, false /*no sequencer*/);
    const QString fileName = "out.ly";
    LilyPondExporter exporter(&doc, SegmentSelection(), qstrtostr(fileName));

    // WHEN generating lilypond
    bool result = exporter.write();

    // THEN it should produce the file but return false and a warning message
    QVERIFY(!result);
    QCOMPARE(exporter.getMessage(), QString("Export succeeded, but the composition was empty."));

    // ... and the output file should match "empty.ly"
    checkFile(fileName, findFile("baseline/empty.ly"));
}

enum Option {
    NoOptions = 0,
    ExportBeaming
};
Q_DECLARE_FLAGS(Options, Option)
Q_DECLARE_OPERATORS_FOR_FLAGS(Options)
Q_DECLARE_METATYPE(Options)
static Options defaultOptions(ExportBeaming);

void TestLilypondExport::testExamples_data()
{
    QTest::addColumn<QString>("baseName");
    QTest::addColumn<Options>("options");

    QTest::newRow("aveverum") << "aveverum" << defaultOptions;
    QTest::newRow("aylindaamiga") << "aylindaamiga" << defaultOptions;
    QTest::newRow("bogus-surf-jam") << "bogus-surf-jam" << defaultOptions;
    QTest::newRow("beaming") << "beaming" << defaultOptions;
    QTest::newRow("Brandenburg_No3-BWV_1048") << "Brandenburg_No3-BWV_1048" << defaultOptions;
    QTest::newRow("bwv-1060-trumpet-duet-excerpt") << "bwv-1060-trumpet-duet-excerpt" << defaultOptions;

    // Those work but are very slow, and the output lots and lots of
    // WARNING: Rosegarden::Exception: "Bad type for Indication model event (expected indication, found controller)"
    //QTest::newRow("children") << "children" << defaultOptions;
    //QTest::newRow("Chopin-Prelude-in-E-minor-Aere") << "Chopin-Prelude-in-E-minor-Aere" << defaultOptions;

    QTest::newRow("Djer-Fire") << "Djer-Fire" << defaultOptions;
    QTest::newRow("doodle-q") << "doodle-q" << defaultOptions;
    QTest::newRow("exercise_notation") << "exercise_notation" << defaultOptions;
    QTest::newRow("glazunov-for-solo-and-piano-with-cue") << "glazunov-for-solo-and-piano-with-cue" << defaultOptions;
    QTest::newRow("glazunov") << "glazunov" << defaultOptions;
    QTest::newRow("Hallelujah_Chorus_from_Messiah") << "Hallelujah_Chorus_from_Messiah" << defaultOptions;
    QTest::newRow("headers-and-unicode-lyrics") << "headers-and-unicode-lyrics" << defaultOptions;
    QTest::newRow("himno_de_riego") << "himno_de_riego" << defaultOptions;
    QTest::newRow("interpretation-example") << "interpretation-example" << defaultOptions;
    QTest::newRow("let-all-mortal-flesh") << "let-all-mortal-flesh" << defaultOptions;
    QTest::newRow("lilypond-alternative-endings_new-way") << "lilypond-alternative-endings_new-way" << defaultOptions;
    QTest::newRow("lilypond-alternative-endings") << "lilypond-alternative-endings" << defaultOptions;
    QTest::newRow("lilypond-directives") << "lilypond-directives" << defaultOptions;
    QTest::newRow("lilypond-staff-groupings") << "lilypond-staff-groupings" << defaultOptions;
    QTest::newRow("logical-segments-4") << "logical-segments-4" << defaultOptions;
    QTest::newRow("mandolin-sonatina") << "mandolin-sonatina" << defaultOptions;
    QTest::newRow("marks-test") << "marks-test" << defaultOptions;
    QTest::newRow("mozart-quartet") << "mozart-quartet" << defaultOptions;
    QTest::newRow("notation-for-string-orchestra-in-D-minor") << "notation-for-string-orchestra-in-D-minor" << defaultOptions;
    QTest::newRow("perfect-moment") << "perfect-moment" << defaultOptions;
    QTest::newRow("ravel-pc-gmaj-adagio") << "ravel-pc-gmaj-adagio" << defaultOptions;
    QTest::newRow("Romanza") << "Romanza" << defaultOptions;

    // THIS ONE FAILS
    // sicut-locutus.ly:98:47: error: syntax error, unexpected '}'
    //                < f g > 2 _\markup { \italic
    //                                              } _\markup { \italic Masked and substituted }  _~ f _~  |
    // ### QTest::newRow("sicut-locutus") << "sicut-locutus" << defaultOptions;

    QTest::newRow("stormy-riders") << "stormy-riders" << defaultOptions;
    QTest::newRow("test_tuplets") << "test_tuplets" << defaultOptions;
    QTest::newRow("the-rose-garden") << "the-rose-garden" << defaultOptions;
    QTest::newRow("vivaldi-cs3mv2") << "vivaldi-cs3mv2" << defaultOptions;
    QTest::newRow("vivaldi_op44_11_1") << "vivaldi_op44_11_1" << defaultOptions;
}

void TestLilypondExport::testExamples()
{
    QFETCH(QString, baseName);
    QFETCH(Options, options);

    // GIVEN
    const QString input = findFile("../../data/examples/" + baseName + ".rg");
    QVERIFY(!input.isEmpty()); // file not found
    const QString expected = findFile("baseline/" + baseName + ".ly");

    const QString fileName = baseName + ".ly";
    qDebug() << "Loading" << input << "and exporting to" << fileName;

    QSettings settings;
    settings.beginGroup(LilyPondExportConfigGroup);
    settings.setValue("lilyexportbeamings", (options & ExportBeaming) ? true : false);
    settings.endGroup();

    RosegardenDocument doc(0, 0, true /*skip autoload*/, true, false /*no sequencer*/);
    doc.openDocument(input, false /*not permanent, i.e. don't create midi devices*/, true /*no progress dlg*/);
    LilyPondExporter exporter(&doc, SegmentSelection(), qstrtostr(fileName));

    // WHEN
    QVERIFY(exporter.write());

    // THEN
    if (expected.isEmpty()) {
        // No baseline yet.

        // Use lilypond to check this file compiles before we add it to our baseline
        QProcess proc;
        proc.start("lilypond", QStringList() << "--ps" << fileName);
        proc.waitForStarted();
        proc.waitForFinished();
        QCOMPARE(proc.exitStatus(), QProcess::NormalExit);
        if (proc.exitCode() != 0) {
            qWarning() << "Generated file" << fileName << "does NOT compile!";
        }

        qWarning() << "*********** GENERATING NEW BASELINE FILE (remember to add it to SVN) ***********";
        QFile in(fileName);
        QVERIFY(in.open(QIODevice::ReadOnly));
        QFile out(QFile::decodeName(SRCDIR) + "/baseline/" + baseName + ".ly");
        QVERIFY(!out.exists());
        QVERIFY(out.open(QIODevice::WriteOnly));
        while (!in.atEnd()) {
            out.write(in.readLine());
        }
        QVERIFY(false); // make the test fail, so developers add the baseline to SVN and try again
    }

    // Compare generated file with expected file
    checkFile(fileName, expected);
}

QTEST_MAIN(TestLilypondExport)

#include "lilypond_export_test.moc"