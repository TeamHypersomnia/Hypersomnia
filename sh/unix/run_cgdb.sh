source sh/unix/launcher.sh 

function standard_launcher() {
	cgdb $1	
}

launcher standard_launcher
