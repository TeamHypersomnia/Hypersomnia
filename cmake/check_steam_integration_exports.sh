#!/usr/bin/env bash
set -eo pipefail

steam_integration_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/steam_integration"

find_llvm_tool() {
	local tool=$1

	for candidate in "$tool" "$tool-20" $(ls -d /usr/lib/llvm-*/bin/$tool 2>/dev/null | sort -V | tail -1); do
		if command -v "$candidate" > /dev/null; then
			echo "$candidate"
			return
		fi
	done

	echo "No $tool found - install LLVM." >&2
	exit 1
}

llvm_nm=$(find_llvm_tool llvm-nm)
llvm_readobj=$(find_llvm_tool llvm-readobj)

declared_functions=$(sed -nE 's/^[[:space:]]*DLL_EXPORT[^(]*[^A-Za-z0-9_]([A-Za-z_][A-Za-z0-9_]*)\(.*/\1/p' "$steam_integration_dir/steam_integration.h" | sort -u)

if [ -z "$declared_functions" ]; then
	echo "No DLL_EXPORT functions found in steam_integration.h." >&2
	exit 1
fi

linux_exports() {
	"$llvm_nm" -D --defined-only "$steam_integration_dir/bin/linux/libsteam_integration.so" | awk '{ print $NF }'
}

macos_exports() {
	local arch=$1
	"$llvm_nm" -g --defined-only --arch="$arch" "$steam_integration_dir/bin/macos/libsteam_integration.dylib" | awk '{ print $NF }' | sed 's/^_//'
}

windows_exports() {
	"$llvm_readobj" --coff-exports "$steam_integration_dir/bin/windows/steam_integration.dll" | awk '/Name:/ { print $2 }'
}

any_missing=0

check_binary() {
	local binary_label=$1
	local exports=$2

	for function_name in $declared_functions; do
		if ! grep -qx "$function_name" <<< "$exports"; then
			echo "$binary_label does not export $function_name - rebuild it with the Steamworks SDK (steambuilders) and commit it." >&2
			any_missing=1
		fi
	done
}

linux_symbols=$(linux_exports)
macos_arm64_symbols=$(macos_exports arm64)
macos_x86_64_symbols=$(macos_exports x86_64)
windows_symbols=$(windows_exports)

check_binary "bin/linux/libsteam_integration.so" "$linux_symbols"
check_binary "bin/macos/libsteam_integration.dylib (arm64)" "$macos_arm64_symbols"
check_binary "bin/macos/libsteam_integration.dylib (x86_64)" "$macos_x86_64_symbols"
check_binary "bin/windows/steam_integration.dll" "$windows_symbols"

if [ $any_missing -eq 1 ]; then
	exit 1
fi

echo "Every Steam integration binary exports all $(wc -w <<< "$declared_functions") functions of steam_integration.h."
