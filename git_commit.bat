@echo off
echo Git push

set /p commit_txt=Commit text: 

git init
git add . --all
git commit -m "%commit_txt%"
git branch -M main
git remote add origin https://github.com/leonllrmc/strat_F38C_robot_2024
git push -u origin main


pause