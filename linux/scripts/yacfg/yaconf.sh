#!/bin/bash

#basedir="$(dirname "$(readlink -f "$0")")"
#cd "${BASEDIR}" || exit 1

config_parameters=".config_vars"
[ -s "$config_parameters" ] && . "$config_parameters"
[ -n "$config_param_dir" ] || config_param_dir="/mnt/ext/Yadisk"

config_template="config.tmpl"
config_out="config.cfg"
include_file="include.txt"
yadsk_lnk=".yadsk"
yadsk_ls=".yadir.txt"

export LC_ALL=C.UTF-8

if [ ! -s "${yadsk_ls}" ]; then
	yadsk_dir=$(readlink -f ${yadsk_lnk} 2>/dev/null)
	[ -d ${yadsk_dir} ] || { echo "ERROR: Yandex dir not available"; exit 1; }
	echo "Processing [${yadsk_dir}] ..."
	ls -1 ${yadsk_dir} > "${yadsk_ls}"
fi
[ -s "${yadsk_ls}" ] || { echo "ERROR: no Yandex dir listing"; exit 1; }

get_exclude_dirs() {
	include_file="$1"
	ls_file="$2"

	exclude_dirs=""

	# Read real dirs from ls file
	real_dirs=""
	while IFS= read -r d <&3; do
		[ -z "$d" ] && continue
		real_dirs="$real_dirs,$d"
	done 3< "$ls_file"
	real_dirs="${real_dirs#,}"

	# Check include entries against real dirs
	while IFS= read -r inc <&4; do
		inc=$(printf '%s' "$inc" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
		[ -z "$inc" ] && continue
		case ",$real_dirs," in
			*",$inc,"*) ;;  # exists
			*) printf '%s\n' "WARNING: included dir not found: [$inc]" >&2 ;;
		esac
	done 4< "$include_file"

	# Build exclude_dirs from ls file
	while IFS= read -r dir_name <&5; do
		[ -z "$dir_name" ] && continue

		skip=
		while IFS= read -r inc <&6; do
			inc=$(printf '%s' "$inc" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//')
			[ -z "$inc" ] && continue
			if [ "$inc" = "$dir_name" ]; then
				skip=1
				break
			fi
		done 6< "$include_file"

		if [ -z "$skip" ]; then
			if [ -z "$exclude_dirs" ]; then
				exclude_dirs="$dir_name"
			else
				exclude_dirs="$exclude_dirs,$dir_name"
			fi
		fi
	done 5< "$ls_file"
}

# Clean: trim, remove empty lines
sed -i '/^[[:space:]]*$/d; s/^[[:space:]]*//; s/[[:space:]]*$//' "$include_file"
sort -o "$include_file" "$include_file"
echo -e "Yandex dirs list: \n=========== [begin]=========="
cat "${yadsk_ls}"
echo "=========== [end]=========="

get_exclude_dirs "${include_file}" "${yadsk_ls}"

#echo "Excluded: [$exclude_dirs]"

[ -e $config_template ] && eval "cat <<EOF > $config_out
$(<$config_template)
EOF" 2> /dev/null

[ -e $config_out ] && {
	echo -e "\nConfig generated:\n#=========== [begin]=========="
	cat $config_out 2>/dev/null
	echo "#=========== [end]=========="
}

# iid must be in /home/user/.config/yandex-disk !
