if ! ./is-process-running.fn.sh alsa_out; then 
	./jack-alsa-out.sh&
	sleep 1
	if ! ./is-process-running.fn.sh alsa_out; then echo "jack-alsa-out failed, aborting run."; exit; fi
fi

gdb --args ruby1.9.1 -I. -I../../std dj.rb debug druby://127.0.0.1:41069 --run-gui