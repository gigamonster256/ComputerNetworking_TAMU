team_id=9

# clean all files
make -C .. clean

mkdir -p mp2_$team_id
rm -rf mp2_$team_id/*

LIBS="../libtcp ../libsbcp"

cp -r ./test_cases_report_9.pdf makefile src README.md $LIBS chatgpt chatgpt_enhanced mp2_ecen602.png mp2_$team_id/

# submission bundles the libs alongside the makefile, so point it at them
sed -i 's|\.\./libtcp|libtcp|g' mp2_$team_id/makefile
sed -i 's|\.\./libsbcp|libsbcp|g' mp2_$team_id/makefile

zip -r mp2_$team_id.zip mp2_$team_id

rm -rf mp2_$team_id