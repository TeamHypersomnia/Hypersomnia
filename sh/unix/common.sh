function build_dir() {
	echo "build/${CONFIGURATION}_${ARCHITECTURE}"
}	

function executable_name() {
	if [[ "${config}" == "Debug" ]]; then
		echo Hypersomnia-Debug
	fi

	echo Hypersomnia
}
