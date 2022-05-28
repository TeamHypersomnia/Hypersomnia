param(
	[String]$artifact_upload_key="Unknown",
	[String]$configuration="Unknown",
	[Int32]$console_mode=0
)

$target_exe = "../nin/" + $configuration + "/Hypersomnia.exe"

$good_log = "logs/exit_success_debug_log.txt"
$fail_log = "logs/exit_failure_debug_log.txt"
$ensr_log = "logs/ensure_failed_debug_log.txt"

if ($console_mode -eq 1) { 
	& $target_exe --unit-tests-only --test-fp-consistency 20000000
}
else {
	& $target_exe --unit-tests-only --test-fp-consistency 20000000 | Out-Null
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

$platform = "Windows"
$uploadUrl = "https://hypersomnia.xyz/upload_artifact.php"
$apiKey = $artifact_upload_key
$filePath = "Hypersomnia-for-$platform.exe"
$commitHash = $(git rev-parse HEAD)
$commitNumber = $(git rev-list --count master)
$commitMessage = $(git log -1 --pretty=%B)
$version = "1.1.$commitNumber"

mkdir detail/ssh
mv ../OpenSSH-Win64/ssh-keygen.exe detail/ssh/ssh-keygen.exe
mv $target_exe Hypersomnia.exe
Get-ChildItem
Remove-item -Recurse -Force cache, logs, user
$releaseNotesPath = "release_notes.txt"
"$version`n$commitHash`n$commitMessage" | out-file -filepath $releaseNotesPath

Push-AppveyorArtifact $releaseNotesPath

cd ../
7z a -sfx $filePath hypersomnia

Push-AppveyorArtifact $filePath
# curl.exe -F "key=$apiKey" -F "platform=$platform" -F "commit_hash=$commitHash" -F "version=$version" -F "artifact=@$filePath" -F "commit_message=$commitMessage" $uploadUrl

$filePath = "Hypersomnia-for-$platform.zip"
7z a $filePath hypersomnia

Push-AppveyorArtifact $filePath
# curl.exe -F "key=$apiKey" -F "platform=$platform" -F "commit_hash=$commitHash" -F "version=$version" -F "artifact=@$filePath" -F "commit_message=$commitMessage" $uploadUrl