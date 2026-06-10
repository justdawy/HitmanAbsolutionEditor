$user_hashes = Get-Content "e:\habsolution\user_hashes.txt"
$chunk_hashes = Get-Content "e:\habsolution\chunk_hashes.txt"

foreach ($chunk_line in $chunk_hashes) {
    if ($chunk_line -match "^([A-Z0-9]+)\s+:\s+(.*)$") {
        $hash = $matches[1]
        $name = $matches[2]
        if ($user_hashes -contains $hash) {
            Write-Host "FOUND: $hash -> $name"
        }
    }
}
