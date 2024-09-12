team_id=9

mkdir -p mp1_$team_id
rm -f mp1_$team_id/*

cp ./test_cases_report_9.pdf chatgpt/* src/* README.md mp1_ecen602.drawio.png mp1_$team_id/

zip -r mp1_$team_id.zip mp1_$team_id

rm -rf mp1_$team_id