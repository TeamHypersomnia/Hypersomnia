param(
	[String]$configuration="Unknown",
	[Int32]$console_mode=0
)

$target_exe = "../nin/" + $configuration + "/Hypersomnia.exe"

$good_log = "cache/log/exit_success_debug_log.txt"
$fail_log = "cache/log/exit_failure_debug_log.txt"
$ensr_log = "cache/log/ensure_failed_debug_log.txt"

if ($console_mode -eq 1) { 
	& $target_exe --unit-tests-only
}
else {
	& $target_exe --unit-tests-only | Out-Null
}

if ($LastExitCode -ne 0) { 
	$mesg = "Test " + $configuration + " launch failed with result: " + $LastExitCode
	Write-Host $mesg -ForegroundColor red
	
	Write-Host "Ensure failed log:" -ForegroundColor red
	cat $ensr_log
	
	Write-Host "Failure log:" -ForegroundColor red
	cat $fail_log
	# $host.SetShouldExit($LastExitCode) 
}
else {
	$mesg = "Test " + $configuration + " launch succeeded."
	Write-Host $mesg -ForegroundColor green

	Write-Host "Good log:" -ForegroundColor green
	cat $good_log
}

Write-Host "Archiving the binary." -ForegroundColor yellow

cp $target_exe Hypersomnia.exe
rm -r cache
cd ../
7z a Hypersomnia-x64.zip hypersomnia