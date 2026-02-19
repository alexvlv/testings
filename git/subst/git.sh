# GIT Rev.: $Format:%cd %cn %h %D$

alias  git.prune="git reflog expire --all --expire=now && git gc --prune=now --aggressive"

#check tracked files that are now ignored
alias  git.ci="git ls-files -ci --exclude-standard"
alias  git.chi="git check-ignore -v "

function git_arc() {
  mkdir -p .out
  git archive --format=tar --prefix=.out/ $1 $2 | tar -xf -
}


# git archive --format=tar --prefix=.out/ HEAD $(git ls-files '*.sh') | tar -xf -
alias git.suba='f_(){ git_arc ${1:-HEAD};};f_ '
alias git.subc='f_(){ git_arc ${1:-HEAD} $(git diff-tree --no-commit-id --name-only -r ${1:-HEAD} -- . | xargs -n1 basename);};f_ '

alias git.subh='f_(){ \
    mkdir -p .out; \
    ofst=${1:?Specify number of history commits}; \
    files=$(git diff --name-only --diff-filter=d HEAD~$ofst HEAD -- . | xargs -n1 basename); \
    [ -n "$files" ] && git archive --format=tar --prefix=.out/ HEAD $files | tar -xf -; \
}; f_'
