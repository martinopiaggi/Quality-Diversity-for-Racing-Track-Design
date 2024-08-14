#$1 - file to draw
logfile=$1.simplix-log.dat
trjfile=$1.simplix-trajectory
echo "set size 1.0, 0.6" > _temp.gnu
echo "set terminal postscript portrait enhanced color solid lw 1 'Helvetica' 10" >> _temp.gnu
echo "set output '$1.ps'" >> _temp.gnu
echo -n "plot '$logfile' using 18:19 title 'Log' w l, " >> _temp.gnu 
echo -n "'$logifile' u 20:21 notitle w l lc rgbcolor 'black', " >> _temp.gnu
echo -n "'$logifile' u 22:23 notitle w l lc rgbcolor 'black', " >> _temp.gnu
echo    "'$trjfile' using 2:3 title 'Trajectory' w l lc rgbcolor 'green'" >> _temp.gnu 
gnuplot _temp.gnu
#rm _temp.gnu
#evince "$1.ps"
#rm "$1.ps"
