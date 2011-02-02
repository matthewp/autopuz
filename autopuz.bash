#!/bin/bash
mkdir /tmp/crossword
cd /tmp/crossword
todaydate=`date +%m-%d-%Y`

wget http://www.chron.com/apps/games/xword/puzzles/today.puz

#Processing unsolved puzzle
nytconv today.puz >today.tex
latex today.tex
dvips  -x 1400 today.dvi
psnup  -l -2 -pletter today.ps today.wps
echo \"quit\" >>today.wps
lpr -P YOUR-PRINTER today.wps

#Processing solved puzzle
nytconv2 today.puz >today2.tex
latex today2.tex
dvips  -x 1400 today2.dvi
ps2pdf today2.ps ~/$todaydate.pdf

rm -f /tmp/crossword/*.*
rmdir /tmp/crossword
