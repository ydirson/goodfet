echo source this file in current shell
export board=arduino
export GOODFET=`dmesg | grep -C 5 www.arduino.cc | grep ttyACM | cut -d: -f3 | sed 's,^ ,/dev/,'`
test -z "$GOODFET" && echo "can't find ttyACM for Arduino plug it in and re-run script"

