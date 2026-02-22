#! /bin/sh

# GIT Rev.: $Format:%cd %cn %h %D$

find . -type f \( -name "*.c" -o -name "*.h" \) -exec sh -c '
	for f; do
		unexpand --tabs=4 --first-only "$f" > "$f.tmp" && mv "$f.tmp" "$f"
	done
' sh {} +
