function build_dir() {
	echo "build/${CONFIGURATION}_${ARCHITECTURE}"
}	

function executable_name() {
	if [[ "${CONFIGURATION}" == "Debug" ]]
   	then
		echo "Hypersomnia-Debug"
	else
		echo "Hypersomnia"
	fi
}
