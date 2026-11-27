git config --add remote.origin.fetch "+refs/dead/heads/*:refs/remotes/origin/dead/*"
    git config --add remote.origin.fetch "+refs/dead/tags/*:refs/tags/dead/*"
    git fetch origin

git clone --reference original-gcc --dissociate https://gcc.gnu.org/git/gcc.git new-gcc
