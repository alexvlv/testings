#/bin/sh

export TITLE="Markdown Test"
export AUTHOR="Alex"
export DATE="$(date '+%Y-%m-%d')"
export GITREV="r$(git rev-parse --short HEAD) at $DATE on $(git rev-parse --abbrev-ref HEAD)"

envsubst < sample.md | pandoc -o .output.rtf --standalone --reference-doc=reference.rtf
