git status
echo    # (optional) move to a new line
read -p "Push digenv.c with the commit message '$1'? " -n 1 -r
echo    # (optional) move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    git add digenv.c
    commit_string="git commit -m $1"
    eval $commit_string
    git push
fi
