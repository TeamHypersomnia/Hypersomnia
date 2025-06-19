param(
	[String]$configuration="Unknown",
	[Int32]$console_mode=0
)

ls

$target_exe = ".\Hypersomnia.exe"

$base_path = Join-Path ([Environment]::GetFolderPath("MyDocuments")) "My Games\Hypersomnia\logs"

$good_log = Join-Path $base_path "exit_success_debug_log.txt"
$fail_log = Join-Path $base_path "exit_failure_debug_log.txt"
$ensr_log = Join-Path $base_path "ensure_failed_debug_log.txt"

if ($console_mode -eq 1) { 
	& $target_exe --unit-tests-only --test-fp-consistency 20000000
}
else {
	& $target_exe --unit-tests-only --test-fp-consistency 20000000 | Out-Null
}

$exitCode = $LastExitCode

if ($exitCode -ne 0) { 
	$mesg = "Test " + $configuration + " launch failed with result: " + $exitCode
	Write-Host $mesg -ForegroundColor red
	
	Write-Host "Ensure failed log:" -ForegroundColor red
	cat $ensr_log -ErrorAction SilentlyContinue
	
	Write-Host "Failure log:" -ForegroundColor red
	cat $fail_log -ErrorAction SilentlyContinue
	$host.SetShouldExit($exitCode) 
	exit
}
else {
	$mesg = "Test " + $configuration + " launch succeeded."
	Write-Host $mesg -ForegroundColor green

	Write-Host "Good log:" -ForegroundColor green
	cat $good_log
}

Write-Host "Archiving the binary." -ForegroundColor yellow

$stem = "Hypersomnia-for-Windows"
$filePath = "$stem.exe"
$commitHash = $(git rev-parse HEAD)
$commitNumber = $(git rev-list --count master)
$commitMessage = $(git log -1 --pretty=%B)
$version = "1.2.$commitNumber"

Get-ChildItem

Remove-Item -Recurse -Force cache, logs, user -ErrorAction SilentlyContinue

ls

Write-Host "Generating release notes and archiving..." -ForegroundColor yellow

$releaseNotesPath = "release_notes.txt"
"$version`n$commitHash`n$commitMessage" | out-file -filepath $releaseNotesPath
dos2unix $releaseNotesPath

cd ../
7z a -sfx $filePath hypersomnia

$filePath = "$stem.zip"
7z a $filePath hypersomnia