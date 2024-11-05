team_id=9

MP=4

# clean all files
make -C .. clean

mkdir -p mp$MP\_$team_id
rm -rf mp$MP\_$team_id/*

LIBS="../libtcp ../libhttp"

cp -r src/* $LIBS README.md test_cases_report_9.pdf mp$MP\_$team_id/

# go through src/makefile and change ../../ to nothing
sed -i 's/..\/..\/libtcp/libtcp/g' mp$MP\_$team_id/makefile
sed -i 's/..\/..\/libhttp/libhttp/g' mp$MP\_$team_id/makefile

zip -r mp$MP\_$team_id.zip mp$MP\_$team_id

rm -rf mp$MP\_$team_id