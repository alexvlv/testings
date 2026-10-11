. ${ALIAS_DIR}/terminal.sh

alias nns='ip netns identify $$'
alias nns.l='ip netns list'
alias nns.run='sudo -E ip netns exec net sudo -u "$USER" -- '
alias nns.sh='nns.run bash'

netns_aliases()
{
	local -A net=(
		[wg_conf]=wgn
	)

	local -A fmsk=(
		[wg_conf]=wgf
	)

	local -A imsk=(
		[wg_conf]=wgi
	)

	local -A buh=(
		[wg_conf]=awgb
	)

	local configs=(net buh fmsk imsk)

	for cfg_name in "${configs[@]}"; do
		local -n cfg="$cfg_name"
		if [[ -v 'cfg[wg_conf]' ]]; then
			#echo "wg_conf for [$cfg_name]: ${cfg[wg_conf]}"
			eval "alias nns.${cfg_name}.up='netns.sh ${cfg_name} up ${cfg[wg_conf]}'"
		else 
			eval "alias nns.${cfg_name}.up='netns.sh ${cfg_name} up'"
		fi
		eval "alias nns.${cfg_name}.down='netns.sh ${cfg_name} down'"
		#eval "alias nns.${cfg_name}='sudo -E ip netns exec ${cfg_name} runuser -u "$USER" --preserve-environment -- '"
		#eval "alias nns.${cfg_name}='sudo -E ip netns exec ${cfg_name} sudo -i -u "$USER" -- '"
		#eval "alias nns.${cfg_name}='sudo -E ip netns exec ${cfg_name} sudo -u "$USER" --preserve-env  -- '"
		eval "alias nns.${cfg_name}='sudo -E ip netns exec ${cfg_name} sudo -u "$USER" -- '"
		eval "alias nns.${cfg_name}.sh='tt NS-${cfg_name};nns.${cfg_name} bash; tt'"
	done
}

#netns_aliases


#alias netns.i='sudo -E ip netns exec inet runuser -u "$USER" --preserve-environment -- '
#sudo -E ip netns exec ${cfg_name} sudo -u "$USER" -- bash
