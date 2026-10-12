. ${ALIAS_DIR}/terminal.sh

# GIT Rev.: $Format:%cd %cn %h %D$

alias nns.i='ip netns identify $$'
alias nns.l='ip netns list'
alias nns.up='netns.sh up'
alias nns.down='netns.sh down'
#alias nns='f_(){ [ "$#" -ge 1 ] && sudo -E ip netns exec net sudo -u "$USER" -- $@ || ip netns list; };f_' 
alias nns='ip netns list && sudo -E ip netns exec net sudo -u "$USER" -- ' 
alias nns.sh='nns bash'

#alias netns.i='sudo -E ip netns exec inet runuser -u "$USER" --preserve-environment -- '
#sudo -E ip netns exec ${cfg_name} sudo -u "$USER" -- bash
