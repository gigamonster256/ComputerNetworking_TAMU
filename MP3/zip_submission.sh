team_id=9

MP=3

# clean all files
make -C .. clean

mkdir -p mp$MP\_$team_id
rm -rf mp$MP\_$team_id/*

LIBS="../libudp ../libtftp"

cp -r src/* $LIBS mp$MP\_$team_id/

# go through src/makefile and change ../../libudp to libudp and ../../libtftp to libtftp
sed -i 's/..\/..\/libudp/libudp/g' mp$MP\_$team_id/makefile
sed -i 's/..\/..\/libtftp/libtftp/g' mp$MP\_$team_id/makefile

zip -r mp$MP\_$team_id.zip mp$MP\_$team_id

rm -rf mp$MP\_$team_id