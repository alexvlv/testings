#/bin/sh

fname=$1

[ -n "$fname" ] || exit

#export TITLE="Markdown Test"
export AUTHOR="Alex"
export DATE="$(date '+%d/%m/%Y %H:%M')"
export HOSTNAME="$(cat /etc/hostname 2>/dev/null)"
#export GITREV="r$(git rev-parse --short HEAD) at $DATE on $(git rev-parse --abbrev-ref HEAD)"

get_git_rev() {
	local REVISIONS CNT HASH
	git -C $1 rev-parse --git-dir >/dev/null 2>&1 || return 1
	[ -n "$2" ] && REVISIONS="$2..HEAD" || REVISIONS=HEAD
	CNT=$(git -C $1 rev-list $REVISIONS | wc -l | awk '{print $1}')
	#CNT=$(($CNT-1))
	HASH=$(git -C $1 log -n 1 --format="%h")
	export GIT_REV="r$CNT-$HASH"
	export GIT_ORIGIN=$(git -C $1 remote -v | grep origin | head -n1 | cut -f 2 | cut -d ' ' -f1 2>/dev/null)
	export GIT_MODIFIED=$(git -C $1 status --porcelain 2>/dev/null)
	[ -n "${GIT_MODIFIED}" ] && GIT_MODIFIED="*"
	export GIT_DATE=$(git -C $1 log -n 1 --format="%cd" --date="format:%Y-%m-%d %H:%M" 2>/dev/null)
	export GIT_BRANCH=$(git -C $1  rev-parse --abbrev-ref HEAD 2>/dev/null | tr  -s ' /' '-') 
	true
}

get_git_rev .
echo "[GIT rev.: ${GIT_REV}${GIT_MODIFIED} on ${GIT_BRANCH} at ${GIT_DATE} from ${GIT_ORIGIN}]"

outfname=${fname%.*}.rtf
echo "[ ${fname} => ${outfname} ]"
envsubst < "${fname}" | pandoc -o ${outfname} --standalone --reference-doc=/usr/local/etc/pandoc_reference.rtf
sed -i 's/{\\rtf1/{\\rtf1\\paperw11906\\paperh16838\\margl567\\margr567\\margt567\\margb567/' ${outfname}
