#!/bin/sh

export LC_ALL=C.UTF-8

#basedir="$(dirname "$(readlink -f "$0")")"
#cd "${BASEDIR}" || exit 1

config_template="cfg.tmpl"
include_file="include.txt"
yandex_dir="/mnt/ext/Yadisk"

get_exclude_dirs() {
	target_dir="$1"
	shift
	include_file="$1"

	exclude_dirs=""

	# First pass: collect real dir names (comma-separated)
	real_dirs=""
	for d in "$target_dir"/*; do
		[ -d "$d" ] && real_dirs="$real_dirs,${d##*/}"
	done
	real_dirs="${real_dirs#,}"

	# Check include file entries against real dirs
	while IFS= read -r inc <&3; do
		inc=$(printf '%s' "$inc" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
		[ -z "$inc" ] && continue
		case ",$real_dirs," in
			*",$inc,"*) ;;  # exists
			*) printf '%s\n' "WARNING: included dir not found: [$inc]" >&2 ;;
		esac
	done 3< "$include_file"

	# Second pass: build exclude_dirs as before
	for d in "$target_dir"/*; do
		[ -d "$d" ] || continue
		dir_name="${d##*/}"

		skip=
		while IFS= read -r inc <&4; do
			inc=$(printf '%s' "$inc" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
			[ -z "$inc" ] && continue
			if [ "$inc" = "$dir_name" ]; then
				skip=1
				break
			fi
		done 4< "$include_file"

		if [ -z "$skip" ]; then
			if [ -z "$exclude_dirs" ]; then
				exclude_dirs="$dir_name"
			else
				exclude_dirs="$exclude_dirs,$dir_name"
			fi
		fi
	done
}

# Clean: trim, remove empty lines
sed -i '/^[[:space:]]*$/d; s/^[[:space:]]*//; s/[[:space:]]*$//' "$include_file"
sort -o "$include_file" "$include_file"

get_exclude_dirs "${yandex_dir}" "${include_file}"

echo [$exclude_dirs]

