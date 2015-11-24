% This LilyPond file was generated by Rosegarden 15.12
\include "nederlands.ly"
\version "2.12.0"
\header {
    subtitle = "LilyPond export and notation editor should match"
    title = "Marks Test"
    tagline = "Created using Rosegarden 15.12 and LilyPond"
}
#(set-global-staff-size 18)
#(set-default-paper-size "a4")
global = { 
    \time 4/4
    \skip 1*13 
}
globalTempo = {
    \override Score.MetronomeMark #'transparent = ##t
    \tempo 4 = 120  
}
\score {
    << % common
        % Force offset of colliding notes in chords:
        \override Score.NoteColumn #'force-hshift = #1.0
        % Allow fingerings inside the staff (configured from export options):
        \override Score.Fingering #'staff-padding = #'()

        \context Staff = "track 1" << 
            \set Staff.midiInstrument = "Acoustic Grand Piano"
            \set Score.skipBars = ##t
            \set Staff.printKeyCancellation = ##f
            \new Voice \global
            \new Voice \globalTempo
            \set Staff.autoBeaming = ##f % turns off all autobeaming

            \context Voice = "voice 1" {
                % Segment: Acoustic Grand Piano (copied)
                \override Voice.TextScript #'padding = #2.0
                \override MultiMeasureRest #'expand-limit = 1
                \once \override Staff.TimeSignature #'style = #'() 
                \time 4/4
                
                \clef "treble"
                f' 4 -\accent r f' -\tenuto r  |
                f' 4 -\staccato r f' -\staccatissimo r  |
                f' 4 -\marcato \startTrillSpan r f' -\open r 
                % warning: overlong bar truncated here |
                f' 4 -\stopped r f' _\sf r  |
%% 5
                f' 4 _\rfz r f' -\trill r  |
                f' 4 r f' -\turn r  |
                f' 4 -\fermata r f' -\upbow r  |
                f' 4 -\downbow r f' -\flageolet r  |
                f' 4 -\mordent \stopTrillSpan r f' -\prall r  |
%% 10
                f' 4 -\prallmordent r f' -\prallprall r  |
                f' 4 _\markup { \italic Text }  r f' 2 \startTrillSpan _~ 
                % warning: overlong bar truncated here |
                f' 2 _~ f'  |
                f' 1 \stopTrillSpan \startTrillSpan  |
                \bar "|."
            } % Voice
        >> % Staff (final) ends

    >> % notes

    \layout {
        \context { \Staff \RemoveEmptyStaves }
        \context { \GrandStaff \accepts "Lyrics" }
    }
%     uncomment to enable generating midi file from the lilypond source
%         \midi {
%         } 
} % score
