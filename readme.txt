STM32F103C8T6

Teraterm:
Foreground coloring
ANSI    terminfo equivalent Description
[ 3 0 m setaf 0 Set foreground to color #0 - black
[ 3 1 m setaf 1 Set foreground to color #1 - red
[ 3 2 m setaf 2 Set foreground to color #2 - green
[ 3 3 m setaf 3 Set foreground to color #3 - yellow
[ 3 4 m setaf 4 Set foreground to color #4 - blue
[ 3 5 m setaf 5 Set foreground to color #5 - magenta
[ 3 6 m setaf 6 Set foreground to color #6 - cyan
[ 3 7 m setaf 7 Set foreground to color #7 - white
[ 3 9 m setaf 9 Set default color as foreground color
Background coloring
ANSI    terminfo equivalent Description
[ 4 0 m setab 0 Set background to color #0 - black
[ 4 1 m setab 1 Set background to color #1 - red
[ 4 2 m setab 2 Set background to color #2 - green
[ 4 3 m setab 3 Set background to color #3 - yellow
[ 4 4 m setab 4 Set background to color #4 - blue
[ 4 5 m setab 5 Set background to color #5 - magenta
[ 4 6 m setab 6 Set background to color #6 - cyan
[ 4 7 m setab 7 Set background to color #7 - white
[ 4 9 m setaf 9 Set default color as background color

echo -e '\033[?47h' # save screen
echo -e '\033[?47l' # restore screen
