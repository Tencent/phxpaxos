
set -e  # exit immediately on error
set -x  # display all commands

git submodule update --init --recursive

(cd third_party && bash ./autoinstall.sh)

./autoinstall.sh

make

