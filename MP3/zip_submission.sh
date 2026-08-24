team_id=9

MP=3

# clean all files
make -C .. clean

mkdir -p mp$MP\_$team_id
rm -rf mp$MP\_$team_id/*

LIBS="../libudp ../libtftp"

cp -r ./test_cases_report_9.pdf README.md makefile src $LIBS mp$MP\_$team_id/

# submission bundles the libs alongside the makefile, so point it at them
sed -i 's|\.\./libudp|libudp|g' mp$MP\_$team_id/makefile
sed -i 's|\.\./libtftp|libtftp|g' mp$MP\_$team_id/makefile

zip -r mp$MP\_$team_id.zip mp$MP\_$team_id

rm -rf mp$MP\_$team_id